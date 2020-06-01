#pragma once
#include <type_traits>

namespace xx {
	/************************************************************************************/
	// std::is_pod 的自定义扩展, 用于标识一个类可以在容器中被r memcpy | memmove

	template<typename T, typename ENABLED = void>
	struct IsPod : std::false_type {};

	template<typename T>
	constexpr bool IsPod_v = IsPod<T>::value;

	template<typename T>
	struct IsPod<T, std::enable_if_t<std::is_pod_v<T>>> : std::true_type {};
}
