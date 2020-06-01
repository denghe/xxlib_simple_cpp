#include <xx_data_view.h>
#include <iostream>
#include <cassert>

int main() {
	xx::Data data;

	xx::Write(data, 1, 2, 3, 4, 5, "asdf");

	int a, b, c, d, e;
	std::string s;
	int r = xx::Read(data, a, b, c, d, e, s);
	assert(r == 0);

	xx::DataReader dr(data);
	r = dr.Read(a, b, c, d, e);
	assert(r == 0);
	r = dr.ReadLimit<3>(s);
	assert(r != 0);
	
	return 0;
}
