#include "FF_class_lite.h"
#include "FF_class_lite.cpp.inc"
namespace FF {
	void PkgGenTypes::RegisterTo(::xx::ObjManager& om) {
	    om.Register<::FF::B>();
	    om.Register<::FF::A>();
	}
}

namespace xx {
	void ObjFuncs<::FF::C, void>::Write(::xx::ObjManager& om, ::FF::C const& in) {
        om.Write(in.x);
        om.Write(in.y);
        om.Write(in.targets);
    }
	int ObjFuncs<::FF::C, void>::Read(::xx::ObjManager& om, ::FF::C& out) {
        if (int r = om.Read(out.x)) return r;
        if (int r = om.Read(out.y)) return r;
        if (int r = om.Read(out.targets)) return r;
        return 0;
    }
	void ObjFuncs<::FF::C, void>::ToString(ObjManager &om, ::FF::C const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::C, void>::ToStringCore(ObjManager &om, ::FF::C const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"x\":", in.x); 
        om.Append(",\"y\":", in.y);
        om.Append(",\"targets\":", in.targets);
    }
    void ObjFuncs<::FF::C>::Clone1(::xx::ObjManager& om, ::FF::C const& in, ::FF::C &out) {
        om.Clone1(in.x, out.x);
        om.Clone1(in.y, out.y);
        om.Clone1(in.targets, out.targets);
    }
    void ObjFuncs<::FF::C>::Clone2(::xx::ObjManager& om, ::FF::C const& in, ::FF::C &out) {
        om.Clone2(in.x, out.x);
        om.Clone2(in.y, out.y);
        om.Clone2(in.targets, out.targets);
    }
    void ObjFuncs<::FF::C>::RecursiveReset(::xx::ObjManager& om, ::FF::C& in) {
        om.RecursiveReset(in.x);
        om.RecursiveReset(in.y);
        om.RecursiveReset(in.targets);
    }
}
namespace FF {
    A::A(A&& o) noexcept {
        this->operator=(std::move(o));
    }
    A& A::operator=(A&& o) noexcept {
        std::swap(this->id, o.id);
        std::swap(this->nick, o.nick);
        std::swap(this->parent, o.parent);
        std::swap(this->children, o.children);
        return *this;
    }
    void A::Write(::xx::ObjManager& om) const {
        om.Write(this->id);
        om.Write(this->nick);
        om.Write(this->parent);
        om.Write(this->children);
    }
    int A::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->id)) return r;
        if (int r = om.Read(this->nick)) return r;
        if (int r = om.Read(this->parent)) return r;
        if (int r = om.Read(this->children)) return r;
        return 0;
    }
    void A::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void A::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"id\":", this->id);
        om.Append(",\"nick\":", this->nick);
        om.Append(",\"parent\":", this->parent);
        om.Append(",\"children\":", this->children);
    }
    void A::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::A*)tar;
        om.Clone1(this->id, out->id);
        om.Clone1(this->nick, out->nick);
        om.Clone1(this->parent, out->parent);
        om.Clone1(this->children, out->children);
    }
    void A::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::A*)tar;
        om.Clone2(this->id, out->id);
        om.Clone2(this->nick, out->nick);
        om.Clone2(this->parent, out->parent);
        om.Clone2(this->children, out->children);
    }
    void A::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->id);
        om.RecursiveReset(this->nick);
        om.RecursiveReset(this->parent);
        om.RecursiveReset(this->children);
    }
    C::C(C&& o) noexcept {
        this->operator=(std::move(o));
    }
    C& C::operator=(C&& o) noexcept {
        std::swap(this->x, o.x);
        std::swap(this->y, o.y);
        std::swap(this->targets, o.targets);
        return *this;
    }
    B::B(B&& o) noexcept {
        this->operator=(std::move(o));
    }
    B& B::operator=(B&& o) noexcept {
        this->BaseType::operator=(std::move(o));
        std::swap(this->data, o.data);
        std::swap(this->c, o.c);
        std::swap(this->c2, o.c2);
        std::swap(this->c3, o.c3);
        return *this;
    }
    void B::Write(::xx::ObjManager& om) const {
        this->BaseType::Write(om);
        om.Write(this->data);
        om.Write(this->c);
        om.Write(this->c2);
        om.Write(this->c3);
    }
    int B::Read(::xx::ObjManager& om) {
        if (int r = this->BaseType::Read(om)) return r;
        if (int r = om.Read(this->data)) return r;
        if (int r = om.Read(this->c)) return r;
        if (int r = om.Read(this->c2)) return r;
        if (int r = om.Read(this->c3)) return r;
        return 0;
    }
    void B::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
        this->BaseType::ToStringCore(om);
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void B::ToStringCore(::xx::ObjManager& om) const {
        this->BaseType::ToStringCore(om);
        om.Append(",\"data\":", this->data);
        om.Append(",\"c\":", this->c);
        om.Append(",\"c2\":", this->c2);
        om.Append(",\"c3\":", this->c3);
    }
    void B::Clone1(::xx::ObjManager& om, void* const &tar) const {
        this->BaseType::Clone1(om, tar);
        auto out = (::FF::B*)tar;
        om.Clone1(this->data, out->data);
        om.Clone1(this->c, out->c);
        om.Clone1(this->c2, out->c2);
        om.Clone1(this->c3, out->c3);
    }
    void B::Clone2(::xx::ObjManager& om, void* const &tar) const {
        this->BaseType::Clone2(om, tar);
        auto out = (::FF::B*)tar;
        om.Clone2(this->data, out->data);
        om.Clone2(this->c, out->c);
        om.Clone2(this->c2, out->c2);
        om.Clone2(this->c3, out->c3);
    }
    void B::RecursiveReset(::xx::ObjManager& om) {
        this->BaseType::RecursiveReset(om);
        om.RecursiveReset(this->data);
        om.RecursiveReset(this->c);
        om.RecursiveReset(this->c2);
        om.RecursiveReset(this->c3);
    }
}
