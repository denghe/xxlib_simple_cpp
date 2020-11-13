#pragma once
#include "xx_obj.h"
#include "FF_class_lite.h.inc"  // user create it for extend include files
namespace FF {
	struct PkgGenMd5 {
		inline static const ::std::string value = "#*MD5<6e0f22cabf2164db9023eaf59a21a308>*#";
    };
	struct PkgGenTypes {
        static void RegisterTo(::xx::ObjManager& om);
    };

    struct Point;
    struct CDCircle;
    struct LockPoint;
    struct TimePoint_Speed;
    struct TimePoint_CDCircles;
    struct TimePoint_LockPoints;
    struct Bullet;
    struct PathwayPoint;
    struct Action_AnimExt;
    struct File_AnimExt;
    struct Pathway;
    struct Cannon;
    struct TimePoint_Frame;
    struct Stuff;
    struct Fish;
    struct CurvePoint;
    struct SimpleBullet;
    struct Player;
    struct Action_Frames;
    struct Root;
    struct Foo;
    struct File_Frames;
    struct File_pathway;
    struct TrackBullet;
}
namespace xx {
    template<> struct TypeId<::FF::Bullet> { static const uint16_t value = 103; };
    template<> struct TypeId<::FF::Pathway> { static const uint16_t value = 66; };
    template<> struct TypeId<::FF::Cannon> { static const uint16_t value = 102; };
    template<> struct TypeId<::FF::Stuff> { static const uint16_t value = 104; };
    template<> struct TypeId<::FF::Fish> { static const uint16_t value = 100; };
    template<> struct TypeId<::FF::SimpleBullet> { static const uint16_t value = 300; };
    template<> struct TypeId<::FF::Player> { static const uint16_t value = 101; };
    template<> struct TypeId<::FF::Root> { static const uint16_t value = 105; };
    template<> struct TypeId<::FF::Foo> { static const uint16_t value = 12; };
    template<> struct TypeId<::FF::TrackBullet> { static const uint16_t value = 301; };
}
namespace FF {

    // 坐标
    struct Point {
        XX_GENCODE_STRUCT_H(Point)
        float x = 0.0f;
        float y = 0.0f;
#include "FF_Point.inc"
    };
    // 碰撞圆
    struct CDCircle {
        XX_GENCODE_STRUCT_H(CDCircle)
        float x = 0.0f;
        float y = 0.0f;
        float r = 0.0f;
#include "FF_CDCircle.inc"
    };
    // 锁定点
    struct LockPoint {
        XX_GENCODE_STRUCT_H(LockPoint)
        float x = 0.0f;
        float y = 0.0f;
    };
    // 时间点--移动速度
    struct TimePoint_Speed {
        XX_GENCODE_STRUCT_H(TimePoint_Speed)
        // 起始时间( 秒 )
        float time = 0.0f;
        // 每秒移动距离(米)
        float speed = 0.0f;
    };
    // 时间点--碰撞圆集合
    struct TimePoint_CDCircles {
        XX_GENCODE_STRUCT_H(TimePoint_CDCircles)
        // 起始时间( 秒 )
        float time = 0.0f;
        // 最大碰撞圆范围（外包围圆），用于碰撞检测粗判
        ::FF::CDCircle maxCDCircle;
        // 具体碰撞圆列表，用于碰撞检测遍历细判
        ::std::vector<::FF::CDCircle> cdCircles;
    };
    // 时间点--锁定点线
    struct TimePoint_LockPoints {
        XX_GENCODE_STRUCT_H(TimePoint_LockPoints)
        // 起始时间( 秒 )
        float time = 0.0f;
        // 主锁定点。如果出屏幕，则锁定 锁定线与屏幕边缘形成的 交点
        ::FF::LockPoint mainLockPoint;
        // 锁定线
        ::std::vector<::FF::LockPoint> lockPoints;
    };
    // 移动路线 -- 点
    struct PathwayPoint {
        XX_GENCODE_STRUCT_H(PathwayPoint)
        // 坐标
        ::FF::Point pos;
        // 角度
        float a = 0.0f;
        // 距离
        float d = 0.0f;
#include "FF_PathwayPoint.inc"
    };
    // 针对 atlas/spine, c3b, frames 等动画文件, 附加 移动 & 碰撞 & 锁定 等数据
    struct Action_AnimExt {
        XX_GENCODE_STRUCT_H(Action_AnimExt)
        // 动作名
        ::std::string name;
        // 总时长( 秒 )
        float totalSeconds = 0.0f;
        // 时间点--锁定点线 集合
        ::std::vector<::FF::TimePoint_LockPoints> lps;
        // 时间点--碰撞圆 集合
        ::std::vector<::FF::TimePoint_CDCircles> cds;
        // 时间点--移动速度 集合
        ::std::vector<::FF::TimePoint_Speed> ss;
    };
    // 针对动画的扩展信息 存盘文件( *.frames.ext, *.atlas.ext, *.c3b.ext 
    struct File_AnimExt {
        XX_GENCODE_STRUCT_H(File_AnimExt)
        // 动作集合
        ::std::vector<::FF::Action_AnimExt> actions;
        float shadowX = 0.0f;
        float shadowY = 0.0f;
        float shadowScale = 0.0f;
    };
    // 时间点--精灵帧
    struct TimePoint_Frame {
        XX_GENCODE_STRUCT_H(TimePoint_Frame)
        // 起始时间( 秒 )
        float time = 0.0f;
        // 精灵帧名称
        ::std::string picName;
    };
    // 曲线途经点
    struct CurvePoint {
        XX_GENCODE_STRUCT_H(CurvePoint)
        float x = 0.0f;
        float y = 0.0f;
        // 张力
        float tension = 0.0f;
        // 切片数
        int32_t numSegments = 0;
#include "FF_CurvePoint.inc"
    };
    // 精灵帧动画--动作( 兼容 spine, c3b, frames )
    struct Action_Frames {
        XX_GENCODE_STRUCT_H(Action_Frames)
        // 动作名
        ::std::string name;
        // 总时长( 秒 )
        float totalSeconds = 0.0f;
        // 时间点--精灵帧 集合
        ::std::vector<::FF::TimePoint_Frame> frames;
    };
    // 精灵帧动画 存盘文件
    struct File_Frames {
        XX_GENCODE_STRUCT_H(File_Frames)
        // 动作集合
        ::std::vector<::FF::Action_Frames> actions;
        // 图位于哪些 plist
        ::std::vector<::std::string> plists;
    };
    // 移动路线 存盘文件 *.pathway
    struct File_pathway {
        XX_GENCODE_STRUCT_H(File_pathway)
        // 是否闭合. 直线无法闭合。将于头尾多填2点，绘制后裁切掉以确保曲线形状正确
        bool isLoop = false;
        // 点集合. 2 个点为直线，更多点为曲线串联
        ::std::vector<::FF::CurvePoint> points;
    };
    struct Bullet : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Bullet, ::xx::ObjBase)
        int32_t id = 0;
        int64_t coin = 0;
#include "FF_Bullet.inc"
    };
    // 移动路线
    struct Pathway : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Pathway, ::xx::ObjBase)
        // 是否闭合( 是 则 最后一个点 的下一个指向 第一个点 )
        bool isLoop = false;
        // 点集合
        ::std::vector<::FF::PathwayPoint> points;
#include "FF_Pathway.inc"
    };
    struct Cannon : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Cannon, ::xx::ObjBase)
        int32_t id = 0;
        int32_t typeId = 0;
        ::std::vector<::xx::Shared<::FF::Bullet>> bullets;
#include "FF_Cannon.inc"
    };
    struct Stuff : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Stuff, ::xx::ObjBase)
        int32_t id = 0;
        int32_t typeId = 0;
        ::FF::Point pos;
        double effectiveTime = 0;
#include "FF_Stuff.inc"
    };
    struct Fish : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Fish, ::xx::ObjBase)
        ::FF::Point pos;
        float angle = 0.0f;
        float scaleX = 1.0f;
        float scaleY = 1.0f;
        float animElapsedSeconds = 0.0f;
        float totalElapsedSeconds = 0.0f;
        uint32_t pathwayI = 0;
        float pathwayD = 0.0f;
        float speedScale = 1.0f;
        float timeScale = 1.0f;
        uint32_t lpsCursor = 0;
        uint32_t cdsCursor = 0;
        uint32_t ssCursor = 0;
        float speed = 0.0f;
        bool loop = true;
        int32_t id = 0;
        int32_t indexAtContainer = 0;
        int64_t coin = 0;
        double effectiveTime = 0;
        ::std::string actionName;
        ::std::string fileName;
        ::std::string pathwayName;
        ::std::string luaName;
        ::xx::Data luaData;
        ::xx::Shared<::FF::Pathway> pathway;
        ::std::vector<::xx::Shared<::FF::Fish>> children;
        ::FF::Point offset;
        ::FF::File_AnimExt file;
#include "FF_Fish.inc"
    };
    struct SimpleBullet : ::FF::Bullet {
        XX_GENCODE_OBJECT_H(SimpleBullet, ::FF::Bullet)
        int32_t angle = 0;
        ::FF::Point pos;
#include "FF_SimpleBullet.inc"
    };
    struct Player : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Player, ::xx::ObjBase)
        int32_t id = 0;
        ::std::string nickname;
        int64_t coin = 0;
        bool autoLock = false;
        bool autoFire = false;
        int32_t autoIncId = 0;
        int32_t sitId = 0;
        bool bankrupt = false;
        ::std::vector<::xx::Shared<::FF::Cannon>> cannons;
        ::std::vector<::xx::Shared<::FF::Stuff>> stuffs;
        ::xx::Weak<::FF::Fish> aimFish;
#include "FF_Player.inc"
    };
    struct Root : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Root, ::xx::ObjBase)
        float dtPool = 0.0f;
        int32_t frame = 0;
        ::std::vector<::xx::Shared<::FF::Player>> players;
        ::std::vector<::xx::Shared<::FF::Fish>> fishs;
#include "FF_Root.inc"
    };
    struct Foo : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Foo, ::xx::ObjBase)
        ::xx::Random1 rnd;
    };
    struct TrackBullet : ::FF::SimpleBullet {
        XX_GENCODE_OBJECT_H(TrackBullet, ::FF::SimpleBullet)
#include "FF_TrackBullet.inc"
    };
}
namespace xx {
	template<>
	struct ObjFuncs<::FF::Point, void> {
		static void Write(ObjManager& om, ::FF::Point const& in);
		static int Read(ObjManager& om, ::FF::Point& out);
		static void ToString(ObjManager& om, ::FF::Point const& in);
		static void ToStringCore(ObjManager& om, ::FF::Point const& in);
		static void Clone1(ObjManager& om, ::FF::Point const& in, ::FF::Point& out);
		static void Clone2(ObjManager& om, ::FF::Point const& in, ::FF::Point& out);
		static int RecursiveCheck(ObjManager& om, ::FF::Point const& in);
		static void RecursiveReset(ObjManager& om, ::FF::Point& in);
		static void SetDefaultValue(ObjManager& om, ::FF::Point& in);
	};
	template<>
	struct ObjFuncs<::FF::CDCircle, void> {
		static void Write(ObjManager& om, ::FF::CDCircle const& in);
		static int Read(ObjManager& om, ::FF::CDCircle& out);
		static void ToString(ObjManager& om, ::FF::CDCircle const& in);
		static void ToStringCore(ObjManager& om, ::FF::CDCircle const& in);
		static void Clone1(ObjManager& om, ::FF::CDCircle const& in, ::FF::CDCircle& out);
		static void Clone2(ObjManager& om, ::FF::CDCircle const& in, ::FF::CDCircle& out);
		static int RecursiveCheck(ObjManager& om, ::FF::CDCircle const& in);
		static void RecursiveReset(ObjManager& om, ::FF::CDCircle& in);
		static void SetDefaultValue(ObjManager& om, ::FF::CDCircle& in);
	};
	template<>
	struct ObjFuncs<::FF::LockPoint, void> {
		static void Write(ObjManager& om, ::FF::LockPoint const& in);
		static int Read(ObjManager& om, ::FF::LockPoint& out);
		static void ToString(ObjManager& om, ::FF::LockPoint const& in);
		static void ToStringCore(ObjManager& om, ::FF::LockPoint const& in);
		static void Clone1(ObjManager& om, ::FF::LockPoint const& in, ::FF::LockPoint& out);
		static void Clone2(ObjManager& om, ::FF::LockPoint const& in, ::FF::LockPoint& out);
		static int RecursiveCheck(ObjManager& om, ::FF::LockPoint const& in);
		static void RecursiveReset(ObjManager& om, ::FF::LockPoint& in);
		static void SetDefaultValue(ObjManager& om, ::FF::LockPoint& in);
	};
	template<>
	struct ObjFuncs<::FF::TimePoint_Speed, void> {
		static void Write(ObjManager& om, ::FF::TimePoint_Speed const& in);
		static int Read(ObjManager& om, ::FF::TimePoint_Speed& out);
		static void ToString(ObjManager& om, ::FF::TimePoint_Speed const& in);
		static void ToStringCore(ObjManager& om, ::FF::TimePoint_Speed const& in);
		static void Clone1(ObjManager& om, ::FF::TimePoint_Speed const& in, ::FF::TimePoint_Speed& out);
		static void Clone2(ObjManager& om, ::FF::TimePoint_Speed const& in, ::FF::TimePoint_Speed& out);
		static int RecursiveCheck(ObjManager& om, ::FF::TimePoint_Speed const& in);
		static void RecursiveReset(ObjManager& om, ::FF::TimePoint_Speed& in);
		static void SetDefaultValue(ObjManager& om, ::FF::TimePoint_Speed& in);
	};
	template<>
	struct ObjFuncs<::FF::TimePoint_CDCircles, void> {
		static void Write(ObjManager& om, ::FF::TimePoint_CDCircles const& in);
		static int Read(ObjManager& om, ::FF::TimePoint_CDCircles& out);
		static void ToString(ObjManager& om, ::FF::TimePoint_CDCircles const& in);
		static void ToStringCore(ObjManager& om, ::FF::TimePoint_CDCircles const& in);
		static void Clone1(ObjManager& om, ::FF::TimePoint_CDCircles const& in, ::FF::TimePoint_CDCircles& out);
		static void Clone2(ObjManager& om, ::FF::TimePoint_CDCircles const& in, ::FF::TimePoint_CDCircles& out);
		static int RecursiveCheck(ObjManager& om, ::FF::TimePoint_CDCircles const& in);
		static void RecursiveReset(ObjManager& om, ::FF::TimePoint_CDCircles& in);
		static void SetDefaultValue(ObjManager& om, ::FF::TimePoint_CDCircles& in);
	};
	template<>
	struct ObjFuncs<::FF::TimePoint_LockPoints, void> {
		static void Write(ObjManager& om, ::FF::TimePoint_LockPoints const& in);
		static int Read(ObjManager& om, ::FF::TimePoint_LockPoints& out);
		static void ToString(ObjManager& om, ::FF::TimePoint_LockPoints const& in);
		static void ToStringCore(ObjManager& om, ::FF::TimePoint_LockPoints const& in);
		static void Clone1(ObjManager& om, ::FF::TimePoint_LockPoints const& in, ::FF::TimePoint_LockPoints& out);
		static void Clone2(ObjManager& om, ::FF::TimePoint_LockPoints const& in, ::FF::TimePoint_LockPoints& out);
		static int RecursiveCheck(ObjManager& om, ::FF::TimePoint_LockPoints const& in);
		static void RecursiveReset(ObjManager& om, ::FF::TimePoint_LockPoints& in);
		static void SetDefaultValue(ObjManager& om, ::FF::TimePoint_LockPoints& in);
	};
	template<>
	struct ObjFuncs<::FF::PathwayPoint, void> {
		static void Write(ObjManager& om, ::FF::PathwayPoint const& in);
		static int Read(ObjManager& om, ::FF::PathwayPoint& out);
		static void ToString(ObjManager& om, ::FF::PathwayPoint const& in);
		static void ToStringCore(ObjManager& om, ::FF::PathwayPoint const& in);
		static void Clone1(ObjManager& om, ::FF::PathwayPoint const& in, ::FF::PathwayPoint& out);
		static void Clone2(ObjManager& om, ::FF::PathwayPoint const& in, ::FF::PathwayPoint& out);
		static int RecursiveCheck(ObjManager& om, ::FF::PathwayPoint const& in);
		static void RecursiveReset(ObjManager& om, ::FF::PathwayPoint& in);
		static void SetDefaultValue(ObjManager& om, ::FF::PathwayPoint& in);
	};
	template<>
	struct ObjFuncs<::FF::Action_AnimExt, void> {
		static void Write(ObjManager& om, ::FF::Action_AnimExt const& in);
		static int Read(ObjManager& om, ::FF::Action_AnimExt& out);
		static void ToString(ObjManager& om, ::FF::Action_AnimExt const& in);
		static void ToStringCore(ObjManager& om, ::FF::Action_AnimExt const& in);
		static void Clone1(ObjManager& om, ::FF::Action_AnimExt const& in, ::FF::Action_AnimExt& out);
		static void Clone2(ObjManager& om, ::FF::Action_AnimExt const& in, ::FF::Action_AnimExt& out);
		static int RecursiveCheck(ObjManager& om, ::FF::Action_AnimExt const& in);
		static void RecursiveReset(ObjManager& om, ::FF::Action_AnimExt& in);
		static void SetDefaultValue(ObjManager& om, ::FF::Action_AnimExt& in);
	};
	template<>
	struct ObjFuncs<::FF::File_AnimExt, void> {
		static void Write(ObjManager& om, ::FF::File_AnimExt const& in);
		static int Read(ObjManager& om, ::FF::File_AnimExt& out);
		static void ToString(ObjManager& om, ::FF::File_AnimExt const& in);
		static void ToStringCore(ObjManager& om, ::FF::File_AnimExt const& in);
		static void Clone1(ObjManager& om, ::FF::File_AnimExt const& in, ::FF::File_AnimExt& out);
		static void Clone2(ObjManager& om, ::FF::File_AnimExt const& in, ::FF::File_AnimExt& out);
		static int RecursiveCheck(ObjManager& om, ::FF::File_AnimExt const& in);
		static void RecursiveReset(ObjManager& om, ::FF::File_AnimExt& in);
		static void SetDefaultValue(ObjManager& om, ::FF::File_AnimExt& in);
	};
	template<>
	struct ObjFuncs<::FF::TimePoint_Frame, void> {
		static void Write(ObjManager& om, ::FF::TimePoint_Frame const& in);
		static int Read(ObjManager& om, ::FF::TimePoint_Frame& out);
		static void ToString(ObjManager& om, ::FF::TimePoint_Frame const& in);
		static void ToStringCore(ObjManager& om, ::FF::TimePoint_Frame const& in);
		static void Clone1(ObjManager& om, ::FF::TimePoint_Frame const& in, ::FF::TimePoint_Frame& out);
		static void Clone2(ObjManager& om, ::FF::TimePoint_Frame const& in, ::FF::TimePoint_Frame& out);
		static int RecursiveCheck(ObjManager& om, ::FF::TimePoint_Frame const& in);
		static void RecursiveReset(ObjManager& om, ::FF::TimePoint_Frame& in);
		static void SetDefaultValue(ObjManager& om, ::FF::TimePoint_Frame& in);
	};
	template<>
	struct ObjFuncs<::FF::CurvePoint, void> {
		static void Write(ObjManager& om, ::FF::CurvePoint const& in);
		static int Read(ObjManager& om, ::FF::CurvePoint& out);
		static void ToString(ObjManager& om, ::FF::CurvePoint const& in);
		static void ToStringCore(ObjManager& om, ::FF::CurvePoint const& in);
		static void Clone1(ObjManager& om, ::FF::CurvePoint const& in, ::FF::CurvePoint& out);
		static void Clone2(ObjManager& om, ::FF::CurvePoint const& in, ::FF::CurvePoint& out);
		static int RecursiveCheck(ObjManager& om, ::FF::CurvePoint const& in);
		static void RecursiveReset(ObjManager& om, ::FF::CurvePoint& in);
		static void SetDefaultValue(ObjManager& om, ::FF::CurvePoint& in);
	};
	template<>
	struct ObjFuncs<::FF::Action_Frames, void> {
		static void Write(ObjManager& om, ::FF::Action_Frames const& in);
		static int Read(ObjManager& om, ::FF::Action_Frames& out);
		static void ToString(ObjManager& om, ::FF::Action_Frames const& in);
		static void ToStringCore(ObjManager& om, ::FF::Action_Frames const& in);
		static void Clone1(ObjManager& om, ::FF::Action_Frames const& in, ::FF::Action_Frames& out);
		static void Clone2(ObjManager& om, ::FF::Action_Frames const& in, ::FF::Action_Frames& out);
		static int RecursiveCheck(ObjManager& om, ::FF::Action_Frames const& in);
		static void RecursiveReset(ObjManager& om, ::FF::Action_Frames& in);
		static void SetDefaultValue(ObjManager& om, ::FF::Action_Frames& in);
	};
	template<>
	struct ObjFuncs<::FF::File_Frames, void> {
		static void Write(ObjManager& om, ::FF::File_Frames const& in);
		static int Read(ObjManager& om, ::FF::File_Frames& out);
		static void ToString(ObjManager& om, ::FF::File_Frames const& in);
		static void ToStringCore(ObjManager& om, ::FF::File_Frames const& in);
		static void Clone1(ObjManager& om, ::FF::File_Frames const& in, ::FF::File_Frames& out);
		static void Clone2(ObjManager& om, ::FF::File_Frames const& in, ::FF::File_Frames& out);
		static int RecursiveCheck(ObjManager& om, ::FF::File_Frames const& in);
		static void RecursiveReset(ObjManager& om, ::FF::File_Frames& in);
		static void SetDefaultValue(ObjManager& om, ::FF::File_Frames& in);
	};
	template<>
	struct ObjFuncs<::FF::File_pathway, void> {
		static void Write(ObjManager& om, ::FF::File_pathway const& in);
		static int Read(ObjManager& om, ::FF::File_pathway& out);
		static void ToString(ObjManager& om, ::FF::File_pathway const& in);
		static void ToStringCore(ObjManager& om, ::FF::File_pathway const& in);
		static void Clone1(ObjManager& om, ::FF::File_pathway const& in, ::FF::File_pathway& out);
		static void Clone2(ObjManager& om, ::FF::File_pathway const& in, ::FF::File_pathway& out);
		static int RecursiveCheck(ObjManager& om, ::FF::File_pathway const& in);
		static void RecursiveReset(ObjManager& om, ::FF::File_pathway& in);
		static void SetDefaultValue(ObjManager& om, ::FF::File_pathway& in);
	};
}
#include "FF_class_lite_.h.inc"  // user create it for extend include files at the end
