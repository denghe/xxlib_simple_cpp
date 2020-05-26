#pragma once
#include <type_traits>

//#ifdef _MSC_VER
//#pragma execution_character_set("utf-8")
//#endif

/************************************************************************************/
// 规范大小尾宏判定
#ifndef _WIN32
#include <arpa/inet.h>  /* __BYTE_ORDER */
#endif
#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)
#    if __BYTE_ORDER == __LITTLE_ENDIAN
#        define __LITTLE_ENDIAN__
#    elif __BYTE_ORDER == __BIG_ENDIAN
#        define __BIG_ENDIAN__
#    elif _WIN32
#        define __LITTLE_ENDIAN__
#    endif
#endif

/************************************************************************************/
// 交换删除. tar 为 container 的某成员, 其 indexMember 字段存储了自己位于 container 的 下标
#define XX_SWAP_REMOVE( tar, indexMember, container ) { \
	auto i = tar->indexMember;							\
	auto lastIndex = (int)container.size() - 1;			\
	if ((int)i < lastIndex) {							\
		std::swap(container[i], container[lastIndex]);	\
		container[lastIndex]->indexMember = i;			\
	}													\
	container.resize(lastIndex);						\
}

/************************************************************************************/
// 提供兼容 win32 下的 Sleep 写法
// 直接用 这个似乎可以避免 windows 下用 this_thread::sleep_for 搞出额外线程
// 可能需要 include windows.h
#ifndef _WIN32
#include <unistd.h>
inline void Sleep(int ms) {
	usleep(ms * 1000);
}
#endif

/************************************************************************************/
// 令 std::min std::max 在某些情况下正常
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

/************************************************************************************/
// 部分平台缺 _countof  _offsetof  container_of
#ifndef _countof
template<typename T, size_t N>
size_t _countof_helper(T const (&arr)[N]) {
	return N;
}
#define _countof(_Array) _countof_helper(_Array)
#endif

#ifndef _offsetof
#define _offsetof(s,m) ((size_t)&reinterpret_cast<char const volatile&>((((s*)0)->m)))
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - _offsetof(type, member)))
#endif

/************************************************************************************/
// stackless 协程相关

// 当前主要用到这些宏。只有 lineNumber 一个特殊变量名要求
#define COR_BEGIN	switch (lineNumber) { case 0:
#define COR_YIELD	return __LINE__; case __LINE__:;
#define COR_EXIT	return 0;
#define COR_END		} return 0;
/*
	int lineNumber = 0;
	int Update() {
		COR_BEGIN
			// COR_YIELD
		COR_END
	}
	... lineNumber = Update();
*/

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
