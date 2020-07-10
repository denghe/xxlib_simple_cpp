#include "xx_object.h"

struct Foo : xx::Object {
    using BaseType = xx::Object;

    XX_OBJECT_OVERRIDES_H

    int i = 1;
    std::string name = "asdf";
};

struct Bar : Foo {
    using BaseType = Foo;

    XX_OBJECT_OVERRIDES_H

    float x = 1.2, y = 3.4;
};

namespace xx {
    template<>
    struct TypeId<Foo> {
        static const uint16_t value = 1;
    };
    template<>
    struct TypeId<Bar> {
        static const uint16_t value = 2;
    };
}

uint16_t Foo::GetTypeId() const { return xx::TypeId_v<Foo>; }

void Foo::Serialize(xx::DataWriterEx &dw) const {
    this->BaseType::Serialize(dw);
    dw.Write(i, name);
}

int Foo::Deserialize(xx::DataReaderEx &dr) {
    if (int r = this->BaseType::Deserialize(dr)) return r;
    return dr.Read(i, name);
}

void Foo::ToString(std::string &s) const {
    xx::Append(s, "{\"typeId\":", GetTypeId());
    this->BaseType::ToStringCore(s);
    ToStringCore(s);
    s.push_back('}');
}

bool Foo::ToStringCore(std::string &s) const {
    ::xx::Append(s, ",\"i\":", i);
    ::xx::Append(s, ",\"name\":", name);
    return true;
}

uint16_t Bar::GetTypeId() const { return xx::TypeId_v<Bar>; }

void Bar::Serialize(xx::DataWriterEx &dw) const {
    this->BaseType::Serialize(dw);
    dw.Write(x, y);
}

int Bar::Deserialize(xx::DataReaderEx &dr) {
    if (int r = this->BaseType::Deserialize(dr)) return r;
    return dr.Read(x, y);
}

void Bar::ToString(std::string &s) const {
    xx::Append(s, "{\"typeId\":", GetTypeId());
    this->BaseType::ToStringCore(s);
    ToStringCore(s);
    s.push_back('}');
}

bool Bar::ToStringCore(std::string &s) const {
    ::xx::Append(s, ",\"x\":", x);
    ::xx::Append(s, ",\"y\":", y);
    return true;
}


int main() {
    Bar b;
    xx::CoutN(b);

    xx::Data d;
    xx::DataWriterEx dw(d);
    dw.WriteOnce(b);

    Bar b2;
    b2.i = 0;
    b2.x = 0;
    b2.y = 0;
    b2.name.clear();
    xx::CoutN(b2);

    xx::DataReaderEx dr(d);
    int r = dr.ReadOnce(b2);
    assert(!r);
    xx::CoutN(b2);
    xx::CoutN(d);

    return 0;
}
