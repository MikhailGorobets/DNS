#include <dns/dns.hpp>

namespace DNS {
    auto ParseName(const uint8_t* pData) noexcept -> std::string {
        if (pData[0] == 0xC0) //Comression name
            return std::string(pData, pData + 2);
        if (pData[0] == 0x00) //Root name
            return std::string(pData, pData + 1);
        size_t size = std::strlen(reinterpret_cast<const char* const>(pData) + 1);
        return std::string(pData, pData + size + 2);
    }

    auto CreatePackageFromBuffer(std::span<const uint8_t> buffer) -> Package {
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

    auto CreateBufferFromPackage(Package const& package) -> std::vector<uint8_t> {
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

    auto ComputeSize(Package const& package) -> size_t {
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

namespace DNS {
    std::ostream& operator<<(std::ostream& os, Header const& header) {
        fmt::print(os, "ID: {} \n", SwapEndian<uint16_t>(header.ID));
        fmt::print(os, "RecursionDesired: {} \n", static_cast<bool>(header.RecursionDesired));
        fmt::print(os, "Truncation: {} \n", static_cast<bool>(header.Truncation));
        fmt::print(os, "Authoritative: {} \n", static_cast<bool>(header.Authoritative));
        fmt::print(os, "Opcode: {} \n", header.Opcode);
        fmt::print(os, "IsResponseCode: {} \n", static_cast<bool>(header.IsResponseCode));
        fmt::print(os, "ResponseCode: {} \n", header.ResponseCode);
        fmt::print(os, "CheckingDisabled: {} \n", static_cast<bool>(header.CheckingDisabled));
        fmt::print(os, "Reserved: {} \n", static_cast<bool>(header.Reserved));
        fmt::print(os, "RecursionAvailable: {} \n", static_cast<bool>(header.RecursionAvailable));
        fmt::print(os, "Count question: {} \n", SwapEndian<uint16_t>(header.CountQuestion));
        fmt::print(os, "Count answer: {} \n", SwapEndian<uint16_t>(header.CountAnswer));
        fmt::print(os, "Count authority: {} \n", SwapEndian<uint16_t>(header.CountAuthority));
        fmt::print(os, "Count additional: {} \n", SwapEndian<uint16_t>(header.CountAdditional));
        return os;
    }

    std::ostream& operator<<(std::ostream& os, Answer const& answer) {
        fmt::print(os, "Type: {} \n", SwapEndian<uint16_t>(answer.Type));
        fmt::print(os, "Class: {} \n", SwapEndian<uint16_t>(answer.Class));
        fmt::print(os, "TTL: {} \n", SwapEndian<uint16_t>(answer.TTL));
        fmt::print(os, "Data size: {} \n", SwapEndian<uint16_t>(answer.DataLenght));
        return os;
    }

    std::ostream& operator<<(std::ostream& os, Question const& question) {
        fmt::print(os, "Type: {} \n", SwapEndian<uint16_t>(question.Type));
        fmt::print(os, "Class: {} \n", SwapEndian<uint16_t>(question.Class));
        return os;
    }

    std::ostream& operator<<(std::ostream& os, Query const& query) {
        fmt::print(os, "Name: {} \n", query.Name.c_str());
        fmt::print(os, "{} \n", query.Question);
        return os;
    }

    std::ostream& operator<<(std::ostream& os, ResourceRecord const& record) {
        fmt::print(os, "Name: {} \n", record.Name.c_str());
        fmt::print(os, "{} \n", record.Answer);
        fmt::print(os, "-------------- \n");
        fmt::print(os, "{} \n", record.Data.c_str());
        return os;
    }

    std::ostream& operator<<(std::ostream& os, Package const& package) {
        fmt::print(os, "DNS package: \n");

        fmt::print(os, "Header: \n");
        fmt::print(os, "{} \n", package.Header);

        for (auto const& e : package.Questions) {
            fmt::print(os, "-------------- \n");
            fmt::print(os, "Questions: \n");
            fmt::print(os, "{} \n", e);
        }

        for (auto const& e : package.Answers) {
            fmt::print(os, "-------------- \n");
            fmt::print(os, "Answers: \n");
            fmt::print(os, "{} \n", e);
        }

        for (auto const& e : package.Authoritys) {
            fmt::print(os, "-------------- \n");
            fmt::print(os, "Authoritys: \n");
            fmt::print(os, "{} \n", e);
        }

        for (auto const& e : package.Additional) {
            fmt::print(os, "-------------- \n");
            fmt::print(os, "Additional: \n");
            fmt::print(os, "{} \n", e);
        }
        return os;
    }
}