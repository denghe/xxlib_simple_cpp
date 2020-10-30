#pragma once

#include <new>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <cassert>
#include <stdexcept>
#include "xx_typename_islambda.h"

// only can create by Make / MakeShared. usually can't be use by VALUE mode. no thread safe
// careful: constructor Shared(this) only for no except mode
// destructor can't Shared(this)

// -> nullptr check throw for try catch
//#define XX_ENABLE_SHARED_NULL_THROW

namespace xx {

    struct PtrHeader {
        union {
            struct {
                uint32_t useCount;    // for Shared
                uint32_t refCount;    // for Weak
            };
            uint64_t data1;
        };
        union {
            struct {
                uint32_t typeId;    // for type check & serialize / deserialize
                uint32_t offset;    // for recursive serialize
            };
            uint64_t data2;
        };
    };

    struct PtrBase {
        [[maybe_unused]] [[nodiscard]] PtrHeader &GetPtrHeader() const {
            return *((PtrHeader *) this - 1);
        };

        virtual ~PtrBase() = default;
    };

    // std::shared_ptr like
    template<typename, typename = void>
    struct Shared;

    // std::weak_ptr like
    template<typename, typename = void>
    struct Weak;

    template<typename T>
    struct Shared<T, std::enable_if_t<std::is_base_of_v<PtrBase, T>>> final {
        T *pointer = nullptr;

        operator T *() noexcept {
            return pointer;
        }

        operator T *const &() const noexcept {
            return pointer;
        }

#ifdef XX_ENABLE_SHARED_NULL_THROW
        T* operator->() const {
            if (!pointer) throw std::runtime_error("exception: pointer == nullptr");
            return pointer;
        }
#else

        T *operator->() const noexcept {
            assert(pointer);
            return pointer;
        }

#endif

        [[maybe_unused]] [[nodiscard]] bool Empty() const noexcept {
            return pointer == nullptr;
        }

        [[maybe_unused]] [[nodiscard]] bool HasValue() const noexcept {
            return pointer != nullptr;
        }

        // unsafe
        [[maybe_unused]] [[nodiscard]] PtrHeader &Header() const noexcept {
            return *((PtrHeader *) pointer - 1);
        }

        [[maybe_unused]] [[nodiscard]] uint32_t useCount() const noexcept {
            if (!pointer) return 0;
            auto &&h = Header();
            return h.useCount;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t refCount() const noexcept {
            if (!pointer) return 0;
            return Header().refCount;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t typeId() const noexcept {
            if (!pointer) return 0;
            return Header().typeId;
        }

        template<typename U>
        void Reset(U *const &ptr) {
            static_assert(std::is_base_of_v<T, U>);
            if (pointer == ptr) return;
            if (pointer) {
                auto &&h = Header();
                assert(h.useCount);
                --h.useCount;
                if (h.useCount == 0) {
                    pointer->~T();
                    if (h.refCount == 0) {
                        free(&h);
                    }
                }
                pointer = nullptr;
            }
            if (ptr) {
                pointer = ptr;
                auto &&h = Header();
                ++h.useCount;
            }
        }

        void Reset() {
            Reset<T>(nullptr);
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

        template<typename U>
        Shared(Shared<U> const &o) {
            static_assert(std::is_base_of_v<T, U>);
            Reset(o.pointer);
        }

        template<typename U>
        Shared(Shared<U> &&o) noexcept {
            static_assert(std::is_base_of_v<T, U>);
            pointer = o.pointer;
            o.pointer = nullptr;
        }

        template<typename U>
        Shared &operator=(U *const &ptr) {
            static_assert(std::is_base_of_v<T, U>);
            Reset(ptr);
            return *this;
        }

        template<typename U>
        Shared &operator=(Shared<U> const &o) {
            static_assert(std::is_base_of_v<T, U>);
            Reset(o.pointer);
            return *this;
        }

        template<typename U>
        Shared &operator=(Shared<U> &&o) {
            static_assert(std::is_base_of_v<T, U>);
            if constexpr (!std::is_same_v<U, T>) {
                Reset();
            }
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
                return dynamic_cast<U*>(pointer);
            }
        }
    };

    template<typename T, typename...Args>
    [[maybe_unused]] Shared<T> MakeShared(Args &&...args) {
        std::string typeName{TypeName_v<T>};
        Shared<T> rtv;
        auto h = (PtrHeader *) malloc(sizeof(PtrHeader) + sizeof(T));
        if (!h) throw std::runtime_error("out of memory");
        h->data1 = 0;
        h->data2 = 0;
        // todo: fill typeId from T::typeId ?
        try {
            rtv.Reset(new(h + 1) T(std::forward<Args>(args)...));
        }
        catch (std::exception const &ex) {
            free(h);
            throw ex;
        }
        return rtv;
    }

    template<typename T>
    struct Weak<T, std::enable_if_t<std::is_base_of_v<PtrBase, T>>> final {
        T *pointer = nullptr;

        // unsafe
        [[nodiscard]] PtrHeader &Header() const noexcept {
            return *((PtrHeader *) pointer - 1);
        }

        [[maybe_unused]] [[nodiscard]] uint32_t useCount() const noexcept {
            if (!pointer) return 0;
            return Header().useCount;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t refCount() const noexcept {
            if (!pointer) return 0;
            return Header().refCount;
        }

        [[maybe_unused]] [[nodiscard]] uint32_t typeId() const noexcept {
            if (!pointer) return 0;
            return Header().typeId;
        }

        template<typename U>
        void Reset(Shared<U> const &ptr) {
            static_assert(std::is_base_of_v<T, U>);
            if (pointer == ptr.pointer) return;
            if (pointer) {
                auto &&h = Header();
                assert(h.refCount);
                --h.refCount;
                if (h.refCount == 0 && h.useCount == 0) {
                    free(&h);
                }
                pointer = nullptr;
            }
            if (ptr.pointer) {
                pointer = ptr.pointer;
                ++Header().refCount;
            }
        }

        void Reset() {
            Reset<T>({});
        }

        [[maybe_unused]] [[nodiscard]] Shared<T> Lock() const {
            if (!pointer || Header().useCount == 0) return {};
            return Shared<T>(pointer);
        }

        template<typename U>
        Weak &operator=(Shared<U> const &o) {
            static_assert(std::is_base_of_v<T, U>);
            Reset(o);
            return *this;
        }

        template<typename U>
        Weak(Shared<U> const &o) {
            static_assert(std::is_base_of_v<T, U>);
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
}
