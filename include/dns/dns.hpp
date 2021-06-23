#pragma once

#include <array>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <fmt/printf.h>
#include <fmt/ostream.h>


namespace DNS {

    constexpr std::size_t PACKAGE_SIZE = 4096;

    using Name = std::string;
    using Data = std::string;

    struct Header {
        uint16_t ID;
        union {
            struct {
                uint8_t RecursionDesired : 1;
                uint8_t Truncation : 1;
                uint8_t Authoritative : 1;
                uint8_t Opcode : 4;
                uint8_t IsResponseCode : 1;

                uint8_t ResponseCode : 4;
                uint8_t CheckingDisabled : 1;
                uint8_t AuthenticatedData : 1;
                uint8_t Reserved : 1;
                uint8_t RecursionAvailable : 1;
            };
            struct {
                uint16_t Flags;
            };
            struct {
                uint8_t  Flags1;
                uint8_t  Flags2;
            };
        };
        uint16_t CountQuestion;
        uint16_t CountAnswer;
        uint16_t CountAuthority;
        uint16_t CountAdditional;
    };

    struct Question {
        uint16_t Type = {};
        uint16_t Class = {};
    };

#pragma pack(push, 1)
    struct Answer {
        uint16_t Type = {};
        uint16_t Class = {};
        uint32_t TTL;
        uint16_t DataLenght = {};
    };
#pragma pack(pop)

    struct Query {
        Name     Name = {};
        Question Question = {};
    };

    struct ResourceRecord {
        Name   Name = {};
        Answer Answer = {};
        Data   Data = {};
    };

    struct Package {
        DNS::Header Header = {};
        std::vector<DNS::Query>          Questions = {};
        std::vector<DNS::ResourceRecord> Answers = {};
        std::vector<DNS::ResourceRecord> Authoritys = {};
        std::vector<DNS::ResourceRecord> Additional = {};
    };

    template <typename T>
    auto SwapEndian(T value) noexcept -> T;

    auto ParseName(const uint8_t* pData) noexcept -> std::string;

    auto CreatePackageFromBuffer(std::vector<uint8_t> const& buffer)->Package;

    auto CreateBufferFromPackage(Package const& package)->std::vector<uint8_t>;

    auto ComputeSize(Package const& package)->std::size_t;

    template<typename Archive>
    auto Serialize(Archive& arhive, Question const& question, uint32_t version) -> void {
        arhive& BOOST_SERIALIZATION_NVP(question.Type);
        arhive& BOOST_SERIALIZATION_NVP(question.Class);
    }

    template<typename Archive>
    auto Serialize(Archive& arhive, Header const& header, uint32_t version) -> void {
        arhive& BOOST_SERIALIZATION_NVP(header.ID);
        arhive& BOOST_SERIALIZATION_NVP(header.Flags);
        arhive& BOOST_SERIALIZATION_NVP(header.CountQuestion);
        arhive& BOOST_SERIALIZATION_NVP(header.CountAnswer);
        arhive& BOOST_SERIALIZATION_NVP(header.CountAuthority);
        arhive& BOOST_SERIALIZATION_NVP(header.CountAdditional);
    }

    template<typename Archive>
    auto Serialize(Archive& arhive, Answer const& answer, uint32_t version) -> void {
        arhive& BOOST_SERIALIZATION_NVP(answer.Type);
        arhive& BOOST_SERIALIZATION_NVP(answer.Class);
        arhive& BOOST_SERIALIZATION_NVP(answer.TTL);
        arhive& BOOST_SERIALIZATION_NVP(answer.DataLenght);
    }

    template<typename Archive>
    auto Serialize(Archive& archive, Query const& query, uint32_t version) -> void {
        archive& BOOST_SERIALIZATION_NVP(query.Name);
        archive& BOOST_SERIALIZATION_NVP(query.Question);
    }

    template<typename Archive>
    auto Serialize(Archive& archive, ResourceRecord const& record, uint32_t version) -> void {
        archive& BOOST_SERIALIZATION_NVP(record.Name);
        archive& BOOST_SERIALIZATION_NVP(record.Answer);
        archive& BOOST_SERIALIZATION_NVP(record.Data);
    }

    template<typename Archive>
    auto Serialize(Archive& archive, Package const& package, uint32_t version) -> void {
        archive& BOOST_SERIALIZATION_NVP(package.Header);
        archive& BOOST_SERIALIZATION_NVP(package.Questions);
        archive& BOOST_SERIALIZATION_NVP(package.Answers);
        archive& BOOST_SERIALIZATION_NVP(package.Authoritys);
        archive& BOOST_SERIALIZATION_NVP(package.Additional);
    }

    std::ostream& operator<<(std::ostream& os, Header const& header);

    std::ostream& operator<<(std::ostream& os, Answer const& answer);

    std::ostream& operator<<(std::ostream& os, Question const& question);

    std::ostream& operator<<(std::ostream& os, Query const& query);

    std::ostream& operator<<(std::ostream& os, ResourceRecord const& record);

    std::ostream& operator<<(std::ostream& os, Package const& query);
}


namespace DNS {

    inline std::ostream& operator<<(std::ostream& os, Header const& header) {
        os << "ID: " << SwapEndian<uint16_t>(header.ID) << std::endl;
        os << "RecursionDesired: " << static_cast<bool>(header.RecursionDesired) << std::endl;
        os << "Truncation: " << static_cast<bool>(header.Truncation) << std::endl;
        os << "Authoritative: " << static_cast<bool>(header.Authoritative) << std::endl;
        os << "Opcode: " << header.Opcode << std::endl;
        os << "IsResponseCode: " << static_cast<bool>(header.IsResponseCode) << std::endl;
        os << "ResponseCode: " << header.ResponseCode << std::endl;
        os << "CheckingDisabled: " << static_cast<bool>(header.CheckingDisabled) << std::endl;
        os << "Reserved: " << static_cast<bool>(header.Reserved) << std::endl;
        os << "RecursionAvailable: " << static_cast<bool>(header.RecursionAvailable) << std::endl;
        os << "Count question: " << SwapEndian<uint16_t>(header.CountQuestion) << std::endl;
        os << "Count answer: " << SwapEndian<uint16_t>(header.CountAnswer) << std::endl;
        os << "Count authority: " << SwapEndian<uint16_t>(header.CountAuthority) << std::endl;
        os << "Count additional: " << SwapEndian<uint16_t>(header.CountAdditional) << std::endl;
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, Answer const& answer) {
        os << "Type: " << SwapEndian<uint16_t>(answer.Type) << std::endl;
        os << "Class: " << SwapEndian<uint16_t>(answer.Class) << std::endl;
        os << "TTL: " << SwapEndian<uint32_t>(answer.TTL) << std::endl;
        os << "Data size: " << SwapEndian<uint16_t>(answer.DataLenght) << std::endl;
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, Question const& question) {
        os << "Type: " << SwapEndian<uint16_t>(question.Type) << std::endl;
        os << "Class: " << SwapEndian<uint16_t>(question.Class) << std::endl;
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, Query const& query) {
        os << "Name: " << query.Name.c_str() << std::endl;
        os << query.Question << std::endl;
        return os;
    }

    inline std::ostream& operator<<(std::ostream& os, ResourceRecord const& record) {
        os << "Name: " << record.Name.c_str() << std::endl;
        os << record.Answer << std::endl;
        os << "Data: " << record.Data.c_str() << std::endl;
        return os;
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

    template <typename T>
    [[nodiscard]] auto SwapEndian(T source) noexcept -> T {
        static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
        union {
            uint8_t bufferUnwrap[sizeof(T)];
            T       bufferWrap = {};
        } dest;
        for (size_t index = 0; index < sizeof(T); index++)
            dest.bufferUnwrap[index] = reinterpret_cast<uint8_t*>(&source)[sizeof(T) - index - 1];
        return dest.bufferWrap;
    }

    [[nodiscard]] inline auto ParseName(const uint8_t* pData) noexcept -> std::string {
        if (pData[0] == 0xC0) //Comression name
            return std::string(pData, pData + 2);
        if (pData[0] == 0x00) //Root name
            return std::string(pData, pData + 1);
        size_t size = std::strlen(reinterpret_cast<const char* const>(pData) + 1);
        return std::string(pData, pData + size + 2);
    }

    [[nodiscard]] inline auto CreatePackageFromBuffer(std::vector<uint8_t> const& buffer) -> Package {
        size_t offset = 0;
        Package package = {};
        std::memcpy(&package.Header, buffer.data(), sizeof(DNS::Header));
        offset += sizeof(DNS::Header);
        auto LoadQuery = [&](std::vector<DNS::Query>& quries) {
            DNS::Query query = {};
            query.Name = DNS::ParseName(buffer.data() + offset);
            offset += query.Name.size();
            std::memcpy(&query.Question, buffer.data() + offset, sizeof(DNS::Question));
            offset += sizeof(DNS::Question);
            quries.push_back(std::move(query));
        };

        auto LoadResource = [&](std::vector<DNS::ResourceRecord>& resources) {
            DNS::ResourceRecord resource = {};
            resource.Name = DNS::ParseName(buffer.data() + offset);
            offset += resource.Name.size();
            std::memcpy(&resource.Answer, buffer.data() + offset, sizeof(DNS::Answer));
            offset += sizeof(DNS::Answer);
            resource.Data.resize(DNS::SwapEndian<uint16_t>(resource.Answer.DataLenght));
            std::memcpy(const_cast<char*>(resource.Data.data()), buffer.data() + offset, DNS::SwapEndian<uint16_t>(resource.Answer.DataLenght));
            offset += DNS::SwapEndian<uint16_t>(resource.Answer.DataLenght);
            resources.push_back(std::move(resource));
        };

        for (size_t index = 0; index < DNS::SwapEndian(package.Header.CountQuestion); index++)
            LoadQuery(package.Questions);

        for (size_t index = 0; index < DNS::SwapEndian(package.Header.CountAnswer); index++)
            LoadResource(package.Answers);

        for (size_t index = 0; index < DNS::SwapEndian(package.Header.CountAuthority); index++)
            LoadResource(package.Authoritys);

        for (size_t index = 0; index < DNS::SwapEndian(package.Header.CountAdditional); index++)
            LoadResource(package.Additional);

        return package;
    }

    [[nodiscard]] inline auto CreateBufferFromPackage(Package const& package) -> std::vector<uint8_t> {
        uint16_t offset = 0;
        std::vector<uint8_t> buffer(PACKAGE_SIZE);
        std::memcpy(buffer.data(), &package.Header, sizeof(DNS::Header));
        offset += sizeof(DNS::Header);

        auto SaveQuery = [&](DNS::Query const& query) {
            std::memcpy(buffer.data() + offset, query.Name.data(), query.Name.size());
            offset += static_cast<uint16_t>(query.Name.size());
            std::memcpy(buffer.data() + offset, &query.Question, sizeof(DNS::Question));
            offset += sizeof(DNS::Question);
        };

        auto SaveResource = [&](DNS::ResourceRecord const& resource) {
            std::memcpy(buffer.data() + offset, resource.Name.data(), resource.Name.size());
            offset += static_cast<uint16_t>(resource.Name.size());
            std::memcpy(buffer.data() + offset, &resource.Answer, sizeof(DNS::Answer));
            offset += sizeof(DNS::Answer);
            std::memcpy(buffer.data() + offset, resource.Data.data(), DNS::SwapEndian(resource.Answer.DataLenght));
            offset += DNS::SwapEndian(resource.Answer.DataLenght);
        };

        for (size_t index = 0; index < DNS::SwapEndian(package.Header.CountQuestion); index++)
            SaveQuery(package.Questions[index]);

        for (size_t index = 0; index < DNS::SwapEndian(package.Header.CountAnswer); index++)
            SaveResource(package.Answers[index]);

        for (size_t index = 0; index < DNS::SwapEndian(package.Header.CountAuthority); index++)
            SaveResource(package.Authoritys[index]);

        for (size_t index = 0; index < DNS::SwapEndian(package.Header.CountAdditional); index++)
            SaveResource(package.Additional[index]);

        buffer.resize(offset);
        return buffer;
    }

    [[nodiscard]] inline auto ComputeSize(Package const& package) -> std::size_t {
        std::size_t size = 0;
        size += sizeof(package.Header);

        auto ComputeSizeQuery = [&](DNS::Query const& e) -> void {
            size += e.Name.size();
            size += sizeof(DNS::Question);
        };

        auto ComputeSizeResource = [&](DNS::ResourceRecord const& e) -> void {
            size += e.Name.size();
            size += sizeof(DNS::Answer);
            size += e.Data.size();
        };

        for (auto const& e : package.Questions)
            ComputeSizeQuery(e);

        for (auto const& e : package.Answers)
            ComputeSizeResource(e);

        for (auto const& e : package.Authoritys)
            ComputeSizeResource(e);

        for (auto const& e : package.Additional)
            ComputeSizeResource(e);

        return size;
    }
}