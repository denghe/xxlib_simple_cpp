#pragma once
#include <time.h>
#include <chrono>

namespace xx {
	/************************************************************************************/
	// time_point <--> .net DateTime.Now.ToUniversalTime().Ticks converts

	// ����ʱ�侫��: ��� 7 �� 0( ���� windows ����߾���. android/ios ���1��0�ľ��� )
	typedef std::chrono::duration<long long, std::ratio<1LL, 10000000LL>> duration_10m;

	// ʱ��� ת epoch (����Ϊ��� 7 �� 0)
	inline int64_t TimePointToEpoch10m(std::chrono::system_clock::time_point const& val) noexcept {
		return std::chrono::duration_cast<duration_10m>(val.time_since_epoch()).count();
	}

	//  epoch (����Ϊ��� 7 �� 0) ת ʱ���
	inline std::chrono::system_clock::time_point Epoch10mToTimePoint(int64_t const& val) noexcept {
		return std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::system_clock::duration>(duration_10m(val)));
	}


	// �õ���ǰʱ���
	inline std::chrono::system_clock::time_point NowTimePoint() noexcept {
		return std::chrono::system_clock::now();
	}

	// �õ���ǰʱ���� epoch (����Ϊ��� 7 �� 0)
	inline int64_t NowEpoch10m() noexcept {
		return TimePointToEpoch10m(NowTimePoint());
	}


	// epoch (����Ϊ��� 7 �� 0) תΪ .Net DateTime Utc Ticks
	inline int64_t Epoch10mToUtcDateTimeTicks(int64_t const& val) noexcept {
		return val + 621355968000000000LL;
	}

	// .Net DateTime Utc Ticks תΪ epoch (����Ϊ��� 7 �� 0)
	inline int64_t UtcDateTimeTicksToEpoch10m(int64_t const& val) noexcept {
		return val - 621355968000000000LL;
	}


	// ʱ��� ת epoch (����Ϊ��)
	inline int32_t TimePointToEpoch(std::chrono::system_clock::time_point const& val) noexcept {
		return (int32_t)(val.time_since_epoch().count() / 10000000);
	}

	//  epoch (����Ϊ��) ת ʱ���
	inline std::chrono::system_clock::time_point EpochToTimePoint(int32_t const& val) noexcept {
		return std::chrono::system_clock::time_point(std::chrono::system_clock::time_point::duration((int64_t)val * 10000000));
	}


	// �õ���ǰ system ʱ���� epoch (����Ϊ ms)
	inline int64_t NowSystemEpochMS() noexcept {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}
	// �õ���ǰ steady ʱ���� epoch (����Ϊ ms)
	inline int64_t NowSteadyEpochMS() noexcept {
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

//	// ��ǰʱ��תΪ�ַ��������
//	inline void NowToString(std::string& s) noexcept {
//		auto&& t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//		std::tm tm;
//#ifdef _WIN32
//		localtime_s(&tm, &t);
//#else
//		localtime_r(&t, &tm);
//#endif
//		std::stringstream ss;
//		ss << std::put_time(&tm, "%Y-%m-%d %X");
//		s += ss.str();
//	}
//
//	// ��ǰʱ��תΪ�ַ���������
//	inline std::string NowToString() noexcept {
//		std::string s;
//		NowToString(s);
//		return s;
//	}
}
