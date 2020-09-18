#include "Objs_class_lite.h"
#include "Objs_class_lite.cpp.inc"
namespace Objs {
	void PkgGenTypes::RegisterTo(xx::ObjectHelper& oh) {
	    oh.Register<Objs::CppFish>();
	}
}

namespace xx {
}
namespace Objs {
    FishBase::FishBase(FishBase&& o) noexcept {
        this->operator=(std::move(o));
    }
    FishBase& FishBase::operator=(FishBase&& o) noexcept {
        std::swap(this->n, o.n);
        return *this;
    }
    void FishBase::Clone1(xx::ObjectHelper &oh, std::shared_ptr<Object> const &tar) const {
        auto&& o = xx::As<Objs::FishBase>(tar);
        xx::CloneFuncs<int32_t>::Clone1(oh, this->n, o->n);
    }
    void FishBase::Clone2(xx::ObjectHelper &oh, std::shared_ptr<Object> const &tar) const {
        auto&& o = xx::As<Objs::FishBase>(tar);
        xx::CloneFuncs<int32_t>::Clone2(oh, this->n, o->n);
    }
    uint16_t FishBase::GetTypeId() const {
        return xx::TypeId_v<Objs::FishBase>;
    }
    void FishBase::Serialize(xx::DataWriterEx& dw) const {
        dw.Write(this->n);
    }
    int FishBase::Deserialize(xx::DataReaderEx& dr) {
        if (int r = dr.Read(this->n)) return r;
        return 0;
    }
    void FishBase::ToString(xx::ObjectHelper &oh) const {
        auto&& iter = oh.objOffsets.find((void*)this);
        if (iter != oh.objOffsets.end()) {
        	xx::AppendEx(oh, iter->second);
        	return;
        }
        else {
            oh.objOffsets[(void*)this] = oh.s.size();
        }
        xx::AppendEx(oh, "{\"#\":", GetTypeId());
        ToStringCore(oh);
        oh.s.push_back('}');
    }
    void FishBase::ToStringCore(xx::ObjectHelper &oh) const {
        xx::AppendEx(oh, ",\"n\":", this->n);
    }
    CppFish::CppFish(CppFish&& o) noexcept {
        this->operator=(std::move(o));
    }
    CppFish& CppFish::operator=(CppFish&& o) noexcept {
        this->BaseType::operator=(std::move(o));
        return *this;
    }
    void CppFish::Clone1(xx::ObjectHelper &oh, std::shared_ptr<Object> const &tar) const {
        this->BaseType::Clone1(oh, tar);
        auto&& o = xx::As<Objs::CppFish>(tar);
    }
    void CppFish::Clone2(xx::ObjectHelper &oh, std::shared_ptr<Object> const &tar) const {
        this->BaseType::Clone2(oh, tar);
        auto&& o = xx::As<Objs::CppFish>(tar);
    }
    uint16_t CppFish::GetTypeId() const {
        return xx::TypeId_v<Objs::CppFish>;
    }
    void CppFish::Serialize(xx::DataWriterEx& dw) const {
        this->BaseType::Serialize(dw);
    }
    int CppFish::Deserialize(xx::DataReaderEx& dr) {
        if (int r = this->BaseType::Deserialize(dr)) return r;
        return 0;
    }
    void CppFish::ToString(xx::ObjectHelper &oh) const {
        auto&& iter = oh.objOffsets.find((void*)this);
        if (iter != oh.objOffsets.end()) {
        	xx::AppendEx(oh, iter->second);
        	return;
        }
        else {
            oh.objOffsets[(void*)this] = oh.s.size();
        }
        xx::AppendEx(oh, "{\"#\":", GetTypeId());
        ToStringCore(oh);
        oh.s.push_back('}');
    }
    void CppFish::ToStringCore(xx::ObjectHelper &oh) const {
        this->BaseType::ToStringCore(oh);
    }
    LuaFishBase::LuaFishBase(LuaFishBase&& o) noexcept {
        this->operator=(std::move(o));
    }
    LuaFishBase& LuaFishBase::operator=(LuaFishBase&& o) noexcept {
        this->BaseType::operator=(std::move(o));
        std::swap(this->fileName, o.fileName);
        return *this;
    }
    void LuaFishBase::Clone1(xx::ObjectHelper &oh, std::shared_ptr<Object> const &tar) const {
        this->BaseType::Clone1(oh, tar);
        auto&& o = xx::As<Objs::LuaFishBase>(tar);
        xx::CloneFuncs<std::string>::Clone1(oh, this->fileName, o->fileName);
    }
    void LuaFishBase::Clone2(xx::ObjectHelper &oh, std::shared_ptr<Object> const &tar) const {
        this->BaseType::Clone2(oh, tar);
        auto&& o = xx::As<Objs::LuaFishBase>(tar);
        xx::CloneFuncs<std::string>::Clone2(oh, this->fileName, o->fileName);
    }
    uint16_t LuaFishBase::GetTypeId() const {
        return xx::TypeId_v<Objs::LuaFishBase>;
    }
    void LuaFishBase::Serialize(xx::DataWriterEx& dw) const {
        this->BaseType::Serialize(dw);
        dw.Write(this->fileName);
    }
    int LuaFishBase::Deserialize(xx::DataReaderEx& dr) {
        if (int r = this->BaseType::Deserialize(dr)) return r;
        if (int r = dr.Read(this->fileName)) return r;
        return 0;
    }
    void LuaFishBase::ToString(xx::ObjectHelper &oh) const {
        auto&& iter = oh.objOffsets.find((void*)this);
        if (iter != oh.objOffsets.end()) {
        	xx::AppendEx(oh, iter->second);
        	return;
        }
        else {
            oh.objOffsets[(void*)this] = oh.s.size();
        }
        xx::AppendEx(oh, "{\"#\":", GetTypeId());
        ToStringCore(oh);
        oh.s.push_back('}');
    }
    void LuaFishBase::ToStringCore(xx::ObjectHelper &oh) const {
        this->BaseType::ToStringCore(oh);
        xx::AppendEx(oh, ",\"fileName\":", this->fileName);
    }
}
