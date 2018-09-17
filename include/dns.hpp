#pragma once

#include <array>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include "dns_detail.hpp"

constexpr std::size_t DNS_PACKAGE_SIZE = 1024;

namespace DNS {

	using Header = DNS::Detail::Header;

	class Query {
	public:
		friend class boost::serialization::access;
	public:
		using DNSName     = std::vector<uint8_t>;
		using DNSQuestion = Detail::Question;
	public:
		DNSName     Name;
		DNSQuestion Question;

		template<typename Archive>
		auto serialize(Archive & ar, const unsigned int version) -> void {
			ar & BOOST_SERIALIZATION_NVP(Name);
			ar & BOOST_SERIALIZATION_NVP(Question);
		}
	};

	class ResourceRecord {
	public:
		using DNSName = std::vector<uint8_t>;
		using DNSData = std::vector<uint8_t>;
		using DNSAnswer = Detail::Answer;
		friend class boost::serialization::access;
	public:
		DNSName   Name;
		DNSAnswer Answer;
		DNSData   Data;
	private:
		template<typename Archive>
		auto serialize(Archive & ar, const unsigned int version) -> void {
			ar & BOOST_SERIALIZATION_NVP(Name);
			ar & BOOST_SERIALIZATION_NVP(Answer);
			ar & BOOST_SERIALIZATION_NVP(Data);
		}
	};

	class Package {
	public:
		friend class boost::serialization::access;
	public:
		DNS::Header Header;
		std::vector<DNS::Query>          Questions;
		std::vector<DNS::ResourceRecord> Answers;
		std::vector<DNS::ResourceRecord> Authoritys;
		std::vector<DNS::ResourceRecord> Additional;
	public:
		static auto CreatePackageFromBuffer(std::array<uint8_t, DNS_PACKAGE_SIZE> const& buffer)->Package;
		static auto CreateBufferFromPackage(Package const& package)->std::array<uint8_t, DNS_PACKAGE_SIZE>;
		static auto ComputeSize(Package const& package)->std::size_t;
	private:
		template<typename Archive>
		auto serialize(Archive & ar, const unsigned int version) -> void {
			ar & BOOST_SERIALIZATION_NVP(Header);
			ar & BOOST_SERIALIZATION_NVP(Questions);
			ar & BOOST_SERIALIZATION_NVP(Answers);
			ar & BOOST_SERIALIZATION_NVP(Authoritys);
			ar & BOOST_SERIALIZATION_NVP(Additional);
		}
	};


	std::ostream& operator<<(std::ostream& os, Query const& query);
	std::ostream& operator<<(std::ostream& os, ResourceRecord const& record);
	std::ostream& operator<<(std::ostream& os, Package const& query);

}

namespace DNS {

	

	inline std::ostream& operator<<(std::ostream& os, Query const& query) {
		os << "Name: " << query.Name.data() << std::endl;
		os << query.Question << std::endl;
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, ResourceRecord const& record) {
		os << "Name: " << record.Name.data() << std::endl;
		os << record.Answer << std::endl;
		os << "Data: " << record.Data.data() << std::endl;
		return os;
	}

	[[nodiscard]] inline auto Package::CreatePackageFromBuffer(std::array<uint8_t, DNS_PACKAGE_SIZE> const& buffer) -> Package {

		uint16_t offset = 0;
		Package package;
		std::memcpy(&package.Header, buffer.data(), sizeof(DNS::Header));
		offset += sizeof(DNS::Header);
		auto loadQuery = [&](std::vector<DNS::Query> & quries) {
			DNS::Query query;
			query.Name = DNS::Detail::ParseName(buffer.data() + offset);
			offset += static_cast<uint16_t>(query.Name.size());
			std::memcpy(&query.Question, buffer.data() + offset, sizeof(DNS::Query::DNSQuestion));
			offset += sizeof(DNS::Query::DNSQuestion);
			quries.push_back(query);

		};

		auto loadResource = [&](std::vector<DNS::ResourceRecord> & resources) {
			DNS::ResourceRecord resource;
			resource.Name = DNS::Detail::ParseName(buffer.data() + offset);
			offset += static_cast<uint16_t>(resource.Name.size());
			std::memcpy(&resource.Answer, buffer.data() + offset, sizeof(DNS::ResourceRecord::DNSAnswer));
			offset += sizeof(DNS::ResourceRecord::DNSAnswer);
			resource.Data.resize(DNS::Detail::SwapEndian<uint16_t>(resource.Answer.DataLenght));
			std::memcpy(resource.Data.data(), buffer.data() + offset, DNS::Detail::SwapEndian<uint16_t>(resource.Answer.DataLenght));
			offset += DNS::Detail::SwapEndian<uint16_t>(resource.Answer.DataLenght);
			resources.push_back(resource);
		};

		for (auto index = 0; index < DNS::Detail::SwapEndian(package.Header.CountQuestion); index++)
			loadQuery(package.Questions);

		for (auto index = 0; index < DNS::Detail::SwapEndian(package.Header.CountAnswer); index++)
			loadResource(package.Answers);

		for (auto index = 0; index < DNS::Detail::SwapEndian(package.Header.CountAuthority); index++)
			loadResource(package.Authoritys);

		for (auto index = 0; index <  DNS::Detail::SwapEndian(package.Header.CountAdditional); index++)
			loadResource(package.Additional);

		return package;
	}

	[[nodiscard]] inline auto Package::CreateBufferFromPackage(Package const & package)->std::array<uint8_t, DNS_PACKAGE_SIZE> {
		uint16_t offset = 0;
		std::array<uint8_t, DNS_PACKAGE_SIZE> buffer;
		std::memcpy(buffer.data(), &package.Header, sizeof(DNS::Header));
		offset += sizeof(DNS::Header);


		auto saveQuery = [&](DNS::Query const& query) {
			std::memcpy(buffer.data() + offset, query.Name.data(), query.Name.size());
			offset += static_cast<uint16_t>(query.Name.size());
			std::memcpy(buffer.data() + offset, &query.Question, sizeof(DNS::Query::DNSQuestion));
			offset += sizeof(DNS::Query::DNSQuestion);
		};

		auto saveResource = [&](DNS::ResourceRecord const& resource) {
			std::memcpy(buffer.data() + offset, resource.Name.data(), resource.Name.size());
			offset += static_cast<uint16_t>(resource.Name.size());
			std::memcpy(buffer.data() + offset, &resource.Answer, sizeof(DNS::ResourceRecord::DNSAnswer));
			offset += sizeof(DNS::ResourceRecord::DNSAnswer);
			std::memcpy(buffer.data() + offset, resource.Data.data(), DNS::Detail::SwapEndian(resource.Answer.DataLenght));
			offset += DNS::Detail::SwapEndian(resource.Answer.DataLenght);
		};

		for (auto index = 0; index <  DNS::Detail::SwapEndian(package.Header.CountQuestion); index++)
			saveQuery(package.Questions[index]);

		for (auto index = 0; index < DNS::Detail::SwapEndian(package.Header.CountAnswer); index++)
			saveResource(package.Answers[index]);

		for (auto index = 0; index < DNS::Detail::SwapEndian(package.Header.CountAuthority); index++)
			saveResource(package.Authoritys[index]);

		for (auto index = 0; index < DNS::Detail::SwapEndian(package.Header.CountAdditional); index++)
			saveResource(package.Additional[index]);

		return buffer;
	}

	inline auto Package::ComputeSize(Package const & package) -> std::size_t {
		std::size_t size = 0;
		size += sizeof(package.Header);

		auto computeSizeQuery = [&](DNS::Query const& e) {
			size += e.Name.size();
			size += sizeof(DNS::Query::DNSQuestion);
		};

		auto computeSizeResource = [&](DNS::ResourceRecord const& e) {
			size += e.Name.size();
			size += sizeof(DNS::ResourceRecord::DNSAnswer);
			size += e.Data.size();
		};

		for (auto const& e : package.Questions)
			computeSizeQuery(e);

		for (auto const& e : package.Answers)
			computeSizeResource(e);

		for (auto const& e : package.Authoritys)
			computeSizeResource(e);

		for (auto const& e : package.Additional)
			computeSizeResource(e);

		return size;
	}

	inline std::ostream& operator<<(std::ostream& os, Package const& package) {
		os << "DNS package:" << std::endl;
		os << "Header: " << std::endl;
		os << package.Header;

		for (auto const& e : package.Questions) {
			os << "--------------" << std::endl;
			os << "Questions: " << std::endl;
			os << e << std::endl;
		}

		for (auto const& e : package.Answers) {
			os << "--------------" << std::endl;
			os << "Answers: " << std::endl;
			os << e << std::endl;
		}

		for (auto const& e : package.Authoritys) {
			os << "--------------" << std::endl;
			os << "Authoritys: " << std::endl;
			os << e << std::endl;
		}

		for (auto const& e : package.Additional) {
			os << "--------------" << std::endl;
			os << "Additional: " << std::endl;
			os << e << std::endl;
		}

		return os;
	}
}