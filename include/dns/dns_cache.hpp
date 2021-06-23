#pragma once

#include <mutex>
#include <iomanip>
#include <ctime>
#include <fstream>
#include "dns.hpp"

namespace DNS {

   // class Cache : boost::noncopyable {
   // public:
   //     friend class boost::serialization::access;
   // public:
   //
   //     using ArhiveOut = boost::archive::xml_oarchive;
   //     using ArhiveIn  = boost::archive::xml_iarchive;
   //     using Address   = std::vector<uint8_t>;
   //     using Mutex     = std::recursive_mutex;
   //     using MutexPtr  = std::unique_ptr<Mutex>;
   //     class Key {
   //     public:
   //         friend class boost::serialization::access;
   //     public:
   //         auto operator==(Key const& lhs) const -> bool {
   //             return this->Flags == lhs.Flags
   //                 && this->Name == lhs.Name
   //                 && this->Question.Class == lhs.Question.Class
   //                 && this->Question.Type == lhs.Question.Type;
   //         }
   //         template<typename Archive>
   //         auto Serialize(Archive& ar, const unsigned int version) -> void {
   //             ar& BOOST_SERIALIZATION_NVP(Flags);
   //             ar& BOOST_SERIALIZATION_NVP(Name);
   //             ar& BOOST_SERIALIZATION_NVP(Question);
   //         }
   //     public:
   //         uint16_t  Flags;
   //         Address  Name;
   //         Question Question;
   //     };
   //     class KeyHash {
   //     public:
   //         auto operator()(const Key& key) const -> std::size_t {
   //             auto seed = key.Name.size();
   //             for (auto const& i : key.Name) seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
   //             return seed ^ ((std::hash<uint16_t>()(key.Question.Class) << 1) >> 1) ^ (std::hash<uint16_t>()(key.Question.Type) << 1) ^ key.Flags;
   //         }
   //     };
   //  //   using Map = std::unordered_map<Key, DNS::Package, KeyHash>;
   // public:
   //     Cache() = default;
   //
   //     Cache(Cache&& lhs) = default;
   //
   //     Cache& operator=(Cache&& lhs) = default;
   //
   //     auto Insert(Key const& addr, Package const& package) -> void;
   //
   //     auto Insert(Key const& addr, Package&& package) -> void;
   //
   //     auto Get(Key const& addr) const -> Package const&;
   //
   //     auto Update(std::time_t time)  -> void;
   //
   //     static auto Load(std::string const& fileName) -> Cache;
   //
   //     static auto Save(std::string const& fileName, Cache const& cache) -> void;
   //
   // private:
   //     template<typename Archive>
   //     auto Serialize(Archive& ar, const unsigned int version) -> void {
   //         ar& BOOST_SERIALIZATION_NVP(m_Cache);
   //         ar& BOOST_SERIALIZATION_NVP(m_LastTime);
   //     }
   // private:
   //     mutable MutexPtr  m_pLock = std::make_unique<Mutex>();
   //     Map               m_Cache = {};
   //     std::time_t       m_LastTime = std::time(nullptr);
   // };

}

namespace DNS {


   // inline auto Cache::Insert(Key const& addr, Package const& package) -> void {
   //     std::lock_guard<std::recursive_mutex> lock{ *m_pLock };
   //     m_Cache.insert(std::make_pair(addr, package));
   // }
   //
   // inline auto Cache::Insert(Key const& addr, Package&& package)-> void {
   //     std::lock_guard<std::recursive_mutex> lock{ *m_pLock };
   //     m_Cache.insert(std::make_pair(addr, std::move(package)));
   // }

    //[[nodiscard]] inline auto Cache::ContainsKey(Key const& addr) -> bool {
    //    std::lock_guard<std::recursive_mutex> lock{ *m_pLock };
    //    return m_Cache.find(addr) != m_Cache.end();
    //}

   // [[nodiscard]] inline auto Cache::Get(Key const& addr) const -> Package const& {
   //     std::lock_guard<std::recursive_mutex> lock{ *m_pLock };
   //     return m_Cache.at(addr);
   // }
   //
   // inline auto Cache::Update(std::time_t currentTime) -> void {
   //
       // auto localTime = *std::localtime(&currentTime);
       // std::cout << "Update cache-> Time: " << std::put_time(&localTime, "%d-%m-%Y %H-%M-%S") << std::endl;
       //
       //
       // std::lock_guard<std::recursive_mutex> lock{ *m_pLock };
       // auto delta = currentTime - m_LastTime;
       // std::vector<Key> removeList;
       //
       // auto computeTime = [](std::vector<DNS::ResourceRecord>& answer, std::time_t delta) {
       //     for (auto& e : answer)
       //         e.Answer.TTL -= DNS::Detail::SwapEndian<uint32_t>(static_cast<uint32_t>(delta));
       // };

        //
        //for (auto const& [x, y] : m_Cache) {
        //
        //    auto predicate = [](auto const& e) -> bool { return DNS::Detail::SwapEndian<uint32_t>(e.Answer.TTL) <= 0; };
        //
        //    computeTime(package.Answers, delta);
        //    //computeTime(package.Authoritys, delta);
        //    //computeTime(package.Additional, delta);
        //
        ////	package.Answers.erase(std::remove_if(package.Answers.begin(), package.Answers.end(), predicate), package.Answers.end());
        ////	package.Authoritys.erase(std::remove_if(package.Authoritys.begin(), package.Authoritys.end(), predicate), package.Authoritys.end());
        ////	package.Additional.erase(std::remove_if(package.Additional.begin(), package.Additional.end(), predicate), package.Additional.end());
        //
        ////	package.Header.CountAnswer = DNS::Detail::SwapEndian<uint16_t>(static_cast<uint16_t>(package.Answers.size()));
        ////	package.Header.CountAuthority = DNS::Detail::SwapEndian<uint16_t>(static_cast<uint16_t>(package.Authoritys.size()));
        ////	package.Header.CountAdditional = DNS::Detail::SwapEndian<uint16_t>(static_cast<uint16_t>(package.Additional.size()));
        //
        //    if (!package.Answers.empty() && DNS::Detail::SwapEndian<uint32_t>(package.Answers.front().Answer.TTL) <= 0)
        //        removeList.push_back(addess);
        //
        //}

        //for (auto const& e : removeList) m_Cache.erase(e);
        //
        //m_LastTime = currentTime;
  // }
   //
   //[[nodiscard]] inline auto Cache::Load(std::string const& fileName) -> Cache {
   //    std::cout << "Load cache" << std::endl;
   //    Cache cache{};
   //    std::ifstream istream{ fileName };
   //    ArhiveIn arhive{ istream };
   //    arhive >> BOOST_SERIALIZATION_NVP(cache);
   //    return std::move(cache);
   //}
   //
   //inline auto Cache::Save(std::string const& fileName, Cache const& cache) -> void {
   //    std::cout << "Save cache" << std::endl;
   //    std::ofstream ostream{ fileName };
   //    ArhiveOut arhive{ ostream };
   //    arhive << BOOST_SERIALIZATION_NVP(cache);
   //
   //}

}