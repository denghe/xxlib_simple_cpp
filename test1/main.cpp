/*
    todo: 测试 thread pool
*/

#include "xx_sqlite.h"
#include "xx_chrono.h"
#include <iostream>

namespace XS = xx::SQLite;

int main() {
    //XS::Connection db(":memory:");
    XS::Connection db("asdf.db3");
    if (!db) return -1;
    try {
        if (!db.TableExists("log")) {
            db.Execute(R"=-=(
CREATE TABLE [log](
    [id] INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,
    [time] INTEGER NOT NULL,
    [desc] TEXT NOT NULL
);)=-=");
        } else {
            db.TruncateTable("log");
        }
        XS::Query insertQuery(db, "insert into log (`id`, `time`, `desc`) values (?, ?, ?)");
        auto ms1 = xx::NowSteadyEpochMS();
        for (int j = 0; j < 10; ++j) {
            db.BeginTransaction();
            for (int i = 0; i < 1000; ++i) {
                insertQuery.SetParameters(j * 1000 + i, xx::NowEpoch10m(), std::to_string(j * 1000 + i) + " asdfasdf");
                insertQuery.Execute();
            }
            db.Commit();
        }
        std::cout << "ms = " << (xx::NowSteadyEpochMS() - ms1) << std::endl;

        //XS::Query selectQuery(db, "select `id`, `time`, `desc` from log");
//        selectQuery.Execute([](XS::Reader &r) {
//            auto id = r.ReadInt32(0);
//            auto time = r.ReadInt64(1);
//            auto desc = r.ReadString(2);
//            std::cout << id << ", " << time << ", " << desc << std::endl;
//        });
    }
    catch (const std::exception &e) {
        std::cout << "catch exception: " << e.what() << std::endl;
    }
    return 0;
}



//#include <xx_datareader.h>
//#include <iostream>
//#include <cassert>
//
//int main() {
//	xx::Data data;
//
//	xx::Write(data, 1, 2, 3, 4, 5, "asdf");
//
//	int a, b, c, d, e;
//	std::string s;
//	int r = xx::Read(data, a, b, c, d, e, s);
//	assert(r == 0);
//	assert(a == 1 && b == 2 && c == 3 && d == 4 && e == 5 && s == "asdf");
//
//	xx::DataReader dr(data);
//	r = dr.Read(a, b, c, d, e);
//	assert(r == 0);
//	auto offset = dr.offset;
//	r = dr.ReadLimit<3>(s);
//	assert(r != 0);
//	dr.offset = offset;
//    r = dr.ReadLimit<4>(s);
//    assert(r == 0);
//
//	std::vector<std::string> ss;
//	ss.emplace_back("asdf");
//	ss.emplace_back("qwer");
//	ss.emplace_back("zxcvb");
//	data.Clear();
//	xx::Write(data, ss);
//	r = xx::Read(data, ss);
//	assert(r == 0);
//
//	dr.Reset(data);
//	r = dr.ReadLimit<3, 5>(ss);
//	assert(r == 0);
//
//	dr.Reset(data);
//	r = dr.ReadLimit<3, 4>(ss);
//	assert(r != 0);
//
//	std::optional<std::vector<std::optional<std::string>>> ss2;
//	ss2.emplace();
//	ss2->emplace_back("asdf");
//	ss2->emplace_back();
//	ss2->emplace_back("qwer");
//	data.Clear();
//	xx::Write(data, ss2);
//
//	return 0;
//}
