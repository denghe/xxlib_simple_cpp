﻿#pragma once
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

    template<typename T>
    struct IsWeak : std::false_type {};

    template<typename T>
    struct IsWeak<std::weak_ptr<T>> : std::true_type {};

    template<typename T>
    constexpr bool IsWeak_v = IsWeak<T>::value;

    template<typename T>
    struct IsUnique : std::false_type {};

    template<typename T>
    struct IsUnique<std::unique_ptr<T>> : std::true_type {};

    template<typename T>
    constexpr bool IsUnique_v = IsUnique<T>::value;


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


    /************************************************************************************/
    // shared_ptr 系列

    template<typename T, typename ...Args>
    std::shared_ptr<T> Make(Args &&...args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename ...Args>
    std::shared_ptr<T> TryMake(Args &&...args) noexcept {
        try {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }
        catch (...) {
            return std::shared_ptr<T>();
        }
    }

    template<typename T, typename ...Args>
    std::shared_ptr<T> &MakeTo(std::shared_ptr<T> &v, Args &&...args) {
        v = std::make_shared<T>(std::forward<Args>(args)...);
        return v;
    }

    template<typename T, typename ...Args>
    std::shared_ptr<T> &TryMakeTo(std::shared_ptr<T> &v, Args &&...args) noexcept {
        v = TryMake<T>(std::forward<Args>(args)...);
        return v;
    }

    template<typename T, typename U>
    std::shared_ptr<T> As(std::shared_ptr<U> const &v) noexcept {
        return std::dynamic_pointer_cast<T>(v);
    }

    /************************************************************************************/
    // weak_ptr 系列

    template<typename T, typename U>
    std::weak_ptr<T> AsWeak(std::shared_ptr<U> const &v) noexcept {
        return std::weak_ptr<T>(As<T>(v));
    }

    template<typename T>
    std::weak_ptr<T> ToWeak(std::shared_ptr<T> const &v) noexcept {
        return std::weak_ptr<T>(v);
    }



    /************************************************************************************/
    // unique_ptr 系列

    template<typename T, typename ...Args>
    std::unique_ptr<T> MakeU(Args &&...args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename ...Args>
    std::unique_ptr<T> TryMakeU(Args &&...args) noexcept {
        try {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }
        catch (...) {
            return std::unique_ptr<T>();
        }
    }
}
