﻿#pragma once
#include "xx_ptr.h"
#include "xx_data.h"
#include "xx_string.h"

namespace xx {

    struct PtrWriter {
        std::vector<void*> ptrs;
        size_t baseLen = 0;
        xx::Data* data = nullptr;

        static PtrWriter& Get(xx::Data const& d) {
            return *(PtrWriter*)d.ud;
        }

        template<typename T>
        void Write(xx::Data& d, xx::Shared<T> const& v) {
            d.ud = this;
            data = &d;
            baseLen = d.len;
            assert(ptrs.empty());

            (*this)(v);

            for (auto&& p : ptrs) {
                *(int*)p = 0;
            }
            ptrs.clear();
        }

        template<typename T>
        PtrWriter& operator()(T const& v) {
            auto& d = *data;
            if constexpr (IsPtrShared_v<T>) {
                auto typeId = v.typeId();
                d.WriteVarIntger(typeId);
                if (typeId == 0) return *this;

                auto&& h = xx::GetPtrHeader(v.pointer);
                if (h.offset == 0) {
                    h.offset = (uint32_t)(d.len - baseLen);
                    ptrs.push_back(&h.offset);
                    d.WriteVarIntger(h.offset);
                    v.pointer->Write(d);
                }
                else {
                    d.WriteVarIntger(h.offset);
                }
            }
            else if constexpr (IsPtrWeak_v<T>) {
                (*this)(v.Lock());
            }
            else if constexpr (IsVector_v<T>) {
                d.WriteVarIntger(v.size());
                for (auto&& o : v) {
                    (*this)(o);
                }
            }
            else if constexpr (std::is_same_v<std::decay_t<T>, std::string> || std::is_same_v<std::decay_t<T>, std::string_view>) {
                d.WriteVarIntger(v.size());
                d.WriteBuf(v.data(), v.size());
            }
            else if constexpr (std::is_integral_v<T>) {
                d.WriteVarIntger(v);
            }
            else if constexpr (std::is_enum_v<T>) {
                d.WriteVarIntger(std::underlying_type_t<T>(v));
            }
            // else if constexpr (HasMember_Write todo for normal struct support?
            else if constexpr (IsPod_v<T>) {
                d.WriteFixed(v);
            }
            else {
                static_assert(false);
            }
            return *this;
        }
    };
}
