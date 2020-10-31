#pragma once
#include "xx_ptr.h"

namespace xx {

    // 将就使用 PtrHeader 的 typeId, offset 来存放 cap, len. 4G 限制 的 类似 xx::Data 的数据容器
    struct SharedBytes : Shared<uint8_t> {

        inline uint32_t len() const {
            if (!pointer) return 0;
            return PtrHeader::Get(pointer).offset;
        }

        inline uint32_t cap() const {
            if (!pointer) return 0;
            return PtrHeader::Get(pointer).typeId;
        }

        // 申请空间. 原 buf 的内容会移动到新 buf. 原 buf 长度清 0, 减持 甚至 释放
        inline void Reserve(uint32_t cap_) {
            if (!cap_) {
                Clear();
                return;
            }
            if (!pointer) {
                pointer = NewBuf(cap_);
                return;
            }
            auto c = cap();
            if (cap_ <= c) return;
            if (cap_ < c * 2) {
                cap_ = c * 2;
            }
            auto buf = NewBuf(cap_); // , header().len
            if (auto len_ = len()) {
                memcpy(buf, pointer, len_ * sizeof(ElementType));
                PtrHeader::Get(buf).offset = len_;
            }
            PtrHeader::Get(pointer).offset = 0;
            Reset();
            pointer = buf;
        }

        // 修改长度 并 返回旧长度. 可能触发 申请空间及其副作用
        inline uint32_t Resize(uint32_t const& siz) {
            auto len_ = len();
            if (siz && !pointer) {
                pointer = NewBuf(siz);
            } else if (siz > len_) {
                Reserve(siz);
            }
            PtrHeader::Get(pointer).offset = siz;
            return len_;
        }

        // 长度清 0
        inline void Clear() {
            if (pointer) {
                PtrHeader::Get(pointer).offset = 0;
            }
        }

        // 下标访问
        inline ElementType const& operator[](uint32_t const& idx) const {
            if (!pointer || len() <= idx) throw std::out_of_range("out of range");
            return pointer[idx];
        }

        inline ElementType& operator[](uint32_t const& idx) {
            if (!pointer || len() <= idx) throw std::out_of_range("out of range");
            return pointer[idx];
        }

        // 从头部移除指定长度数据( 常见于拆包处理移除掉已经访问过的包数据, 将残留部分移动到头部 )
        inline void RemoveFront(uint32_t const& siz) {
            if (!siz) return;
            if (siz > len()) throw std::out_of_range("out of range");
            auto& len_ = PtrHeader::Get(pointer).offset;
            len_ -= siz;
            if (len_) {
                ::memmove(pointer, pointer + siz, len_);
            }
        }

        // 追加写入一段 buf
        inline void WriteBuf(void const* const& ptr, uint32_t const& siz) {
            Reserve(len() + siz);
            auto& len_ = PtrHeader::Get(pointer).offset;
            ::memcpy(pointer + len_, ptr, siz);
            len_ += siz;
        }

        // 追加写入一段 pod 结构内存
        template<typename T, typename ENABLED = std::enable_if_t<std::is_standard_layout_v<T> && std::is_trivial_v<T>>>
        void WriteFixed(T const& v) {
            WriteBuf(&v, sizeof(T));
        }

        inline static uint16_t ZigZagEncode(int16_t const& in) noexcept {
            return (uint16_t)((in << 1) ^ (in >> 15));
        }
        inline static uint32_t ZigZagEncode(int32_t const& in) noexcept {
            return (in << 1) ^ (in >> 31);
        }
        inline static uint64_t ZigZagEncode(int64_t const& in) noexcept {
            return (in << 1) ^ (in >> 63);
        }

        // 追加写入整数( 7bit 变长格式 )
        template<typename T, bool needReserve = true, typename ENABLED = std::enable_if_t<std::is_integral_v<T>>>
        void WriteVarIntger(T const& v) {
            using UT = std::make_unsigned_t<T>;
            UT u(v);
            if constexpr (std::is_signed_v<T>) {
                u = ZigZagEncode(v);
            }
            if constexpr (needReserve) {
                Reserve(len() + sizeof(T) + 1);
            }
            auto& len_ = PtrHeader::Get(pointer).offset;
            while (u >= 1 << 7) {
                pointer[len_++] = char((u & 0x7fu) | 0x80u);
                u = UT(u >> 7);
            };
            pointer[len_++] = char(u);
        }

    protected:
        inline ElementType* NewBuf(uint32_t const& newCap) {
            auto h = (PtrHeader*)malloc(sizeof(PtrHeader) + sizeof(ElementType) * newCap);
            if (!h) throw std::runtime_error("not enough memory");
            h->useCount = 1;
            h->refCount = 0;
            h->typeId = newCap;
            h->offset = 0;
            return (ElementType*)(h + 1);
        }
    };
}
