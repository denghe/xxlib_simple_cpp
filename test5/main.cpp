#if !defined(NDEBUG)
#define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <iostream>
#include <chrono>
#include <vector>
#include <unordered_map>
#include "ska.h"
#include "tsl/hopscotch_map.h"
#include "tsl/bhopscotch_map.h"

int64_t NowMS() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
};

struct Player {
    int64_t id;
    int64_t money;
};
struct Log {
    int64_t playerId;
    int64_t money;
};

//struct Player_id {
//};
//using Players = boost::multi_index_container<Player,
//    boost::multi_index::indexed_by<
//    boost::multi_index::ordered_unique<boost::multi_index::tag<Player_id>, ::boost::multi_index::member<Player, int64_t, &Player::id> >
//    >
//>;
//
//struct Log_playerId {
//};
//struct Log_money {
//};
//struct Log_time {
//};
//using Logs = boost::multi_index_container<Log,
//    boost::multi_index::indexed_by<
//    boost::multi_index::ordered_non_unique<boost::multi_index::tag<Log_playerId>, ::boost::multi_index::member<Log, int64_t, &Log::playerId> >,
//    boost::multi_index::ordered_non_unique<boost::multi_index::tag<Log_money>, ::boost::multi_index::member<Log, int64_t, &Log::money> >
//    >
//>;
int main() {
    //Players players;
    //Logs logs;
    //for (int i = 0; i < 100; ++i) {
    //    players.emplace(Player{ i, i });
    //}
    //for (int j = 0; j < 10000; ++j) {
    //    for (int i = 0; i < 100; ++i) {
    //        logs.emplace(Log{ i, j, i });
    //    }
    //}
    //auto&& ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    //for (auto&& p : players) {
    //    auto&& q = logs.equal_range(p.id);
    //    for (auto iter = q.first; iter != q.second; ++iter) {
    //        (int64_t&)p.money += iter->money;
    //    }
    //}
    //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - ms << std::endl;
    //for (auto&& p : players) {
    //    std::cout << p.id << ", " << p.money << std::endl;
    //}
    auto&& ms = NowMS();
    std::vector<Player> players;
    players.reserve(10000);
    std::vector<Log> logs;
    logs.reserve(10000);
    for (int64_t i = 0; i < 10000; ++i) {
        auto&& o = players.emplace_back();
        o.id = o.money = i;
    }
    for (int64_t j = 0; j < 10000; ++j) {
        for (int64_t i = 0; i < 10000; ++i) {
            auto&& o = logs.emplace_back();
            o.playerId = i;
            o.money = j;
        }
    }
    std::cout << NowMS() - ms << std::endl;
    ms = NowMS();
    //std::unordered_map<int64_t, int64_t> ii;
    //ska::flat_hash_map<int64_t, int64_t> ii;
    //tsl::hopscotch_map<int64_t, int64_t> ii;
    tsl::bhopscotch_map<int64_t, int64_t> ii;
    for (auto&& L : logs) {
        ii[L.playerId] += L.money;
    }
    for (auto&& p : players) {
        p.money += ii[p.id];
    }
    std::cout << NowMS() - ms << std::endl;
    for (int k = 0; k < 10; ++k) {
        std::cout << players[k].id << ", " << players[k].money << std::endl;
    }
    return 0;
}
