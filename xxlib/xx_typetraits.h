#pragma once
#include <type_traits>
#include <cstddef>
#include <optional>
#include <vector>
#include <string>
#include <memory>

namespace xx {
    struct Data;

	/************************************************************************************/
	// std::is_pod 的自定义扩展, 用于标识一个类可以在容器中被r memcpy | memmove

	template<typename T, typename ENABLED = void>
	struct IsPod : std::false_type {};

	template<typename T>
	constexpr bool IsPod_v = IsPod<T>::value;

	template<typename T>
	struct IsPod<T, std::enable_if_t<std::is_pod_v<T>>> : std::true_type {};


    /************************************************************************************/
    // 模板类型识别系列

    template<typename T>
    struct IsOptional : std::false_type {};

    template<typename T>
    struct IsOptional<std::optional<T>> : std::true_type {};

    template<typename T>
    constexpr bool IsOptional_v = IsOptional<T>::value;

    template<typename T>
    struct IsVector : std::false_type {};

    template<typename T>
    struct IsVector<std::vector<T>> : std::true_type {};

    template<typename T>
    constexpr bool IsVector_v = IsVector<T>::value;

    template<typename T>
    struct IsShared : std::false_type {};

    template<typename T>
    struct IsShared<std::shared_ptr<T>> : std::true_type {};

    template<typename T>
    constexpr bool IsShared_v = IsShared<T>::value;


    /************************************************************************************/
    // 容器子类型检测相关

    template<typename T>
    struct ChildType {
        using type = void;
    };
    template<typename T>
    using ChildType_t = typename ChildType<T>::type;


    template<typename T>
    struct ChildType<std::optional<T>> {
        using type = T;
    };
    template<typename T>
    struct ChildType<std::vector<T>> {
        using type = T;
    };

    /************************************************************************************/
    // 容器深度探测 系列: 计算 Container<Container<... 嵌套深度. 返回 0 表明不是 container


    template<typename T>
    constexpr size_t DeepLevel(T* const& v) {
        if constexpr (IsOptional_v<T>) return 0 + DeepLevel((ChildType_t<T>*)0);
        if constexpr (IsVector_v<T>) return 1 + DeepLevel((ChildType_t<T>*)0);
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, Data>) return 1;
        return 0;
    }

    template<typename T>
    constexpr size_t DeepLevel_v = DeepLevel((std::decay_t<T>*)0);
}
