#include "xx_object.h"
namespace PKG {
    struct PkgGenMd5 {
        inline static const std::string value = "#*MD5<65502dc9d3b31ea59b7193aec8366d8e>*#";
    };

    struct C;
    struct D;
    struct A {
        XX_GENCODE_STRUCT_H(A)
        int32_t x = 0;
        int32_t y = 0;
        std::shared_ptr<PKG::C> c;
    };
    struct B : PKG::A {
        XX_GENCODE_STRUCT_H(B)
        int32_t z = 0;
        std::weak_ptr<PKG::C> wc;
    };
    struct C : xx::Object {
        XX_GENCODE_OBJECT_H(C, xx::Object)
        PKG::A a;
        PKG::B b;
    };
    struct D : PKG::C {
        XX_GENCODE_OBJECT_H(D, PKG::C)
        std::string name;
    };
}
namespace xx {
    template<>
    struct DataFuncsEx<PKG::A, void> {
        static void Write(DataWriter& dw, PKG::A const& in);
        static int Read(DataReader& dr, PKG::A& out);
    };
    template<>
    struct DataFuncsEx<PKG::B, void> {
        static void Write(DataWriter& dw, PKG::B const& in);
        static int Read(DataReader& dr, PKG::B& out);
    };
}


/***********************************************************************/

namespace PKG {
    A::A(A&& o) {
        this->operator=(std::move(o));
    }
    A& A::operator=(A&& o) {
        std::swap(this->x, o.x);
        std::swap(this->y, o.y);
        std::swap(this->c, o.c);
        return *this;
    }
    B::B(B&& o) {
        this->operator=(std::move(o));
    }
    B& B::operator=(B&& o) {
        this->PKG::A::operator=(std::move(o));
        std::swap(this->z, o.z);
        std::swap(this->wc, o.wc);
        return *this;
    }
    C::C(C&& o) {
        this->operator=(std::move(o));
    }
    C& C::operator=(C&& o) {
        std::swap(this->a, o.a);
        std::swap(this->b, o.b);
        return *this;
    }
    uint16_t C::GetTypeId() const {
        return xx::TypeId_v<PKG::C>;
    }
    void C::Serialize(xx::DataWriterEx& dw) const {
        dw.Write(this->a);
        dw.Write(this->b);
    }
    int C::Deserialize(xx::DataReaderEx& dr) {
        if (int r = dr.Read(this->a)) return r;
        if (int r = dr.Read(this->b)) return r;
        return 0;
    }
    void C::ToString(std::string& s) const {
        if (this->toStringFlag) {
            xx::Append(s, "[\"***** recursived *****\"]");
            return;
        }
        else {
            ((C*)this)->toStringFlag = true;
        }
        xx::Append(s, "{\"structTypeId\":", GetTypeId());
        ToStringCore(s);
        s.push_back('}');
        ((C*)this)->toStringFlag = false;
    }
    void C::ToStringCore(std::string& s) const {
        xx::Append(s, ",\"a\":", this->a);
        xx::Append(s, ",\"b\":", this->b);
    }
    D::D(D&& o) {
        this->operator=(std::move(o));
    }
    D& D::operator=(D&& o) {
        this->PKG::C::operator=(std::move(o));
        std::swap(this->name, o.name);
        return *this;
    }
    uint16_t D::GetTypeId() const {
        return xx::TypeId_v<PKG::D>;
    }
    void D::Serialize(xx::DataWriterEx& dw) const {
        this->BaseType::Serialize(dw);
        dw.Write(this->name);
    }
    int D::Deserialize(xx::DataReaderEx& dr) {
        if (int r = this->BaseType::Deserialize(dr)) return r;
        if (int r = dr.Read(this->name)) return r;
        return 0;
    }
    void D::ToString(std::string& s) const {
        if (this->toStringFlag) {
            xx::Append(s, "[\"***** recursived *****\"]");
            return;
        }
        else {
            ((D*)this)->toStringFlag = true;
        }
        xx::Append(s, "{\"structTypeId\":", GetTypeId());
        ToStringCore(s);
        s.push_back('}');
        ((D*)this)->toStringFlag = false;
    }
    void D::ToStringCore(std::string& s) const {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ",\"name\":", this->name);
    }
}
namespace xx {
    void DataFuncsEx<PKG::A, void>::Write(DataWriter& dw, PKG::A const& in) {
        dw.Write(in.x);
        dw.Write(in.y);
        dw.Write(in.c);
    }
    int DataFuncsEx<PKG::A, void>::Read(DataReader& d, PKG::A& out) {
        if (int r = d.Read(out.x)) return r;
        if (int r = d.Read(out.y)) return r;
        if (int r = d.Read(out.c)) return r;
        return 0;
    }
    void DataFuncsEx<PKG::B, void>::Write(DataWriter& dw, PKG::B const& in) {
        DataFuncsEx<PKG::A>::Write(dw, in);
        dw.Write(in.z);
        dw.Write(in.wc);
    }
    int DataFuncsEx<PKG::B, void>::Read(DataReader& d, PKG::B& out) {
        if (int r = DataFuncsEx<PKG::A>::Read(d, out)) return r;
        if (int r = d.Read(out.z)) return r;
        if (int r = d.Read(out.wc)) return r;
        return 0;
    }
}




int main() {
    PKG::C c;
}



//struct Foo : xx::Object {
//    using BaseType = xx::Object;
//
//    XX_OBJECT_OVERRIDES_H
//
//    int i = 1;
//    std::string name = "asdf";
//};
//
//struct Bar : Foo {
//    using BaseType = Foo;
//
//    XX_OBJECT_OVERRIDES_H
//
//    float x = 1.2, y = 3.4;
//};
//
//namespace xx {
//    template<>
//    struct TypeId<Foo> {
//        static const uint16_t value = 1;
//    };
//    template<>
//    struct TypeId<Bar> {
//        static const uint16_t value = 2;
//    };
//}
//
//uint16_t Foo::GetTypeId() const { return xx::TypeId_v<Foo>; }
//
//void Foo::Serialize(xx::DataWriterEx &dw) const {
//    this->BaseType::Serialize(dw);
//    dw.Write(i, name);
//}
//
//int Foo::Deserialize(xx::DataReaderEx &dr) {
//    if (int r = this->BaseType::Deserialize(dr)) return r;
//    return dr.Read(i, name);
//}
//
//void Foo::ToString(std::string &s) const {
//    xx::Append(s, "{\"typeId\":", GetTypeId());
//    this->BaseType::ToStringCore(s);
//    ToStringCore(s);
//    s.push_back('}');
//}
//
//bool Foo::ToStringCore(std::string &s) const {
//    ::xx::Append(s, ",\"i\":", i);
//    ::xx::Append(s, ",\"name\":", name);
//    return true;
//}
//
//uint16_t Bar::GetTypeId() const { return xx::TypeId_v<Bar>; }
//
//void Bar::Serialize(xx::DataWriterEx &dw) const {
//    this->BaseType::Serialize(dw);
//    dw.Write(x, y);
//}
//
//int Bar::Deserialize(xx::DataReaderEx &dr) {
//    if (int r = this->BaseType::Deserialize(dr)) return r;
//    return dr.Read(x, y);
//}
//
//void Bar::ToString(std::string &s) const {
//    xx::Append(s, "{\"typeId\":", GetTypeId());
//    this->BaseType::ToStringCore(s);
//    ToStringCore(s);
//    s.push_back('}');
//}
//
//bool Bar::ToStringCore(std::string &s) const {
//    ::xx::Append(s, ",\"x\":", x);
//    ::xx::Append(s, ",\"y\":", y);
//    return true;
//}
//
//
//int main() {
//    Bar b;
//    xx::CoutN(b);
//
//    xx::Data d;
//    xx::DataWriterEx dw(d);
//    dw.WriteOnce(b);
//
//    Bar b2;
//    b2.i = 0;
//    b2.x = 0;
//    b2.y = 0;
//    b2.name.clear();
//    xx::CoutN(b2);
//
//    xx::DataReaderEx dr(d);
//    int r = dr.ReadOnce(b2);
//    assert(!r);
//    xx::CoutN(b2);
//    xx::CoutN(d);
//
//    return 0;
//}
