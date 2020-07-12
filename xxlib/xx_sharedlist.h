#pragma once

#include "xx_typehelpers.h"
#include <cassert>
#include <cstring>

namespace xx {

    // 内存平铺版 std::shared_ptr<std::vector>. 复制规则变为复制指针增加引用计数
    // 注意：任何 添加 操作都可能导致 buf 新建，从而和之前引用到的 buf 脱钩( 如果有的话 )
    // 特别适合一次创建 & 填充，多次只读的场景
    // 如果必须慢慢添加，则可以初始化一个较大的 cap，避免 realloc buf 情况的出现
    template<typename T>
    struct SharedList {
        // receive len
        static const size_t headerLen = sizeof(size_t) * 4;

        // recv + (buf)T...........
        T *buf = nullptr;

        inline size_t const &len() const {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[0];
        }

        inline size_t const &cap() const {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[1];
        }

        inline size_t const &useCount() const {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[2];
        }

        inline size_t const &unused() const {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[3];
        }

        inline size_t &len() {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[0];
        }

        inline size_t &cap() {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[1];
        }

        inline size_t &useCount() {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[2];
        }

        inline size_t &unused() {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[3];
        }

        explicit SharedList(size_t const &cap_ = 0) {
            if (cap_) {
                Reserve(cap_);
            }
        }

        SharedList(SharedList const &o) {
            buf = o.buf;
            ++useCount();
        }

        SharedList(SharedList &&o) : buf(o.buf) {
            o.buf = nullptr;
        }

        SharedList &operator=(SharedList const &o) {
            if (this != &o) {
                Free();
                Alloc(o.len());
            }
            return *this;
        }

        SharedList &operator=(SharedList &&o)  {
            std::swap(buf, o.buf);
        }

        ~SharedList() {
            Free();
        }


        // 申请空间. 原buf 的内容会移动到新 buf. 原buf 长度清0, 减持 甚至 释放
        inline void Reserve(size_t cap_) {
            assert(cap_);
            if (buf) {
                if (cap_ <= cap()) return;
                else {
                    auto siz = len();
                    if (cap_ < cap() * 2) {
                        cap_ = cap() * 2;
                    }
                    auto newBuf = NewBuf(cap_, siz);
                    if constexpr (IsPod_v<T>) {
                        memcpy((void *) newBuf, (void *) buf, siz * sizeof(T));
                    } else {
                        for (size_t i = 0; i < siz; ++i) {
                            new(&newBuf[i]) T((T &&) buf[i]);
                        }
                    }
                    len() = 0;
                    Free();
                    buf = newBuf;
                }
            } else {
                buf = NewBuf(cap_);
            }
        }

        // 长度清 0
        inline void Clear() {
            if constexpr (!std::is_pod_v<T>) {
                auto siz = len();
                for (size_t i = 0; i < siz; ++i) {
                    buf[i].~T();
                }
            }
            len() = 0;
        }

        // 减持 甚至 释放 buf
        inline void Free() {
            if (buf) {
                if (--useCount() == 0) {
                    Clear();
                    free((void *) ((char *) buf - headerLen));
                }
                buf = nullptr;
            }
        }

        // 修改长度. 可能触发 申请空间及其副作用
        size_t Resize(size_t const& siz)  {
            if (siz && !buf) {
                buf = NewBuf(siz);
            }
            if (siz == len()) return siz;
            else if (siz < len()) {
                if constexpr (!std::is_pod_v<T>) {
                    for (size_t i = siz; i < len(); ++i) {
                        buf[i].~T();
                    }
                }
            }
            else {	// len > len()
                Reserve(siz);
                if constexpr (!std::is_pod_v<T>) {
                    for (size_t i = len(); i < siz; ++i) {
                        new (buf + i) T();
                    }
                }
            }
            auto rtv = len();
            len() = siz;
            return rtv;
        }

        // 查找并移除一个元素
        void Remove(T const& v)  {
            assert(buf);
            auto siz = len();
            for (size_t i = 0; i < siz; ++i) {
                if (v == buf[i]) {
                    RemoveAt(i);
                    return;
                }
            }
        }

        // 移除指定下标的元素. 后面的元素依次移动
        void RemoveAt(size_t const& idx)  {
            assert(buf && idx < len());
            --len();
            if constexpr (IsPod_v<T>) {
                buf[idx].~T();
                memmove(buf + idx, buf + idx + 1, (len() - idx) * sizeof(T));
            }
            else {
                for (size_t i = idx; i < len(); ++i) {
                    buf[i] = (T&&)buf[i + 1];
                }
                if constexpr (!std::is_pod_v<T>) {
                    buf[len()].~T();
                }
            }
        }

        // 和最后一个元素做交换删除.
        // 通常环境为随机访问或 倒循环扫描 if (list.len) { for (auto i = list.len - 1; i != (size_t)-1; --i) { ...
        // 通常上一句为 list[list.len - 1]->idx = o->idx;
        void SwapRemoveAt(size_t const& idx)  {
            if (idx + 1 < len()) {
                std::swap(buf[idx], buf[len() - 1]);
            }
            --len();
            if constexpr (!std::is_pod_v<T>) {
                buf[len()].~T();
            }
        }


        // 用 T 的一到多个构造函数的参数来追加构造一个 item
        template<typename...Args>
        T& Emplace(Args&&...args)  {
            Reserve(len() + 1);
            return *new (&buf[len()++]) T(std::forward<Args>(args)...);
        }

        // 用 T 的一到多个构造函数的参数来构造一个 item 插入到指定下标. 下标越界等同于追加
        template<typename...Args>
        T& EmplaceAt(size_t const& idx, Args&&...args)  {
            Reserve(len() + 1);
            if (idx < len()) {
                if constexpr (IsPod_v<T>) {
                    memmove(buf + idx + 1, buf + idx, (len() - idx) * sizeof(T));
                }
                else {
                    new (buf + len()) T((T&&)buf[len() - 1]);
                    for (size_t i = len() - 1; i > idx; --i) {
                        buf[i] = (T&&)buf[i - 1];
                    }
                    if constexpr (!std::is_pod_v<T>) {
                        buf[idx].~T();
                    }
                }
            }
            else idx = len();
            ++len();
            new (buf + idx) T(std::forward<Args>(args)...);
            return buf[idx];
        }

        // 与 Emplace 不同的是, 这个仅支持1个参数的构造函数, 可一次追加多个
        template<typename ...TS>
        void Add(TS&&...vs)  {
            std::initializer_list<int> n{ (Emplace(std::forward<TS>(vs)), 0)... };
            (void)(n);
        }

        // 批量添加
        void AddRange(T const* const& items, size_t const& count) {
            Reserve(len() + count);
            if constexpr (IsPod_v<T>) {
                memcpy(buf + len(), items, count * sizeof(T));
            }
            else {
                for (size_t i = 0; i < count; ++i) {
                    new (&buf[len() + i]) T((T&&)items[i]);
                }
            }
            len() += count;
        }

        void AddRange(SharedList<T> const& list) {
            return AddRange(list.buf, list.len);
        }

        // 如果找到就返回索引. 找不到将返回 size_t(-1)
        size_t Find(T const& v) const {
            auto siz = len();
            for (size_t i = 0; i < siz; ++i) {
                if (v == buf[i]) return i;
            }
            return size_t(-1);
        }


        T const &operator[](size_t const &index) const {
            assert(buf && index < len());
            return buf[index];
        }

        T &operator[](size_t const &index) {
            assert(buf && index < len());
            return buf[index];
        }

        T const& At(size_t const& index) const {
            assert(buf && index < len());
            return buf[index];
        }
        T& At(size_t const& index) {
            assert(buf && index < len());
            return buf[index];
        }

        T& Top() {
            assert(buf && len());
            return buf[len() - 1];
        }
        void Pop() {
            assert(buf && len());
            --len();
            buf[len()].~T();
        }
        T const& Top() const {
            assert(buf && len());
            return buf[len() - 1];
        }
        bool TryPop(T& output) {
            if (!buf || !len()) return false;
            output = (T&&)buf[--len()];
            buf[len()].~T();
            return true;
        }

        // support for( auto&& c : list )
        struct Iter {
            T *ptr;

            bool operator!=(Iter const &other) { return ptr != other.ptr; }

            Iter &operator++() {
                ++ptr;
                return *this;
            }

            T &operator*() { return *ptr; }
        };

        Iter begin() { return Iter{buf}; }

        Iter end() { return Iter{buf + len()}; }

        Iter begin() const { return Iter{buf}; }

        Iter end() const { return Iter{buf + len()}; }

    protected:
        inline T *NewBuf(size_t const &cap, size_t const& len = 0) {
            auto p = (size_t *) malloc(headerLen + sizeof(T) * cap);
            p[0] = len;
            p[1] = cap;
            p[2] = 1;
            p[3] = 0;
            return (T *) &p[4];
        }

        // 简单兼容一下 std::vector
    public:
        inline size_t const &size() const {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[0];
        }
        inline size_t &size() {
            if (!buf) return *(size_t*)&buf;
            return ((size_t *) ((char *) buf - headerLen))[0];
        }
        template<typename...Args>
        T& emplace_back(Args&&...args) {
            return Emplace(args...);
        }
        template<typename U>
        void push_back(U&& v) {
            Add(std::forward<U>(v));
        }
    };
}
