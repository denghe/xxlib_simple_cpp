#pragma once

#include "xx_typehelpers.h"

// 类似 std::shared_ptr / weak_ptr, 大 try catch 风格用法( 能捕获 空-> )

namespace xx {

    /************************************************************************************/
    // Make 时会在内存块头部附加

    struct PtrHeader {
        union {
            struct {
                uint32_t useCount;      // 强引用技术
                uint32_t refCount;      // 弱引用技术
            };
            uint64_t data1;
        };
        union {
            struct {
                uint32_t typeId;        // 序列化 或 类型转换用
                uint32_t offset;        // 序列化等过程中使用
            };
            uint64_t data2;
        };

        // unsafe: 从一个类指针反查引用到内存块头的 PtrHeader 类型
        [[maybe_unused]] [[nodiscard]] inline static PtrHeader &Get(void *const &o) {
            return *((PtrHeader *) o - 1);
        };
    };




    /************************************************************************************/
    // std::shared_ptr like

    template<typename T>
    struct Weak;

    template<typename T>
    struct Shared {
        using ElementType = T;
        T *pointer = nullptr;

        T *operator->() const {
            if (!pointer) throw std::runtime_error("exception: pointer == nullptr");
            return pointer;
        }

        T &Value() {
            if (!pointer) throw std::runtime_error("exception: pointer == nullptr");
            return *pointer;
        }

        [[maybe_unused]] [[nodiscard]] operator bool() const noexcept {
            return pointer != nullptr;
        }

        [[maybe_unused]] [[nodiscard]] bool Empty() const noexcept {
            return pointer == nullptr;
        }

        [[maybe_unused]] [[nodiscard]] bool HasValue() const noexcept {
            return pointer != nullptr;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t useCount() const noexcept {
            if (!pointer) return 0;
            return PtrHeader::Get(pointer).useCount;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t refCount() const noexcept {
            if (!pointer) return 0;
            return PtrHeader::Get(pointer).refCount;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t typeId() const noexcept {
            if (!pointer) return 0;
            assert(PtrHeader::Get(pointer).typeId);
            return PtrHeader::Get(pointer).typeId;
        }

        void Reset() {
            if (pointer) {
                auto &&h = PtrHeader::Get(pointer);
                assert(h.useCount);
                --h.useCount;
                if (h.useCount == 0) {
                    auto needFree = h.refCount == 0;
                    pointer->~T();
                    if (needFree) {
                        free(&h);
                    }
                }
                pointer = nullptr;
            }
        }

        template<typename U>
        void Reset(U *const &ptr) {
            static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>);
            if (pointer == ptr) return;
            Reset();
            if (ptr) {
                pointer = ptr;
                auto &&h = PtrHeader::Get(pointer);
                ++h.useCount;
            }
        }

        ~Shared() {
            Reset();
        }

        Shared() = default;

        template<typename U>
        Shared(U *const &ptr) {
            static_assert(std::is_base_of_v<T, U>);
            Reset(ptr);
        }
        Shared(T *const &ptr) {
            Reset(ptr);
        }

        template<typename U>
        Shared(Shared<U> const &o) {
            static_assert(std::is_base_of_v<T, U>);
            Reset(o.pointer);
        }
        Shared(Shared const &o) {
            Reset(o.pointer);
        }

        template<typename U>
        Shared(Shared<U> &&o) noexcept {
            static_assert(std::is_base_of_v<T, U>);
            pointer = o.pointer;
            o.pointer = nullptr;
        }
        Shared(Shared&& o) noexcept {
            pointer = o.pointer;
            o.pointer = nullptr;
        }

        template<typename U>
        Shared &operator=(U *const &ptr) {
            static_assert(std::is_base_of_v<T, U>);
            Reset(ptr);
            return *this;
        }
        Shared &operator=(T *const &ptr) {
            Reset(ptr);
            return *this;
        }

        template<typename U>
        Shared &operator=(Shared<U> const &o) {
            static_assert(std::is_base_of_v<T, U>);
            Reset(o.pointer);
            return *this;
        }
        Shared &operator=(Shared const &o) {
            Reset(o.pointer);
            return *this;
        }

        template<typename U>
        Shared &operator=(Shared<U> &&o) {
            static_assert(std::is_base_of_v<T, U>);
            Reset();
            std::swap(pointer, o.pointer);
            return *this;
        }
        Shared& operator=(Shared&& o) {
            std::swap(pointer, o.pointer);
            return *this;
        }

        template<typename U>
        bool operator==(Shared<U> const &o) const noexcept {
            return pointer == o.pointer;
        }

        template<typename U>
        bool operator!=(Shared<U> const &o) const noexcept {
            return pointer != o.pointer;
        }

        template<typename U>
        Shared<U> As() const noexcept {
            if constexpr (std::is_same_v<U, T>) {
                return *this;
            } else if constexpr (std::is_base_of_v<U, T>) {
                return pointer;
            } else {
                return dynamic_cast<U *>(pointer);
            }
        }

        struct Weak<T> ToWeak() const noexcept;
    };




    /************************************************************************************/
    // std::weak_ptr like

    template<typename T>
    struct Weak {
        using ElementType = T;
        T *pointer = nullptr;

        [[maybe_unused]] [[nodiscard]] uint32_t useCount() const noexcept {
            if (!pointer) return 0;
            return PtrHeader::Get(pointer).useCount;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t refCount() const noexcept {
            if (!pointer) return 0;
            return PtrHeader::Get(pointer).refCount;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t typeId() const noexcept {
            if (!pointer) return 0;
            return PtrHeader::Get(pointer).typeId;
        }

        void Reset() {
            if (pointer) {
                auto &&h = PtrHeader::Get(pointer);
                assert(h.refCount);
                --h.refCount;
                if (h.refCount == 0 && h.useCount == 0) {
                    free(&h);
                }
                pointer = nullptr;
            }
        }

        template<typename U>
        void Reset(Shared<U> const &ptr) {
            static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>);
            if (pointer == ptr.pointer) return;
            Reset();
            if (ptr.pointer) {
                pointer = ptr.pointer;
                ++PtrHeader::Get(pointer).refCount;
            }
        }

        [[maybe_unused]] [[nodiscard]] Shared<T> Lock() const {
            if (!pointer || PtrHeader::Get(pointer).useCount == 0) return {};
            return Shared<T>(pointer);
        }

        template<typename U>
        Weak &operator=(Shared<U> const &o) {
            static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>);
            Reset(o);
            return *this;
        }

        template<typename U>
        Weak(Shared<U> const &o) {
            static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>);
            Reset(o);
        }


        ~Weak() {
            Reset();
        }

        Weak() = default;

        Weak(Weak const &o) {
            Reset(o.pointer);
        }

        Weak &operator=(Weak const &o) {
            if (&o != this) {
                Reset(o.pointer);
            }
            return *this;
        }

        Weak(Weak &&o) noexcept {
            std::swap(pointer, o.pointer);
        }

        Weak &operator=(Weak &&o) noexcept {
            std::swap(pointer, o.pointer);
            return *this;
        }

        bool operator==(Weak const &o) const noexcept {
            return pointer == o.pointer;
        }

        bool operator!=(Weak const &o) const noexcept {
            return pointer != o.pointer;
        }
    };

    template<typename T>
    Weak<T> Shared<T>::ToWeak() const noexcept {
        return Weak<T>(*this);
    }




    /************************************************************************************/
    // type traits

    template<typename T>
    struct IsPtrShared : std::false_type {
    };
    template<typename T>
    struct IsPtrShared<Shared<T>> : std::true_type {
    };
    template<typename T>
    struct IsPtrShared<Shared<T> &> : std::true_type {
    };
    template<typename T>
    struct IsPtrShared<Shared<T> const &> : std::true_type {
    };
    template<typename T>
    constexpr bool IsPtrShared_v = IsPtrShared<T>::value;


    template<typename T>
    struct IsPtrWeak : std::false_type {
    };
    template<typename T>
    struct IsPtrWeak<Weak<T>> : std::true_type {
    };
    template<typename T>
    struct IsPtrWeak<Weak<T> &> : std::true_type {
    };
    template<typename T>
    struct IsPtrWeak<Weak<T> const &> : std::true_type {
    };
    template<typename T>
    constexpr bool IsPtrWeak_v = IsPtrWeak<T>::value;





    /************************************************************************************/
    // helpers

    template<typename T, typename...Args>
    [[maybe_unused]] Shared<T> MakeShared(Args &&...args) {
        Shared<T> rtv;
        auto h = (PtrHeader *) malloc(sizeof(PtrHeader) + sizeof(T));
        if (!h) throw std::runtime_error("out of memory");
        h->data1 = 0;
        h->typeId = TypeId_v<T>;
        h->offset = 0;
        try {
            rtv.Reset(new(h + 1) T(std::forward<Args>(args)...));
        }
        catch (std::exception const &ex) {
            free(h);
            throw ex;
        }
        return rtv;
    }

}
