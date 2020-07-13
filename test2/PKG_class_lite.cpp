#include "PKG_class_lite.h"
#include "PKG_class_lite.cpp.inc"
namespace xx {
    template<> struct TypeId<PKG::C> { static const uint16_t value = 12; };
    template<> struct TypeId<PKG::Node> { static const uint16_t value = 78; };
    template<> struct TypeId<PKG::D> { static const uint16_t value = 34; };
    template<> struct TypeId<PKG::Scene> { static const uint16_t value = 56; };
}
namespace PKG {
	void PkgGenTypes::RegisterTo(xx::ObjectHelper& oh) {
	    oh.Register<PKG::C>();
	    oh.Register<PKG::D>();
	    oh.Register<PKG::Node>();
	    oh.Register<PKG::Scene>();
	}
}

namespace xx {
	void DataFuncsEx<PKG::A, void>::Write(DataWriterEx& dw, PKG::A const& in) {
        dw.Write(in.x);
        dw.Write(in.y);
        dw.Write(in.c);
    }
	int DataFuncsEx<PKG::A, void>::Read(DataReaderEx& d, PKG::A& out) {
        if (int r = d.Read(out.x)) return r;
        if (int r = d.Read(out.y)) return r;
        if (int r = d.Read(out.c)) return r;
        return 0;
    }
	void StringFuncsEx<PKG::A, void>::Append(ObjectHelper &oh, PKG::A const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<PKG::A, void>::AppendCore(ObjectHelper &oh, PKG::A const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"x\":", in.x); 
        xx::AppendEx(oh, ",\"y\":", in.y);
        xx::AppendEx(oh, ",\"c\":", in.c);
    }
	void DataFuncsEx<PKG::B, void>::Write(DataWriterEx& dw, PKG::B const& in) {
        DataFuncsEx<PKG::A>::Write(dw, in);
        dw.Write(in.z);
        dw.Write(in.wc);
    }
	int DataFuncsEx<PKG::B, void>::Read(DataReaderEx& d, PKG::B& out) {
        if (int r = DataFuncsEx<PKG::A>::Read(d, out)) return r;
        if (int r = d.Read(out.z)) return r;
        if (int r = d.Read(out.wc)) return r;
        return 0;
    }
	void StringFuncsEx<PKG::B, void>::Append(ObjectHelper &oh, PKG::B const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<PKG::B, void>::AppendCore(ObjectHelper &oh, PKG::B const& in) {
        auto sizeBak = oh.s.size();
        StringFuncsEx<PKG::A>::AppendCore(oh, in);
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"z\":", in.z); 
        xx::AppendEx(oh, ",\"wc\":", in.wc);
    }
}
namespace PKG {
    A::A(A&& o) noexcept {
        this->operator=(std::move(o));
    }
    A& A::operator=(A&& o) noexcept {
        std::swap(this->x, o.x);
        std::swap(this->y, o.y);
        std::swap(this->c, o.c);
        return *this;
    }
    B::B(B&& o) noexcept {
        this->operator=(std::move(o));
    }
    B& B::operator=(B&& o) noexcept {
        this->PKG::A::operator=(std::move(o));
        std::swap(this->z, o.z);
        std::swap(this->wc, o.wc);
        return *this;
    }
    C::C(C&& o) noexcept {
        this->operator=(std::move(o));
    }
    C& C::operator=(C&& o) noexcept {
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
    void C::ToString(xx::ObjectHelper &oh) const {
        auto&& iter = oh.ptrOffsets.find((void*)this);
        if (iter != oh.ptrOffsets.end()) {
        	xx::AppendEx(oh, iter->second);
        	return;
        }
        else {
            oh.ptrOffsets[(void*)this] = oh.s.size();
        }
        xx::AppendEx(oh, "{\"#\":", GetTypeId());
        ToStringCore(oh);
        oh.s.push_back('}');
    }
    void C::ToStringCore(xx::ObjectHelper &oh) const {
        xx::AppendEx(oh, ",\"a\":", this->a);
        xx::AppendEx(oh, ",\"b\":", this->b);
    }
    Node::Node(Node&& o) noexcept {
        this->operator=(std::move(o));
    }
    Node& Node::operator=(Node&& o) noexcept {
        std::swap(this->parent, o.parent);
        std::swap(this->childs, o.childs);
        return *this;
    }
    uint16_t Node::GetTypeId() const {
        return xx::TypeId_v<PKG::Node>;
    }
    void Node::Serialize(xx::DataWriterEx& dw) const {
        dw.Write(this->parent);
        dw.Write(this->childs);
    }
    int Node::Deserialize(xx::DataReaderEx& dr) {
        if (int r = dr.Read(this->parent)) return r;
        if (int r = dr.Read(this->childs)) return r;
        return 0;
    }
    void Node::ToString(xx::ObjectHelper &oh) const {
        auto&& iter = oh.ptrOffsets.find((void*)this);
        if (iter != oh.ptrOffsets.end()) {
        	xx::AppendEx(oh, iter->second);
        	return;
        }
        else {
            oh.ptrOffsets[(void*)this] = oh.s.size();
        }
        xx::AppendEx(oh, "{\"#\":", GetTypeId());
        ToStringCore(oh);
        oh.s.push_back('}');
    }
    void Node::ToStringCore(xx::ObjectHelper &oh) const {
        xx::AppendEx(oh, ",\"parent\":", this->parent);
        xx::AppendEx(oh, ",\"childs\":", this->childs);
    }
    D::D(D&& o) noexcept {
        this->operator=(std::move(o));
    }
    D& D::operator=(D&& o) noexcept {
        this->BaseType::operator=(std::move(o));
        std::swap(this->name, o.name);
        std::swap(this->desc, o.desc);
        return *this;
    }
    uint16_t D::GetTypeId() const {
        return xx::TypeId_v<PKG::D>;
    }
    void D::Serialize(xx::DataWriterEx& dw) const {
        this->BaseType::Serialize(dw);
        dw.Write(this->name);
        dw.Write(this->desc);
    }
    int D::Deserialize(xx::DataReaderEx& dr) {
        if (int r = this->BaseType::Deserialize(dr)) return r;
        if (int r = dr.Read(this->name)) return r;
        if (int r = dr.Read(this->desc)) return r;
        return 0;
    }
    void D::ToString(xx::ObjectHelper &oh) const {
        auto&& iter = oh.ptrOffsets.find((void*)this);
        if (iter != oh.ptrOffsets.end()) {
        	xx::AppendEx(oh, iter->second);
        	return;
        }
        else {
            oh.ptrOffsets[(void*)this] = oh.s.size();
        }
        xx::AppendEx(oh, "{\"#\":", GetTypeId());
        ToStringCore(oh);
        oh.s.push_back('}');
    }
    void D::ToStringCore(xx::ObjectHelper &oh) const {
        this->BaseType::ToStringCore(oh);
        xx::AppendEx(oh, ",\"name\":", this->name);
        xx::AppendEx(oh, ",\"desc\":", this->desc);
    }
    Scene::Scene(Scene&& o) noexcept {
        this->operator=(std::move(o));
    }
    Scene& Scene::operator=(Scene&& o) noexcept {
        this->BaseType::operator=(std::move(o));
        return *this;
    }
    uint16_t Scene::GetTypeId() const {
        return xx::TypeId_v<PKG::Scene>;
    }
    void Scene::Serialize(xx::DataWriterEx& dw) const {
        this->BaseType::Serialize(dw);
    }
    int Scene::Deserialize(xx::DataReaderEx& dr) {
        if (int r = this->BaseType::Deserialize(dr)) return r;
        return 0;
    }
    void Scene::ToString(xx::ObjectHelper &oh) const {
        auto&& iter = oh.ptrOffsets.find((void*)this);
        if (iter != oh.ptrOffsets.end()) {
        	xx::AppendEx(oh, iter->second);
        	return;
        }
        else {
            oh.ptrOffsets[(void*)this] = oh.s.size();
        }
        xx::AppendEx(oh, "{\"#\":", GetTypeId());
        ToStringCore(oh);
        oh.s.push_back('}');
    }
    void Scene::ToStringCore(xx::ObjectHelper &oh) const {
        this->BaseType::ToStringCore(oh);
    }
}
