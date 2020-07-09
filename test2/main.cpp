#include "xx_object.h"
int main() {
    std::vector<std::vector<int>> intss;
    auto &&ints = intss.emplace_back();
    ints.emplace_back(4);
    ints.emplace_back(5);

    xx::CoutTN(1, 2.3, "asdf", intss);

    xx::Data d;
    xx::DataWriter dw(d);
    dw.Write(1, 2.3, "asdf", intss);


    return 0;
}
