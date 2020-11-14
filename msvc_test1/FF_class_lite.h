#pragma once
#include "xx_obj.h"
#include "FF_class_lite.h.inc"  // user create it for extend include files
namespace FF {
	struct PkgGenMd5 {
		inline static const ::std::string value = "#*MD5<5eed49720000083228fb631398592107>*#";
    };
	struct PkgGenTypes {
        static void RegisterTo(::xx::ObjManager& om);
    };

    struct Point;
    struct CDCircle;
    struct LockPoint;
    struct Bullet;
    struct TimePoint_LockPoints;
    struct TimePoint_CDCircles;
    struct TimePoint_Speed;
    struct Action_AnimExt;
    struct PathwayPoint;
    struct Pathway;
    struct File_AnimExt;
    struct Cannon;
    struct Fish;
    struct Stuff;
    struct TimePoint_Frame;
    struct Player;
    struct Foo;
    struct Action_Frames;
    struct SimpleBullet;
    struct CurvePoint;
    struct File_Frames;
    struct Foo2;
    struct Root;
    struct File_pathway;
    struct TrackBullet;
}
namespace xx {
    template<> struct TypeId<::FF::Bullet> { static const uint16_t value = 103; };
    template<> struct TypeId<::FF::Pathway> { static const uint16_t value = 66; };
    template<> struct TypeId<::FF::Cannon> { static const uint16_t value = 102; };
    template<> struct TypeId<::FF::Fish> { static const uint16_t value = 100; };
    template<> struct TypeId<::FF::Stuff> { static const uint16_t value = 104; };
    template<> struct TypeId<::FF::Player> { static const uint16_t value = 101; };
    template<> struct TypeId<::FF::Foo> { static const uint16_t value = 1000; };
    template<> struct TypeId<::FF::SimpleBullet> { static const uint16_t value = 300; };
    template<> struct TypeId<::FF::Foo2> { static const uint16_t value = 1001; };
    template<> struct TypeId<::FF::Root> { static const uint16_t value = 105; };
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
    // 时间点--移动速度
    struct TimePoint_Speed {
        XX_GENCODE_STRUCT_H(TimePoint_Speed)
        // 起始时间( 秒 )
        float time = 0.0f;
        // 每秒移动距离(米)
        float speed = 0.0f;
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
    struct Stuff : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Stuff, ::xx::ObjBase)
        int32_t id = 0;
        int32_t typeId = 0;
        ::FF::Point pos;
        double effectiveTime = 0;
#include "FF_Stuff.inc"
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
    struct Foo : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Foo, ::xx::ObjBase)
        int32_t x = 5;
        float y = 0.5f;
        ::std::string name = "sb";
    };
    struct SimpleBullet : ::FF::Bullet {
        XX_GENCODE_OBJECT_H(SimpleBullet, ::FF::Bullet)
        int32_t angle = 0;
        ::FF::Point pos;
#include "FF_SimpleBullet.inc"
    };
    struct Foo2 : ::FF::Foo {
        XX_GENCODE_OBJECT_H(Foo2, ::FF::Foo)
        ::std::optional<::std::string> name;
        ::xx::Shared<::FF::Foo> ptr;
    };
    struct Root : ::xx::ObjBase {
        XX_GENCODE_OBJECT_H(Root, ::xx::ObjBase)
        float dtPool = 0.0f;
        int32_t frame = 0;
        ::std::vector<::xx::Shared<::FF::Player>> players;
        ::std::vector<::xx::Shared<::FF::Fish>> fishs;
#include "FF_Root.inc"
    };
    struct TrackBullet : ::FF::SimpleBullet {
        XX_GENCODE_OBJECT_H(TrackBullet, ::FF::SimpleBullet)
#include "FF_TrackBullet.inc"
    };
}
namespace xx {
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::Point)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::CDCircle)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::LockPoint)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::TimePoint_LockPoints)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::TimePoint_CDCircles)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::TimePoint_Speed)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::Action_AnimExt)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::PathwayPoint)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::File_AnimExt)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::TimePoint_Frame)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::Action_Frames)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::CurvePoint)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::File_Frames)
	XX_GENCODE_STRUCT_TEMPLATE_H(::FF::File_pathway)
}
#include "FF_class_lite_.h.inc"  // user create it for extend include files at the end
