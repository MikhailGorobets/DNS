#pragma once

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <ostream>
#include <span>


namespace DNS {

    constexpr std::size_t PACKAGE_SIZE = 2048;

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

    auto ParseName(const uint8_t* pData) noexcept -> std::string;

    auto CreatePackageFromBuffer(std::span<const uint8_t> buffer) -> Package;

    auto CreateBufferFromPackage(Package const& package) -> std::vector<uint8_t>;

    auto ComputeSize(Package const& package) -> size_t;

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
