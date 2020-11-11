#include "FF_class_lite.h"
#include "FF_Math.h"
namespace FF {

	PathwayMaker::PathwayMaker(Point const& pos) {
		pathway.Emplace();
		pathway->points.emplace_back(pos);
	}

	PathwayMaker& PathwayMaker::Reset(Point const& pos) {
		pathway.Emplace();
		pathway->points.emplace_back(pos);
		return *this;
	}


	PathwayMaker& PathwayMaker::Forward(float const& d) {
		auto&& p = pathway->points.back();
		p.d = d;
		auto a = p.a;
		auto pos = p.pos;
		pathway->points.emplace_back(Rotate(Point{ d, 0 }, a) + pos).a = a;
		return *this;
	}

	PathwayMaker& PathwayMaker::RotateTo(float const& a) {
		pathway->points.back().a = a;
		return *this;
	}

	PathwayMaker& PathwayMaker::RotateTo(Point const& tarPos) {
		pathway->points.back().a = GetAngle(pathway->points.back().pos, tarPos);
		return *this;
	}

	PathwayMaker& PathwayMaker::RotateBy(float const& a) {
		pathway->points.back().a += a;
		return *this;
	}

	PathwayMaker& PathwayMaker::To(Point const& tarPos) {
		auto&& p = pathway->points.back();
		auto a = p.a = GetAngle(p.pos, tarPos);
		p.d = GetDistance(p.pos, tarPos);
		pathway->points.emplace_back(tarPos).a = a;
		return *this;
	}

	 PathwayMaker& PathwayMaker::ForwardTo(Point const& tarPos, float const& d) {
		pathway->points.back().a = GetAngle(pathway->points.back().pos, tarPos);
		Forward(d);
		return *this;
	}

	PathwayMaker& PathwayMaker::BounceForward(float d, float const& rectX, float const& rectY, float const& rectW, float const& rectH) {
		// todo：根据 rect 求出 4 条边界的坐标，依次和 当前点 前进 d 产生的 线段 判断 相交？
		// 如果有交点，根据边界方位，计算反弹角度? 令 d 减去 出发点到交点的距离？
		// 如果 d 还有剩，使用新的角度前进并找交点？
		return *this;
	}


	xx::Shared<Pathway> PathwayMaker::Loop() {
		auto&& p1 = pathway->points.back();
		auto&& p2 = pathway->points[0];
		p1.a = GetAngle(p1.pos, p2.pos);
		p1.d = GetDistance(p1.pos, p2.pos);
		pathway->isLoop = true;
		return pathway;
	}

	xx::Shared<Pathway> PathwayMaker::End() const {
		return pathway;
	}

}
