#pragma once

#include <cinttypes>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>

namespace DNS {
    namespace Detail {

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

        template <typename T>
        auto SwapEndian(T value) noexcept -> T;

        auto ParseName(const uint8_t* pData) noexcept -> std::string;

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
            arhive & BOOST_SERIALIZATION_NVP(answer.Type);
            arhive & BOOST_SERIALIZATION_NVP(answer.Class);
            arhive & BOOST_SERIALIZATION_NVP(answer.TTL);
            arhive & BOOST_SERIALIZATION_NVP(answer.DataLenght);
        }

        std::ostream& operator<<(std::ostream& os, Header const& header);

        std::ostream& operator<<(std::ostream& os, Answer const& answer);

        std::ostream& operator<<(std::ostream& os, Question const& question);
    }
}

namespace DNS {
	namespace Detail {

		template <typename T>
		[[nodiscard]] auto SwapEndian(T source) noexcept -> T {
			static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");
            union {
                uint8_t bufferUnwrap[sizeof(T)];
                T       bufferWrap = {};
            } dest;         
			for (size_t index = 0; index < sizeof(T); index++)
				dest.bufferUnwrap[index] =  reinterpret_cast<uint8_t*>(&source)[sizeof(T) - index - 1];
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
	}
}