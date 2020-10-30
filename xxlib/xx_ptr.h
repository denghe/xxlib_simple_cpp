#pragma once

#include <new>
#include <cstdlib>
#include <cstdint>
#include <type_traits>
#include <cassert>
#include <stdexcept>

// only can create by Make / MakeShared. usually can't be use by VALUE mode. no thread safe
// careful: constructor Shared(this) only for no except mode
// destructor can't Shared(this)

// -> nullptr check throw for try catch
//#define XX_ENABLE_SHARED_NULL_THROW

namespace xx {

    struct PtrBase {
        // strong
        [[nodiscard]] int &useCount() const {
            return ((int *) this)[-2];
        };

        // weak
        [[nodiscard]] int &refCount() const {
            return ((int *) this)[-1];
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

        explicit operator T *() noexcept {
            return pointer;
        }

        explicit operator T *const &() const noexcept {
            return pointer;
        }

#ifdef XX_ENABLE_SHARED_NULL_THROW
        T *operator->() const {
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

        [[nodiscard]] int useCount() const noexcept {
            return pointer ? pointer->useCount() : 0;
        }

        [[nodiscard]] int refCount() const noexcept {
            return pointer ? pointer->refCount() : 0;
        }

        template<typename U = T>
        void Reset(std::enable_if_t<std::is_base_of_v<T, U>, U *> const &ptr = nullptr) {
            if (pointer == ptr) return;
            if (pointer) {
                assert(pointer->useCount());
                --pointer->useCount();
                if (pointer->useCount() == 0) {
                    pointer->~T();
                }
                if (pointer->refCount() == 0) {
                    free((char *) pointer - 8);
                }
                pointer = nullptr;
            }
            if (ptr) {
                pointer = ptr;
                ++ptr->useCount();
            }
        }

        ~Shared() {
            Reset();
        }

        Shared() = default;

        template<typename U = T>
        explicit Shared(std::enable_if_t<std::is_base_of_v<T, U>, U *> const &ptr) {
            Reset(ptr);
        }

        template<typename U = T>
        explicit Shared(std::enable_if_t<std::is_base_of_v<T, U>, Shared<U>> const &o) {
            Reset(o.pointer);
        }

        template<typename U = T>
        Shared &operator=(std::enable_if_t<std::is_base_of_v<T, U>, U *> const &ptr) {
            Reset(ptr);
            return *this;
        }

        template<typename U = T>
        Shared &operator=(std::enable_if_t<std::is_base_of_v<T, U>, Shared<U>> const &o) {
            Reset(o.pointer);
            return *this;
        }

        Shared(Shared &&o) noexcept {
            std::swap(pointer, o.pointer);
        }

        Shared &operator=(Shared &&o) noexcept {
            std::swap(pointer, o.pointer);
            return *this;
        }

        template<typename U = T>
        bool operator==(Shared<U> const &o) const noexcept {
            return pointer == o.pointer;
        }

        template<typename U = T>
        bool operator!=(Shared<U> const &o) const noexcept {
            return pointer != o.pointer;
        }

        template<typename U = T>
        Shared<std::enable_if_t<std::is_base_of_v<PtrBase, U>, U>> As() const noexcept {
            if constexpr (std::is_same_v<U, T>) {
                return *this;
            } else if constexpr (std::is_base_of_v<U, T>) {
                return pointer;
            } else {
                return dynamic_cast<U>(pointer);
            }
        }

        template<typename...Args>
        Shared &Make(Args &&...args) {
            auto m = (char *) malloc(8 + sizeof(T));
            assert(((size_t) m & !(size_t) 7) == 0);
            *(uint64_t *) m = 0;
            try {
                new(m + 8) T(std::forward<Args>(args)...);
                Reset((T *) (m + 8));
            }
            catch (...) {
                free(m);
                // throw ?
            }
            return *this;
        }
    };

    template<typename T, typename...Args>
    [[maybe_unused]] static Shared<T> MakeShared(Args &&...args) {
        Shared<T> rtv;
        rtv.Make(std::forward<Args>(args)...);
        return rtv;
    }


    template<typename T>
    struct Weak<T, std::enable_if_t<std::is_base_of_v<PtrBase, T>>> final {
        T *pointer = nullptr;

        [[nodiscard]] int useCount() const noexcept {
            return pointer ? pointer->useCount() : 0;
        }

        [[nodiscard]] int refCount() const noexcept {
            return pointer ? pointer->refCount() : 0;
        }

        template<typename U = T>
        void Reset(std::enable_if_t<std::is_base_of_v<T, U>, Shared<U>> const &ptr = Shared<U>(nullptr)) {
            if (pointer == ptr.pointer) return;
            if (pointer) {
                assert(pointer->refCount());
                --pointer->refCount();
                if (pointer->refCount() == 0 && pointer->useCount() == 0) {
                    free((char *) pointer - 8);
                }
                pointer = nullptr;
            }
            if (ptr.pointer) {
                pointer = ptr.pointer;
                ++ptr->refCount();
            }
        }

        [[nodiscard]] Shared<T> Lock() const {
            if (!pointer || pointer->useCount() == 0) return {};
            return Shared<T>(pointer);
        }

        template<typename U = T>
        Weak &operator=(Shared<U> const &o) {
            Reset(o);
            return *this;
        }

        template<typename U = T>
        explicit Weak(Shared<U> const &o) {
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
