#pragma once
#include <memory>

namespace xx {
	/************************************************************************************/
	// shared_ptr 系列

	template<typename T, typename ...Args>
	std::shared_ptr<T> Make(Args&& ...args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	std::shared_ptr<T> TryMake(Args&& ...args) noexcept {
		try {
			return std::make_shared<T>(std::forward<Args>(args)...);
		}
		catch (...) {
			return std::shared_ptr<T>();
		}
	}

	template<typename T, typename ...Args>
	std::shared_ptr<T>& MakeTo(std::shared_ptr<T>& v, Args&& ...args) {
		v = std::make_shared<T>(std::forward<Args>(args)...);
		return v;
	}

	template<typename T, typename ...Args>
	std::shared_ptr<T>& TryMakeTo(std::shared_ptr<T>& v, Args&& ...args) noexcept {
		v = TryMake<T>(std::forward<Args>(args)...);
		return v;
	}

	template<typename T, typename U>
	std::shared_ptr<T> As(std::shared_ptr<U> const& v) noexcept {
		return std::dynamic_pointer_cast<T>(v);
	}


	template<typename T>
	struct IsShared : std::false_type {};

	template<typename T>
	struct IsShared<std::shared_ptr<T>> : std::true_type {};

	template<typename T>
	constexpr bool IsShared_v = IsShared<T>::value;


	/************************************************************************************/
	// weak_ptr 系列

	template<typename T, typename U>
	std::weak_ptr<T> AsWeak(std::shared_ptr<U> const& v) noexcept {
		return std::weak_ptr<T>(As<T>(v));
	}

	template<typename T>
	std::weak_ptr<T> ToWeak(std::shared_ptr<T> const& v) noexcept {
		return std::weak_ptr<T>(v);
	}


	template<typename T>
	struct IsWeak : std::false_type {};

	template<typename T>
	struct IsWeak<std::weak_ptr<T>> : std::true_type {};

	template<typename T>
	constexpr bool IsWeak_v = IsWeak<T>::value;


	/************************************************************************************/
	// unique_ptr 系列

	template<typename T, typename ...Args>
	std::unique_ptr<T> MakeU(Args&& ...args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	std::unique_ptr<T> TryMakeU(Args&& ...args) noexcept {
		try {
			return std::make_unique<T>(std::forward<Args>(args)...);
		}
		catch (...) {
			return std::unique_ptr<T>();
		}
	}


	template<typename T>
	struct IsUnique : std::false_type {};

	template<typename T>
	struct IsUnique<std::unique_ptr<T>> : std::true_type {};

	template<typename T>
	constexpr bool IsUnique_v = IsUnique<T>::value;

}
