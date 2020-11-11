#include "FF_class_lite.h"
#include "FF_Math.h"
namespace FF {

	bool Pathway::Forward(float total, uint32_t& i, float& d, Point& pos, float& a) const {
		auto siz = points.size();
	LabBegin:
		auto left = points[i].d - d;
		// 总距离大于当前点剩余距离：从 total 中减去, 剩余距离清0, i 指向下一个点
		if (total >= left) {
			// 从 dist 中减去当前路点剩余距离
			total -= left;
			d = 0;
			if (isLoop) {
				// 指向下一个路点, 如果到终点, 就指向起点
				if (++i == siz) {
					i = 0;
				}
				// 继续 while 从 dist 减 p->distance
				goto LabBegin;
			}
			// 如果还有下一个路点存在( 最后一个点不算 )
			else if (i + 2 < siz) {
				++i;
				goto LabBegin;
			}
			else {
				pos = points[i].pos;
				a = points[i].a;
				return true;
			}
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

	bool Pathway::Backward(float total, uint32_t& i, float& d, Point& pos, float& a) const {
		auto siz = points.size();
	LabBegin:
		if (total >= d) {
			total -= d;
			if (isLoop) {
				i = i ? (i - 1) : (uint32_t)(siz - 1);
			}
			else {
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
		}
		else {
			d -= total;
		}
		pos = points[i].pos + ((points[i == siz - 1 ? 0 : i + 1].pos - points[i].pos) * (d / points[i].d));
		a = points[i].a;
		return false;
	}


	void Pathway::Begin(uint32_t& i, float& d, Point& pos, float& a) const {
		i = 0;
		d = 0;
		pos = points[0].pos;
		a = points[0].a;
	}

	void Pathway::End(uint32_t& i, float& d, Point& pos, float& a) const {
		i = (uint32_t)points.size() - 1;
		d = points[i].d;
		pos = points[i].pos;
		a = points[i].a;
	}

	void Pathway::FillDA() {
		auto n = points.size() - 1;
		size_t i = 0;
		for (; i < n; ++i) {
			points[i].a = GetAngle(points[i].pos, points[i + 1].pos);
			points[i].d = GetDistance(points[i].pos, points[i + 1].pos);
		}
		if (isLoop) {
			points[i].a = GetAngle(points[i].pos, points[0].pos);
			points[i].d = GetDistance(points[i].pos, points[0].pos);
		}
		else {
			points[i].a = points[i - 1].a;
			points[i].d = 0;
		}
	}

	xx::Shared<Pathway> Pathway::MakeCurve(bool const& isLoop, std::vector<CurvePoint> const& ps) {
		auto rtv = xx::MakeShared<Pathway>();
		rtv->isLoop = isLoop;

		auto siz = ps.size();
		// 点数量需要 >= 2
		if (siz < 2) return nullptr;

		auto&& rps = rtv->points;

		// 如果只有 2 个点：直线
		if (siz == 2) {
			// 填充第一个点: 计算到第二个点的角度和距离
			rps.emplace_back(ps[0].x, ps[0].y
				, GetAngle(ps[0], ps[1])
				, GetDistance(ps[0], ps[1]));
			rtv->isLoop = isLoop;
			// 填充第二个点
			if (isLoop) {
				// 不断来回弹的直线? 角度相反，距离相同
				rps.emplace_back(ps[1].x, ps[1].y
					, rps[0].a + (float)M_PI
					, rps[0].d);
			}
			else {
				// 角度相同，距离为 0
				rps.emplace_back(ps[1].x, ps[1].y, rps[0].a, 0.f);
			}
		}
		// 曲线
		else {
			std::vector<CurvePoint> bs, cs;

			// 曲线 转为 带控制点的贝塞尔
			if (isLoop) {
				// 循环曲线则前后各追加几个点算控制点
				bs.push_back(ps[siz - 3]);
				bs.push_back(ps[siz - 2]);
				bs.push_back(ps[siz - 1]);
				bs.insert(bs.end(), ps.begin(), ps.end());
				bs.push_back(ps[0]);
				bs.push_back(ps[1]);
				bs.push_back(ps[2]);
				CurveToBezier(cs, bs);
				bs.clear();
				// 移除追加
				cs.resize(cs.size() - 6);
				cs.erase(cs.begin(), cs.begin() + 9);
			}
			else {
				CurveToBezier(cs, ps);
			}

			// 带控制点的贝塞尔 转为 点集合
			BezierToPoints(rps, cs);
			cs.clear();

			// 填充距离和角度
			rtv->FillDA();
		}

		return rtv;
	}
}
