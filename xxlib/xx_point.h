#pragma once

#include "xx_data_rw.h"
#include "xx_string.h"
#include <cmath>        // PI.....

namespace xx {

    // 坐标
    struct Point {
        float x = 0, y = 0;

        Point() = default;

        Point(float const &x, float const &y) : x(x), y(y) {}

        Point(Point const &) = default;

        Point &operator=(Point const &) = default;

        inline bool operator==(const Point &v) const noexcept {
            return x == v.x && y == v.y;
        }

        inline Point &operator+=(Point const &v) noexcept {
            x += v.x;
            y += v.y;
            return *this;
        }

        inline Point operator+(Point const &v) const noexcept {
            return Point{x + v.x, y + v.y};
        }

        inline Point operator-(Point const &v) const noexcept {
            return Point{x - v.x, y - v.y};
        }

        inline Point operator*(float const &s) const noexcept {
            return Point{x * s, y * s};
        }

        inline Point operator/(float const &s) const noexcept {
            return Point{x / s, y / s};
        }
    };


    // 移动路线 -- 点
    struct PathwayPoint {
        // 2d坐标
        xx::Point pos;
        // 角度, 距离
        float a = 0, d = 0;

        PathwayPoint() = default;

        PathwayPoint(PathwayPoint const &) = default;

        PathwayPoint &operator=(PathwayPoint const &) = default;

        PathwayPoint(float const &x, float const &y, float const &a, float const &d) : pos(x, y), a(a), d(d) {}

        template<typename T>
        explicit PathwayPoint(T const &p) : pos(p.x, p.y) {}
    };


    // 移动路线
    struct Pathway {
        // 是否闭合( 是 则 最后一个点 的下一个指向 第一个点 )
        bool isLoop = false;

        // 点集合
        std::vector<PathwayPoint> points;

        Pathway() = default;

        // 禁用复制，只能在智能指针环境下使用
        Pathway(Pathway const &) = delete;

        Pathway &operator=(Pathway const &) = delete;

        // 前进: 传入 一共要移动的距离长度( 正数 )，当前点下标，当前点已移动距离，回填坐标 & 角度
        // 返回是否已移动到终点( isLoop == false )
        bool MoveForward(float total, size_t &i, float &d, xx::Point &pos, float &a) const;

        // 前进: 传入 一共要移动的距离长度( 正数 )，当前点下标，当前点剩余距离，回填坐标 & 角度
        // 后退: 返回是否已移动到起点( isLoop == false )
        bool MoveBack(float total, size_t &i, float &d, xx::Point &pos, float &a) const;

        // 填充 角度 和 距离
        void FillDA();
    };


    // 为特殊阵型服务的直线 / 线段构造器. 临时使用
    struct PathwayMaker {
        // 指向被 make 的 移动路线
        std::shared_ptr<Pathway> pathway;

        // make pathway 并 push 起始坐标 作为第一个点
        explicit PathwayMaker(xx::Point const &pos);

        // 从最后个点前进一段距离，形成新的点，新点.a = 最后个点.a，新点.d = 0
        PathwayMaker &Forward(float const &d);

        // 改变最后个点角度( = )
        PathwayMaker &RotateTo(float const &a);

        // 改变最后个点角度( + )
        PathwayMaker &RotateBy(float const &a);

        // 令最后个点针对 tarPos 计算 a, d, 追加形成新的点，新点.a = 最后个点.a，新点.d = 0
        PathwayMaker &To(xx::Point const &tarPos);

        // 令最后个点针对第一个点计算 a, d，标记循环 并返回 pathway 容器
        std::shared_ptr<Pathway> Loop();

        // 返回 pathway 容器
        [[nodiscard]] std::shared_ptr<Pathway> End() const;

        PathwayMaker() = delete;

        PathwayMaker(PathwayMaker const &) = delete;

        PathwayMaker &operator=(PathwayMaker const &) = delete;
    };


    // 标识内存可移动
    template<>
    struct IsPod<Point, void> : std::true_type {
    };

    // 适配 Point 之 序列化 & 反序列化
    template<>
    struct DataFuncs<Point, void> {
        static inline void Write(DataWriter &dw, Point const &in) {
            dw.WriteFixed(in);
        }

        static inline int Read(DataReader &dr, Point &out) {
            return dr.ReadFixed(out);
        }
    };

    // 适配 Point 之 ToString
    template<>
    struct StringFuncs<Point, void> {
        static inline void Append(std::string &s, Point const &in) {
            xx::Append(s, '[', in.x, ',', in.y, ']');
        }
    };


    // 标识内存可移动
    template<>
    struct IsPod<PathwayPoint, void> : std::true_type {
    };

    // 适配 PathwayPoint 之 序列化 & 反序列化
    template<>
    struct DataFuncs<PathwayPoint, void> {
        static inline void Write(DataWriter &dw, PathwayPoint const &in) {
            dw.WriteFixed(in);
        }

        static inline int Read(DataReader &dr, PathwayPoint &out) {
            return dr.ReadFixed(out);
        }
    };

    // 适配 PathwayPoint 之 ToString
    template<>
    struct StringFuncs<PathwayPoint, void> {
        static inline void Append(std::string &s, PathwayPoint const &in) {
            xx::Append(s, '[', in.pos.x, ',', in.pos.y, ',', in.a, ',', in.d, ']');
        }
    };


    // 适配 Pathway 之 序列化 & 反序列化
    template<>
    struct DataFuncs<Pathway, void> {
        static inline void Write(DataWriter &dw, Pathway const &in) {
            dw.Write(in.isLoop, in.points);
        }

        static inline int Read(DataReader &dr, Pathway &out) {
            return dr.Read(out.isLoop, out.points);
        }
    };

    // 适配 Pathway 之 ToString
    template<>
    struct StringFuncs<Pathway, void> {
        static inline void Append(std::string &s, Pathway const &in) {
            xx::Append(s, "{\"isLoop\"", in.isLoop, ",\"points\"", in.points, '}');
        }
    };






    // 弧度 转为角度要 * (180.0f / float(M_PI))

    // 计算直线的弧度
    template<typename Point1, typename Point2>
    float GetAngle(Point1 const &from, Point2 const &to) noexcept {
        if (from.x == to.x && from.y == to.y) return 0.0f;
        auto &&len_y = to.y - from.y;
        auto &&len_x = to.x - from.x;
        return atan2f(len_y, len_x);
    }

    // 计算距离
    template<typename Point1, typename Point2>
    float GetDistance(Point1 const &a, Point2 const &b) noexcept {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return sqrtf(dx * dx + dy * dy);
    }

    // 点围绕 0,0 为中心旋转 a 弧度   ( 角度 * (float(M_PI) / 180.0f) )
    template<typename Point>
    inline Point Rotate(Point const &pos, float const &a) noexcept {
        auto &&sinA = sinf(a);
        auto &&cosA = cosf(a);
        return Point{pos.x * cosA - pos.y * sinA, pos.x * sinA + pos.y * cosA};
    }

    // 判断两线段( p0-p1, p2-p3 )是否相交, 并且往 p 填充交点
    template<typename Point>
    inline bool GetSegmentIntersection(Point const &p0, Point const &p1, Point const &p2, Point const &p3, Point *const &p = nullptr) noexcept {
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

        t = t_numer / denom;        // Collision detected
        if (p) {
            p->x = p0.x + (t * s10.x);
            p->y = p0.y + (t * s10.y);
        }
        return true;
    }

    // 途径点曲线( p p p ... ) 转 2 控制点的贝塞尔( p c c p c c p ... )
    template<typename Point>
    void CurveToBezier(std::vector<Point> &bs, std::vector<Point> const &cs) {
        auto n = cs.size();
        auto len = n * 3 - 2;
        bs.resize(len);

        bs[0] = cs[0];
        bs[1] = (cs[1] - cs[0]) * cs[0].tension + cs[0];

        for (size_t i = 0; i < n - 2; i++) {
            auto diff = (cs[i + 2] - cs[i]) * cs[i].tension;
            bs[3 * i + 2] = cs[i + 1] - diff;
            bs[3 * i + 3] = cs[i + 1];
            bs[3 * i + 4] = cs[i + 1] + diff;
        }
        bs[len - 2] = (cs[n - 2] - cs[n - 1]) * cs[n - 2].tension + cs[n - 1];
        bs[len - 1] = cs[n - 1];
    }

    // 2 控制点的贝塞尔( p c c p c c p ... ) 切片转点
    template<typename Point1, typename Point2>
    void BezierToPoints(std::vector<Point1> &ps, std::vector<Point2> const &bs) {
        auto len = (bs.size() - 1) / 3;
        size_t totalSegments = 0;
        for (size_t j = 0; j < len; ++j) {
            totalSegments += bs[j * 3].numSegments;
        }
        ps.resize(totalSegments);
        totalSegments = 0;
        for (size_t j = 0; j < len; ++j) {
            auto idx = j * 3;
            auto numSegments = bs[idx].numSegments;
            auto step = 1.0f / numSegments;
            for (int i = 0; i < numSegments; ++i) {
                auto t = step * i;
                auto t1 = 1 - t;
                ps[totalSegments + i] = bs[idx] * t1 * t1 * t1
                                        + bs[idx + 1] * 3 * t * t1 * t1
                                        + bs[idx + 2] * 3 * t * t * (1 - t)
                                        + bs[idx + 3] * t * t * t;
            }
            totalSegments += numSegments;
        }
    }


    inline bool Pathway::MoveForward(float total, size_t &i, float &d, xx::Point &pos, float &a) const {
        auto siz = points.size();
        LabBegin:
        auto left = points[i].d - d;
        // 总距离大于当前点剩余距离：从 total 中减去, 剩余距离清0, i 指向下一个点
        if (total > left) {
            ++i;
            total -= left;
            d = 0;
            // 循环路线: i 越界就指向头部
            if (isLoop && i == siz) {
                i = 0;
            }
                // 非循环路线：i 指向尾部就返回 true
            else if (i + 1 == siz) {
                pos = points[i].pos;
                a = points[i].a;
                return true;
            }
            // 继续移动
            goto LabBegin;
        }
            // 记入当前点已移动距离
        else {
            d += total;
        }
        // 根据当前点上已经前进的距离, 结合下一个点的位置算坐标
        pos = points[i].pos + ((points[i == siz - 1 ? 0 : i + 1].pos - points[i].pos) * (d / points[i].d));
        a = points[i].a;
        return false;
    }

    inline bool Pathway::MoveBack(float total, size_t &i, float &d, xx::Point &pos, float &a) const {
        auto siz = points.size();
        LabBegin:
        if (total > d) {
            if (isLoop) {
                i = i ? (i - 1) : (siz - 1);
                total -= d;
            } else {
                if (i == 0) {
                    d = 0;
                    pos = points[0].pos;
                    a = points[0].a;
                    return true;
                }
                --i;
            }
            d = points[i].d;
            goto LabBegin;
        } else {
            d -= total;
        }
        pos = points[i].pos + ((points[i == siz - 1 ? 0 : i + 1].pos - points[i].pos) * (d / points[i].d));
        a = points[i].a;
        return false;
    }


    inline void Pathway::FillDA() {
        auto n = points.size() - 1;
        size_t i = 0;
        for (; i < n; ++i) {
            points[i].a = GetAngle(points[i].pos, points[i + 1].pos);
            points[i].d = GetDistance(points[i].pos, points[i + 1].pos);
        }
        if (isLoop) {
            points[i].a = GetAngle(points[i].pos, points[0].pos);
            points[i].d = GetDistance(points[i].pos, points[0].pos);
        } else {
            points[i].a = points[i - 1].a;
            points[i].d = 0;
        }
    }


    inline PathwayMaker::PathwayMaker(xx::Point const &pos) {
        xx::MakeTo(pathway);
        pathway->points.emplace_back(pos);
    }

    inline PathwayMaker &PathwayMaker::Forward(float const &d) {
        auto a = pathway->points.back().a;
        pathway->points.emplace_back(xx::Rotate(xx::Point{d, 0}, a)).a = a;
        return *this;
    }

    inline PathwayMaker &PathwayMaker::RotateTo(float const &a) {
        pathway->points.back().a = a;
        return *this;
    }
    inline PathwayMaker &PathwayMaker::RotateBy(float const &a) {
        pathway->points.back().a += a;
        return *this;
    }

    inline PathwayMaker &PathwayMaker::To(xx::Point const &tarPos) {
        auto &&p = pathway->points.back();
        auto a = p.a = GetAngle(p.pos, tarPos);
        p.d = GetDistance(p.pos, tarPos);
        pathway->points.emplace_back(tarPos).a = a;
        return *this;
    }

    inline std::shared_ptr<Pathway> PathwayMaker::Loop() {
        auto &&p1 = pathway->points.back();
        auto &&p2 = pathway->points[0];
        p1.a = GetAngle(p1.pos, p2.pos);
        p1.d = GetDistance(p1.pos, p2.pos);
        pathway->isLoop = true;
        return pathway;
    }

    inline std::shared_ptr<Pathway> PathwayMaker::End() const {
        return pathway;
    }

}
