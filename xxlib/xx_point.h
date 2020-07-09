#pragma once
#include "xx_data_rw.h"

namespace xx
{
	// 用来存坐标
	struct Point {
		float x = 0, y = 0;

		inline bool operator==(const Point& v) const noexcept {
			return x == v.x && y == v.y;
		}
		inline Point& operator+=(Point const& v) noexcept {
			x += v.x;
			y += v.y;
			return *this;
		}
		inline Point operator+(Point const& v) const noexcept {
			return Point{ x + v.x, y + v.y };
		}
		inline Point operator-(Point const& v) const noexcept {
			return Point{ x - v.x, y - v.y };
		}
		inline Point operator*(float const& s) const noexcept {
			return Point{ x * s, y * s };
		}
		inline Point operator/(float const& s) const noexcept {
			return Point{ x / s, y / s };
		}

		inline float Distance(Point const& v) const {
			auto dx = v.x - x;
			auto dy = v.y - y;
			return std::sqrt(dx * dx + dy * dy);
		}
	};

	// 标识内存可移动
	template<>
	struct IsPod<Point, void> : std::true_type {};

	// 适配 Point 之 序列化 & 反序列化
	template<>
	struct DataFuncs<Point, void> {
		static inline void Write(Data& d, Point const& in) {
			d.WriteFixed(in);
		}
		static inline int Read(DataReader& d, Point& out) {
			return d.ReadFixed(out);
		}
	};

	// 判断两线段( p0-p1, p2-p3 )是否相交, 并且往 p 填充交点
	inline bool GetSegmentIntersection(Point const& p0, Point const& p1, Point const& p2, Point const& p3, Point* const& p = nullptr) noexcept {
		Point s02, s10, s32;
		float s_numer, t_numer, denom, t;
		s10.x = p1.x - p0.x;
		s10.y = p1.y - p0.y;
		s32.x = p3.x - p2.x;
		s32.y = p3.y - p2.y;

		denom = s10.x * s32.y - s32.x * s10.y;
		if (denom == 0) return false; // Collinear
		bool denomPositive = denom > 0;

		s02.x = p0.x - p2.x;
		s02.y = p0.y - p2.y;
		s_numer = s10.x * s02.y - s10.y * s02.x;
		if ((s_numer < 0) == denomPositive) return false; // No collision

		t_numer = s32.x * s02.y - s32.y * s02.x;
		if ((t_numer < 0) == denomPositive) return false; // No collision

		if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive)) return false; // No collision

		t = t_numer / denom;		// Collision detected
		if (p) {
			p->x = p0.x + (t * s10.x);
			p->y = p0.y + (t * s10.y);
		}
		return true;
	}

	// 计算直线的弧度( 转为角度还要  * (180.0f / float(M_PI) )
	inline float GetAngle(Point const& from, Point const& to) noexcept
	{
		if (from == to) return 0.0f;
		auto&& len_y = to.y - from.y;
		auto&& len_x = to.x - from.x;
		return atan2f(len_y, len_x);
	}
	inline float GetAngle(std::pair<Point, Point> const& fromTo) noexcept {
		return GetAngle(fromTo.first, fromTo.second);
	}

	// 计算距离
	inline float GetDistance(Point const& a, Point const& b) noexcept
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		return sqrtf(dx * dx + dy * dy);
	}
	inline float GetDistance(std::pair<Point, Point> const& fromTo) noexcept {
		return GetDistance(fromTo.first, fromTo.second);
	}

	// 以 0,0 为中心旋转. a 为弧度( 角度 * (float(M_PI) / 180.0f) )
	inline Point Rotate(Point const& pos, float const& a) noexcept
	{
		auto&& sinA = sinf(a);
		auto&& cosA = cosf(a);
		return Point{ pos.x * cosA - pos.y * sinA, pos.x * sinA + pos.y * cosA };
	}

	// 2 控制点的贝塞尔( p c c p c c p ... ) 切片转点
	inline void BezierToPoints(std::vector<Point>& ps, std::vector<Point> const& bs, int numSegments = 50) {
		auto len = (bs.size() - 1) / 3;
		ps.resize(numSegments * len);
		auto step = 1.0f / numSegments;
		for (size_t i = 0; i < numSegments; ++i) {
			auto t = step * i;
			auto t1 = 1 - t;
			for (size_t j = 0; j < len; ++j)
			{
				auto idx = j * 3;
				ps[j * numSegments + i] = bs[idx] * t1 * t1 * t1
					+ bs[idx + 1] * 3 * t * t1 * t1
					+ bs[idx + 2] * 3 * t * t * (1 - t)
					+ bs[idx + 3] * t * t * t;
			}
		}
	}

	// 途径点曲线( p p p ... ) 转 2 控制点的贝塞尔( p c c p c c p ... )
	inline void CurveToBezier(std::vector<Point>& bs, std::vector<Point> const& cs, float tension = 0.3f) {
		auto n = cs.size();
		auto len = n * 3 - 2;
		bs.resize(len);

		bs[0] = cs[0];
		bs[1] = (cs[1] - cs[0]) * tension + cs[0];

		for (size_t i = 0; i < n - 2; i++) {
			auto diff = (cs[i + 2] - cs[i]) * tension;
			bs[3 * i + 2] = cs[i + 1] - diff;
			bs[3 * i + 3] = cs[i + 1];
			bs[3 * i + 4] = cs[i + 1] + diff;
		}
		bs[len - 2] = (cs[n - 2] - cs[n - 1]) * tension + cs[n - 1];
		bs[len - 1] = cs[n - 1];
	}

}
