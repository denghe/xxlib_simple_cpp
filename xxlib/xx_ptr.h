﻿#pragma once

#include "xx_typehelpers.h"

// 类似 std::shared_ptr / weak_ptr，非线程安全，Weak 提供了无损 useCount 检测功能以方便直接搞事情

namespace xx {

	/************************************************************************************/
	// Make 时会在内存块头部附加

	struct PtrHeader {
		uint32_t useCount;      // 强引用技术
		uint32_t refCount;      // 弱引用技术
		union {
			struct {
				uint32_t typeId;        // 序列化 或 类型转换用
				uint32_t offset;        // 序列化等过程中使用
			};
			void* ud;
		};
	};


	/************************************************************************************/
	// std::shared_ptr like

	template<typename T>
	struct Weak;

	template<typename T>
	struct Shared {
		using ElementType = T;
		T* pointer = nullptr;

		XX_FORCEINLINE operator T* const& () const noexcept {
			return pointer;
		}
		XX_FORCEINLINE operator T*& () noexcept {
			return pointer;
		}

		XX_FORCEINLINE T* const& operator->() const noexcept {
			return pointer;
		}

		XX_FORCEINLINE T const& Value() const noexcept {
			return *pointer;
		}
		XX_FORCEINLINE T& Value() noexcept {
			return *pointer;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE explicit operator bool() const noexcept {
			return pointer != nullptr;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE bool Empty() const noexcept {
			return pointer == nullptr;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE bool HasValue() const noexcept {
			return pointer != nullptr;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE uint32_t useCount() const noexcept {
			if (!pointer) return 0;
			return header()->useCount;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE uint32_t refCount() const noexcept {
			if (!pointer) return 0;
			return header()->refCount;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE uint32_t typeId() const noexcept {
			if (!pointer) return 0;
			assert(header()->typeId);
			return header()->typeId;
		}

		// unsafe
		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE PtrHeader* header() const noexcept {
			return ((PtrHeader*)pointer - 1);
		}

		// 将 header 内容拼接为 string 返回 以方便显示 & 调试
		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE std::string GetHeaderInfo() const noexcept {
			std::string s;
			if (!pointer) {
				s += "nil";
			}
			else {
				auto h = header();
				s += "{\"useCount\":" + std::to_string(h->useCount)
					+ ",\"refCount\":" + std::to_string(h->refCount)
					+ ",\"typeId\":" + std::to_string(h->typeId)
					+ ",\"offset\":" + std::to_string(h->offset)
					+ "}";
			}
			return s;
		}

		void Reset() {
			if (pointer) {
				auto h = header();
				assert(h->useCount);
				// 不能在这里 -1, 这将导致成员 weak 指向自己时触发 free
				if (h->useCount == 1) {
					pointer->~T();
					pointer = nullptr;
					if (h->refCount == 0) {
						free(h);
					} else {
						h->useCount = 0;
					}
				}
				else {
					--h->useCount;
					pointer = nullptr;
				}
			}
		}

		template<typename U>
		void Reset(U* const& ptr) {
			static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>);
			if (pointer == ptr) return;
			Reset();
			if (ptr) {
				pointer = ptr;
				++((PtrHeader*)ptr - 1)->useCount;
			}
		}

		XX_FORCEINLINE ~Shared() {
			Reset();
		}

		Shared() = default;

		template<typename U>
		XX_FORCEINLINE Shared(U* const& ptr) {
			static_assert(std::is_base_of_v<T, U>);
			Reset(ptr);
		}
		XX_FORCEINLINE Shared(T* const& ptr) {
			Reset(ptr);
		}

		template<typename U>
		XX_FORCEINLINE Shared(Shared<U> const& o) {
			static_assert(std::is_base_of_v<T, U>);
			Reset(o.pointer);
		}
		XX_FORCEINLINE Shared(Shared const& o) {
			Reset(o.pointer);
		}

		template<typename U>
		XX_FORCEINLINE Shared(Shared<U>&& o) noexcept {
			static_assert(std::is_base_of_v<T, U>);
			pointer = o.pointer;
			o.pointer = nullptr;
		}
		XX_FORCEINLINE Shared(Shared&& o) noexcept {
			pointer = o.pointer;
			o.pointer = nullptr;
		}

		template<typename U>
		XX_FORCEINLINE Shared& operator=(U* const& ptr) {
			static_assert(std::is_base_of_v<T, U>);
			Reset(ptr);
			return *this;
		}
		XX_FORCEINLINE Shared& operator=(T* const& ptr) {
			Reset(ptr);
			return *this;
		}

		template<typename U>
		XX_FORCEINLINE Shared& operator=(Shared<U> const& o) {
			static_assert(std::is_base_of_v<T, U>);
			Reset(o.pointer);
			return *this;
		}
		XX_FORCEINLINE Shared& operator=(Shared const& o) {
			Reset(o.pointer);
			return *this;
		}

		template<typename U>
		XX_FORCEINLINE Shared& operator=(Shared<U>&& o) {
			static_assert(std::is_base_of_v<T, U>);
			Reset();
			std::swap(pointer, (*(Shared*)&o).pointer);
			return *this;
		}
		XX_FORCEINLINE Shared& operator=(Shared&& o) {
			std::swap(pointer, o.pointer);
			return *this;
		}

		template<typename U>
		XX_FORCEINLINE bool operator==(Shared<U> const& o) const noexcept {
			return pointer == o.pointer;
		}

		template<typename U>
		XX_FORCEINLINE bool operator!=(Shared<U> const& o) const noexcept {
			return pointer != o.pointer;
		}

		// 有条件的话尽量使用 ObjManager 的 As, 避免发生 dynamic_cast
		template<typename U>
		XX_FORCEINLINE Shared<U> As() const noexcept {
			if constexpr (std::is_same_v<U, T>) {
				return *this;
			}
			else if constexpr (std::is_base_of_v<U, T>) {
				return pointer;
			}
			else {
				return dynamic_cast<U*>(pointer);
			}
		}

		// unsafe: 直接硬转返回. 使用前通常会根据 typeId 进行合法性检测
		template<typename U>
		XX_FORCEINLINE Shared<U>& ReinterpretCast() const noexcept {
			return *(Shared<U>*)this;
		}

		struct Weak<T> ToWeak() const noexcept;

		// 填充式 make
		template<typename...Args>
		Shared& Emplace(Args&&...args);
	};




	/************************************************************************************/
	// std::weak_ptr like

	template<typename T>
	struct Weak {
		using ElementType = T;
		PtrHeader* h = nullptr;

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE uint32_t useCount() const noexcept {
			if (!h) return 0;
			return h->useCount;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE uint32_t refCount() const noexcept {
			if (!h) return 0;
			return h->refCount;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE uint32_t typeId() const noexcept {
			if (!h) return 0;
			return h->typeId;
		}

		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE explicit operator bool() const noexcept {
			return h && h->useCount;
		}

		// unsafe: 直接计算出指针
		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE T* pointer() const {
			return (T*)(h + 1);
		}

		XX_FORCEINLINE void Reset() {
			if (h) {
				if (h->refCount == 1 && h->useCount == 0) {
					free(h);
				}
				else {
					--h->refCount;
				}
				h = nullptr;
			}
		}

		template<typename U>
		XX_FORCEINLINE void Reset(Shared<U> const& s) {
			static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>);
			Reset();
			if (s.pointer) {
				h = ((PtrHeader*)s.pointer - 1);
				++h->refCount;
			}
		}
		
		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE Shared<T> Lock() const {
			if (h && h->useCount) {
                auto p = h + 1;
                return *(Shared<T> *) &p;
            }
			return {};
		}

		// unsafe 系列: 要安全使用，每次都 if 真 再调用这些函数 1 次。一次 if 多次调用的情景除非很有把握在期间 Shared 不会析构，否则还是 Lock()
		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE ElementType* operator->() const noexcept {
			return (ElementType*)(h + 1);
		}
		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE ElementType const& Value() const noexcept {
			return *(ElementType*)(h + 1);
		}
		[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE ElementType& Value() noexcept {
			return *(ElementType*)(h + 1);
		}
		XX_FORCEINLINE operator ElementType* () const noexcept {
			return (ElementType*)(h + 1);
		}

		template<typename U>
		XX_FORCEINLINE Weak& operator=(Shared<U> const& o) {
			static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>);
			Reset(o);
			return *this;
		}

		template<typename U>
		XX_FORCEINLINE Weak(Shared<U> const& o) {
			static_assert(std::is_same_v<T, U> || std::is_base_of_v<T, U>);
			Reset(o);
		}

		XX_FORCEINLINE ~Weak() {
			Reset();
		}

		Weak() = default;

		XX_FORCEINLINE Weak(Weak const& o) {
			if ((h = o.h)) {
				++o.h->refCount;
			}
		}
		template<typename U>
		XX_FORCEINLINE Weak(Weak<U> const& o) {
			static_assert(std::is_base_of_v<T, U>);
			if ((h = o.h)) {
				++o.h->refCount;
			}
		}

		XX_FORCEINLINE Weak(Weak&& o) noexcept {
			h = o.h;
			o.h = nullptr;
		}
		template<typename U>
		XX_FORCEINLINE Weak(Weak<U>&& o) noexcept {
			static_assert(std::is_base_of_v<T, U>);
			h = o.h;
			o.h = nullptr;
		}

		XX_FORCEINLINE Weak& operator=(Weak const& o) {
			if (&o != this) {
				Reset(o.Lock());
			}
			return *this;
		}
		template<typename U>
		XX_FORCEINLINE Weak& operator=(Weak<U> const& o) {
			static_assert(std::is_base_of_v<T, U>);
			if ((void*)&o != (void*)this) {
				Reset(((Weak*)(&o))->Lock());
			}
			return *this;
		}

		XX_FORCEINLINE Weak& operator=(Weak&& o) noexcept {
			std::swap(h, o.h);
			return *this;
		}
		// operator=(Weak&& o) 没有模板实现，因为不确定交换 h 之后的类型是否匹配

		template<typename U>
		XX_FORCEINLINE bool operator==(Weak<U> const& o) const noexcept {
			return h == o.h;
		}
		template<typename U>
		XX_FORCEINLINE bool operator!=(Weak<U> const& o) const noexcept {
			return h != o.h;
		}
	};

	template<typename T>
	XX_FORCEINLINE Weak<T> Shared<T>::ToWeak() const noexcept {
		if (pointer) {
			auto h = (PtrHeader*)pointer - 1;
			return *(Weak<T>*)&h;
		}
		return {};
	}

	template<typename T>
	template<typename...Args>
	XX_FORCEINLINE Shared<T>& Shared<T>::Emplace(Args&&...args) {
		Reset();
		auto h = (PtrHeader*)malloc(sizeof(PtrHeader) + sizeof(T));
		h->useCount = 1;
		h->refCount = 0;
		h->typeId = TypeId_v<T>;
		h->offset = 0;
		pointer = new(h + 1) T(std::forward<Args>(args)...);
		return *this;
	}


	/************************************************************************************/
	// type traits

	template<typename T>
	struct IsXxShared : std::false_type {
	};
	template<typename T>
	struct IsXxShared<Shared<T>> : std::true_type {
	};
	template<typename T>
	struct IsXxShared<Shared<T>&> : std::true_type {
	};
	template<typename T>
	struct IsXxShared<Shared<T> const&> : std::true_type {
	};
	template<typename T>
	constexpr bool IsXxShared_v = IsXxShared<T>::value;


	template<typename T>
	struct IsXxWeak : std::false_type {
	};
	template<typename T>
	struct IsXxWeak<Weak<T>> : std::true_type {
	};
	template<typename T>
	struct IsXxWeak<Weak<T>&> : std::true_type {
	};
	template<typename T>
	struct IsXxWeak<Weak<T> const&> : std::true_type {
	};
	template<typename T>
	constexpr bool IsXxWeak_v = IsXxWeak<T>::value;


	template<typename T>
	struct IsPointerClass<T, std::enable_if_t<IsXxShared_v<T> || IsXxWeak_v<T>>>
		: std::true_type {
	};

	template<typename T>
	struct ToPointerFuncs<T, std::enable_if_t<IsXxWeak_v<T>>> {
		static inline auto Convert(T&& v) {
			return v.Lock();
		}
	};

	/************************************************************************************/
	// helpers

	template<typename T, typename...Args>
	[[maybe_unused]] [[nodiscard]] XX_FORCEINLINE Shared<T> MakeShared(Args &&...args) {
		auto h = (PtrHeader*)malloc(sizeof(PtrHeader) + sizeof(T));
		h->useCount = 0;
		h->refCount = 0;
		h->typeId = TypeId_v<T>;
		h->offset = 0;
		return new(h + 1) T(std::forward<Args>(args)...);
	}


}

// 令 Shared Weak 支持放入 hash 容器
namespace std {
	template <typename T>
	struct hash<xx::Shared<T>> {
		size_t operator()(xx::Shared<T> const& v) const {
			return (size_t)v.pointer;
		}
	};

	template <typename T>
	struct hash<xx::Weak<T>> {
		size_t operator()(xx::Weak<T> const& v) const {
			return (size_t)v.h;
		}
	};
}
