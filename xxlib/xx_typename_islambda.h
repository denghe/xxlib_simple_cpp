﻿#pragma once

#include <type_traits>
#include <string_view>

namespace xx {
    /************************************************************************************/
    // TypeName_v

    namespace Detail {
        template<typename... T>
        constexpr auto NameOf() noexcept {
#if defined(__clang__)
            /* 输出前后缀：
auto xx::Detail::NameOf() [T = <
>]
*/
            return std::string_view{__PRETTY_FUNCTION__ + 32, sizeof(__PRETTY_FUNCTION__) - 35};
#elif defined(__GNUC__)
            /* 输出前后缀：
constexpr auto xx::Detail::NameOf() [with T = {
}]
*/
            return std::string_view{__PRETTY_FUNCTION__ + 47, sizeof(__PRETTY_FUNCTION__) - 50};
#elif defined(_MSC_VER) && _MSC_VER >= 1920 && _MSC_VER < 1930  // vs2019 16.0
            /* 输出前后缀：
auto __cdecl xx::Detail::NameOf<
>(void) noexcept
*/
            return std::string_view{ __FUNCSIG__ + 32, sizeof(__FUNCSIG__) - 49 };
#else
            static_assert(false, "unsupported compiler");
#endif
        }
    }
    template<typename... T>
    inline constexpr auto TypeName_v = Detail::NameOf<T...>();

    /************************************************************************************/
    // IsLambda_v

    namespace Detail {
        template<typename T>
        constexpr bool IsLambda() {
            constexpr std::string_view tn = TypeName_v<T>;
#if defined(__clang__)
            // lambda 输出长相: (lambda at /full_path/fileName.ext:line:colum)
            return tn.rfind("(lambda at ", 0) == 0 && *tn.rbegin() == ')' && tn.find(':') > tn.rfind('(');
#elif defined(__GNUC__)
            // lambda 输出长相: ... <lambda( ... )>
            if constexpr (tn.size() < 10 || tn[tn.size() - 1] != '>' || tn[tn.size() - 2] != ')') return false;
            auto at = tn.size() - 3;
            for (std::size_t i = 1; i; --at) {
                i += tn[at = tn.find_last_of("()", at)] == '(' ? -1 : 1;
            }
            return at >= 6 && tn.substr(at - 6, 7) == "<lambda";
#elif defined(_MSC_VER) && _MSC_VER >= 1920 && _MSC_VER < 1930  // vs2019 16.0
            // lambda 输出长相: class <lambda_1499ff3675c53ea7d4fea6e5391ecc4d>
            return tn.rfind("class <lambda_", 0) == 0 && *tn.rbegin() == '>';
#endif
        }
    }

    template<typename T>
    struct IsLambda : std::integral_constant<bool, Detail::IsLambda<T>()> {
    };

    template<typename T>
    inline constexpr bool IsLambda_v = IsLambda<T>::value;


    /************************************************************************************/
    // LambdaRtv_t   LambdaArgs_t  LambdaClass_t  (lambda / function 类型拆解)

    template<typename T, typename = void>
    struct LambdaTraits;

    template<typename Rtv, typename...Args>
    struct LambdaTraits<Rtv (*)(Args ...)> {
        using R = Rtv;
        using A = std::tuple<std::decay_t<Args>...>;
        using C = void;
    };

    // 适配 写在 std::funcion 里面那种表达式
    template<typename Rtv, typename...Args>
    struct LambdaTraits<Rtv(Args ...)> {
        using R = Rtv;
        using A = std::tuple<std::decay_t<Args>...>;
        using C = void;
    };

    template<typename Rtv, typename CT, typename... Args>
    struct LambdaTraits<Rtv (CT::*)(Args ...) const> {
        using R = Rtv;
        using A = std::tuple<std::decay_t<Args>...>;
        using C = CT;
    };

    template<typename T>
    struct LambdaTraits<T, std::void_t<decltype(&T::operator())> >
            : public LambdaTraits<decltype(&T::operator())> {
    };

    template<typename T>
    using LambdaRtv_t = typename LambdaTraits<T>::R;
    template<typename T>
    using LambdaArgs_t = typename LambdaTraits<T>::A;
    template<typename T>
    using LambdaClass_t = typename LambdaTraits<T>::C;


    /************************************************************************************/
    // MemberPointerRtv_t   MemberPointerClass_t  (成员变量指针 类型拆解)

    template<typename T>
    struct MemberPointerTypeTraits;
    template<typename Rtv, typename CT>
    struct MemberPointerTypeTraits<Rtv(CT::*)> {
        using R = Rtv;
        using C = CT;
    };

    template<typename T>
    using MemberPointerRtv_t = typename MemberPointerTypeTraits<T>::R;
    template<typename T>
    using MemberPointerClass_t = typename MemberPointerTypeTraits<T>::C;
}
