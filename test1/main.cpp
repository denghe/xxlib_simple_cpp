#include "xx_sqlite.h"
#include "xx_mysql.h"

int main() {
    xx::SQLite::Connection L("asdf.db3");

    xx::MySql::Connection M;
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
