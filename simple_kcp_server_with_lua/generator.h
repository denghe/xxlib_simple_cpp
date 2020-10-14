#pragma once

// c++20 泛型之 发生器. 配合 for 语法 + co_yield 实现协程逻辑基本够用了

#include <experimental/coroutine>
namespace xx {
    using namespace std::experimental;

    template<typename T>
    struct Generator {
        // 这个类型名字不能改
        struct promise_type;

        using Handle = coroutine_handle<promise_type>;
    private:
        Handle h;
    public:
        explicit Generator(Handle&& h) : h(std::move(h)) {}

        Generator(const Generator &) = delete;
        Generator &operator=(const Generator &) = delete;

        Generator(Generator &&oth) noexcept: h(oth.h) {
            oth.h = nullptr;
        }
        Generator &operator=(Generator &&other) noexcept {
            std::swap(h, other.h);
            return *this;
        }

        ~Generator() {
            if (h) {
                h.destroy();
            }
        }

        bool Next() {
            h.resume();
            return not h.done();
        }

        T Value() {
            return h.promise().current_value;
        }


        template<typename F>
        void Foreach(F&& func) {
            while(true) {
                h.resume();
                if (h.done()) return;
                func(h.promise().current_value);
            }
        }

        void AWait() {
            while(true) {
                h.resume();
                if (h.done()) return;
            }
        }

        struct promise_type {
        private:
            T current_value{};

            friend class Generator;

        public:
            promise_type() = default;

            ~promise_type() = default;

            promise_type(const promise_type &) = delete;

            promise_type(promise_type &&) = delete;

            promise_type &operator=(const promise_type &) = delete;

            promise_type &operator=(promise_type &&) = delete;

            auto initial_suspend() {
                return suspend_always{};
            }

            auto final_suspend() {
                return suspend_always{};
            }

            auto get_return_object() {
                return Generator{Handle::from_promise(*this)};
            }

            auto return_void() {
                return suspend_never{};
            }

            auto yield_value(T some_value) {
                current_value = some_value;
                return suspend_always{};
            }

            void unhandled_exception() {
                std::exit(1);
            }
        };


        struct iterator_end_sentinel {
        };

        struct iterator {
            template<class>
            friend
            class Generator;

            using iterator_category = std::input_iterator_tag;
            using value_type = T;

            T operator*() {
                return _promise->current_value;
            }

            void operator++() {
                Handle::from_promise(*_promise).resume();
            }

            bool operator!=(iterator_end_sentinel) {
                return !Handle::from_promise(*_promise).done();
            }

        private:
            explicit iterator(promise_type *promise) : _promise(promise) {}

            promise_type *_promise;
        };

        iterator begin() { return iterator(&h.promise()); }

        iterator_end_sentinel end() { return {}; }
    };
}
