#include "FF_class_lite.h"
#include "FF_class_lite.cpp.inc"
namespace FF {
	void PkgGenTypes::RegisterTo(xx::ObjManager& om) {
	    om.Register<FF::A>();
	}
}

namespace xx {
}
namespace FF {
    A::A(A&& o) noexcept {
        this->operator=(std::move(o));
    }
    A& A::operator=(A&& o) noexcept {
        std::swap(this->id, o.id);
        std::swap(this->parent, o.parent);
        std::swap(this->children, o.children);
        return *this;
    }
    void A::Write(xx::ObjManager& om) const {
        om.Write(this->id);
        om.Write(this->parent);
        om.Write(this->children);
    }
    int A::Read(xx::ObjManager& om) {
        if (int r = om.Read(this->id)) return r;
        if (int r = om.Read(this->parent)) return r;
        if (int r = om.Read(this->children)) return r;
        return 0;
    }
    void A::ToString(xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.Append("}");
    }
    void A::ToStringCore(xx::ObjManager& om) const {
        om.Append(",\"id\":", this->id);
        om.Append(",\"parent\":", this->parent);
        om.Append(",\"children\":", this->children);
    }
    void A::Clone1(xx::ObjManager &om, void* const &tar) const {
        auto out = (FF::A*)tar;
        om.Clone1(this->id, out->id);
        om.Clone1(this->parent, out->parent);
        om.Clone1(this->children, out->children);
    }
    void A::Clone2(xx::ObjManager &om, void* const &tar) const {
        auto out = (FF::A*)tar;
        om.Clone2(this->id, out->id);
        om.Clone2(this->parent, out->parent);
        om.Clone2(this->children, out->children);
    }
    void A::RecursiveReset(xx::ObjManager &om) {
        om.RecursiveReset(this->id);
        om.RecursiveReset(this->parent);
        om.RecursiveReset(this->children);
    }
}
