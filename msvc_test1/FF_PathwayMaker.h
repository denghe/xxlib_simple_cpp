#include "FF_class_lite.h"
namespace FF {
	// pathway 线段构造器
	struct PathwayMaker {
		// 指向被 make 的 移动路线
		xx::Shared<Pathway> pathway;

		// make pathway 并 push 起始坐标 作为第一个点
		explicit PathwayMaker(Point const& pos);

		PathwayMaker& Reset(Point const& pos);

		// 从最后个点前进一段距离，形成新的点，新点.a = 最后个点.a，新点.d = 0
		PathwayMaker& Forward(float const& d);

		// 改变最后个点角度( = )
		PathwayMaker& RotateTo(float const& a);

		// 改变最后个点角度指向目标坐标
		PathwayMaker& RotateTo(Point const& tarPos);

		// 改变最后个点角度( + )
		PathwayMaker& RotateBy(float const& a);

		// 令最后个点针对 tarPos 计算 a, d, 追加形成新的点，新点.a = 最后个点.a，新点.d = 0
		PathwayMaker& To(Point const& tarPos);

		// RotateTo + Forward
		PathwayMaker& ForwardTo(Point const& tarPos, float const& d);

		// 从最后个点弹跳前进一段距离，遇到边界会反弹( 类似台球 ). 会在改变角度时形成新的节点
		PathwayMaker& BounceForward(float d, float const& rectX, float const& rectY, float const& rectW, float const& rectH);

		// 令最后个点针对第一个点计算 a, d，标记循环 并返回 pathway 容器
		xx::Shared<Pathway> Loop();

		// 返回 pathway 容器
		xx::Shared<Pathway> End() const;

		PathwayMaker() = delete;

		PathwayMaker(PathwayMaker const&) = delete;

		PathwayMaker& operator=(PathwayMaker const&) = delete;
	};
}
