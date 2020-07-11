#include "PKG_class_lite.h"
#include "PKG_class_lite.cpp.inc"
namespace xx {
    template<> struct TypeId<PKG::C> { static const uint16_t value = 12; };
    template<> struct TypeId<PKG::Node> { static const uint16_t value = 78; };
    template<> struct TypeId<PKG::D> { static const uint16_t value = 34; };
    template<> struct TypeId<PKG::Scene> { static const uint16_t value = 56; };
}
namespace PKG {
	void PkgGenTypes::RegisterTo(xx::ObjectCreators& oc) {
	    oc.Register<PKG::C>(12);
	    oc.Register<PKG::D>(34);
	    oc.Register<PKG::Node>(78);
	    oc.Register<PKG::Scene>(56);
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
	void StringFuncs<PKG::A, void>::Append(std::string& s, PKG::A const& in) {
        s.push_back('{');
        AppendCore(s, in);
        s.push_back('}');
    }
	void StringFuncs<PKG::A, void>::AppendCore(std::string& s, PKG::A const& in) {
        auto sizeBak = s.size();
        if (sizeBak == s.size()) {
            s.push_back(',');
        }
        xx::Append(s, "\"x\":", in.x); 
        xx::Append(s, ",\"y\":", in.y);
        xx::Append(s, ",\"c\":", in.c);
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
	void StringFuncs<PKG::B, void>::Append(std::string& s, PKG::B const& in) {
        s.push_back('{');
        AppendCore(s, in);
        s.push_back('}');
    }
	void StringFuncs<PKG::B, void>::AppendCore(std::string& s, PKG::B const& in) {
        auto sizeBak = s.size();
        StringFuncs<PKG::A>::AppendCore(s, in);
        if (sizeBak == s.size()) {
            s.push_back(',');
        }
        xx::Append(s, "\"z\":", in.z); 
        xx::Append(s, ",\"wc\":", in.wc);
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
    void C::ToString(std::string& s) const {
        if (this->toStringFlag) {
        	xx::Append(s, "{}");
        	return;
        }
        else {
            ((C*)this)->toStringFlag = true;
        }
        xx::Append(s, "{\"#\":", GetTypeId());
        ToStringCore(s);
        s.push_back('}');
        ((C*)this)->toStringFlag = false;
    }
    void C::ToStringCore(std::string& s) const {
        xx::Append(s, ",\"a\":", this->a);
        xx::Append(s, ",\"b\":", this->b);
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
    void Node::ToString(std::string& s) const {
        if (this->toStringFlag) {
        	xx::Append(s, "{}");
        	return;
        }
        else {
            ((Node*)this)->toStringFlag = true;
        }
        xx::Append(s, "{\"#\":", GetTypeId());
        ToStringCore(s);
        s.push_back('}');
        ((Node*)this)->toStringFlag = false;
    }
    void Node::ToStringCore(std::string& s) const {
        xx::Append(s, ",\"parent\":", this->parent);
        xx::Append(s, ",\"childs\":", this->childs);
    }
    D::D(D&& o) noexcept {
        this->operator=(std::move(o));
    }
    D& D::operator=(D&& o) noexcept {
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
        	xx::Append(s, "{}");
        	return;
        }
        else {
            ((D*)this)->toStringFlag = true;
        }
        xx::Append(s, "{\"#\":", GetTypeId());
        ToStringCore(s);
        s.push_back('}');
        ((D*)this)->toStringFlag = false;
    }
    void D::ToStringCore(std::string& s) const {
        this->BaseType::ToStringCore(s);
        xx::Append(s, ",\"name\":", this->name);
    }
    Scene::Scene(Scene&& o) noexcept {
        this->operator=(std::move(o));
    }
    Scene& Scene::operator=(Scene&& o) noexcept {
        this->PKG::Node::operator=(std::move(o));
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
    void Scene::ToString(std::string& s) const {
        if (this->toStringFlag) {
        	xx::Append(s, "{}");
        	return;
        }
        else {
            ((Scene*)this)->toStringFlag = true;
        }
        xx::Append(s, "{\"#\":", GetTypeId());
        ToStringCore(s);
        s.push_back('}');
        ((Scene*)this)->toStringFlag = false;
    }
    void Scene::ToStringCore(std::string& s) const {
        this->BaseType::ToStringCore(s);
    }
}
