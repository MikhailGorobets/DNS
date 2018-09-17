#pragma once

#include <cinttypes>
#include <iostream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>

namespace DNS {
	namespace Detail {

#pragma pack(push, 1)
		class Header {
		public:
			friend class boost::serialization::access;
		public:
			uint16_t ID;

			union {
				struct {
					uint8_t  RecursionDesired : 1;
					uint8_t  Truncation : 1;
					uint8_t  Authoritative : 1;
					uint8_t  Opcode : 4;
					uint8_t  IsResponseCode : 1;

					uint8_t  ResponseCode : 4;
					uint8_t  CheckingDisabled : 1;
					uint8_t  AuthenticatedData : 1;
					uint8_t  Reserved : 1;
					uint8_t  RecursionAvailable : 1;
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

			template<typename Archive>
			auto serialize(Archive & ar, const unsigned int version) -> void {
				ar & BOOST_SERIALIZATION_NVP(ID);
				ar & BOOST_SERIALIZATION_NVP(Flags);
				ar & BOOST_SERIALIZATION_NVP(CountQuestion);
				ar & BOOST_SERIALIZATION_NVP(CountAnswer);
				ar & BOOST_SERIALIZATION_NVP(CountAuthority);
				ar & BOOST_SERIALIZATION_NVP(CountAdditional);
			}

		};
#pragma pack(pop)

#pragma pack(push, 1)
		class Question {
		public:
			friend class boost::serialization::access;
		public:
			uint16_t Type;
			uint16_t Class;
		private:
			template<typename Archive>
			auto serialize(Archive & ar, const unsigned int version) -> void {
				ar & BOOST_SERIALIZATION_NVP(Type);
				ar & BOOST_SERIALIZATION_NVP(Class);
			}
		};
#pragma pack(pop)

#pragma pack(push, 1)
		class Answer {
		public:
			friend class boost::serialization::access;
		public:
			uint16_t Type;
			uint16_t Class;
			uint32_t TTL;
			uint16_t DataLenght;
		private:
			template<typename Archive>
			auto serialize(Archive & ar, const unsigned int version) -> void {
				ar & BOOST_SERIALIZATION_NVP(Type);
				ar & BOOST_SERIALIZATION_NVP(Class);
				ar & BOOST_SERIALIZATION_NVP(TTL);
				ar & BOOST_SERIALIZATION_NVP(DataLenght);
			}
		};
#pragma pack(pop)

		template <typename T> auto SwapEndian(T u)                 noexcept->T;
	                          auto ParseName(const uint8_t* pData) noexcept->std::vector<uint8_t>;

		std::ostream& operator<<(std::ostream& os, Header const& header);
		std::ostream& operator<<(std::ostream& os, Answer const& answer);
		std::ostream& operator<<(std::ostream& os, Question const& question);
	}
}

namespace DNS {
	namespace Detail {

		template <typename T>
		[[nodiscard]] auto SwapEndian(T u) noexcept -> T {
			static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
			union {
				T u;
				uint8_t u8[sizeof(T)];
			} source, dest;
			source.u = u;
			for (size_t i = 0; i < sizeof(T); i++)
				dest.u8[i] = source.u8[sizeof(T) - i - 1];
			return dest.u;
		}


		[[nodiscard]] inline auto ParseName(const uint8_t* pData) noexcept -> std::vector<uint8_t> {
			if (pData[0] == 0xC0)
				return std::vector<uint8_t>{ pData, pData + 2};
			uint8_t count = 0;
			while (pData[count] != 0x00) count++;
			return std::vector<uint8_t>{ pData, pData + count + 1};
		}


		inline std::ostream& operator<<(std::ostream& os, Header const& header) {
			os << "ID: " << SwapEndian<uint16_t>(header.ID) << std::endl;
			os << "RecursionDesired: " << static_cast<bool>(header.RecursionDesired) << std::endl;
			os << "Truncation: " << static_cast<bool>(header.Truncation) << std::endl;
			os << "Authoritative: " << static_cast<bool>(header.Authoritative) << std::endl;
			os << "Opcode: " << static_cast<bool>(header.Opcode) << std::endl;
			os << "IsResponseCode: " << static_cast<bool>(header.IsResponseCode) << std::endl;
			os << "ResponseCode: " << static_cast<bool>(header.ResponseCode) << std::endl;
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
	}
}