using TemplateLibrary;

/*
帧动画( frames )数据结构：带总时长的时间轴。到达某个时间，就切换成某张图

| pic         pic           pic                             pic
|......|......|......|......|......|......|......|......|......|......|......|...

所有动画类型的动作可能具备下列 3 种时间轴数据：碰撞圈，锁定线，速度变化

| cd                        cd
| lp
| speed       speed             speed  speed  speed  speed
|......|......|......|......|......|......|......|......|......|......|......|...

同一个动作含有的时间轴，因为密度不同，数据存储是分离的。它们结构类似，都是 起始时间点 + 状态数据，总时长依赖于动作本身


运行时播放器接口需求与层级关系：

1. 动画播放器：针对 frames，spine, c3b 的原始存档，能跨类型加载，能切动作，能设置是否循环，能快进/暂停 甚至后退，能指定显示坐标，角度，缩放，隐藏/显示，层级，有播放结束事件
2. 承担部分业务逻辑的移动对象：组合 动画播放器，能加载 ext 附加数据并提供相关功能，能绑移动路径，能在路径上快进/前进/后退/停止. 可设置相对坐标，角度，缩放，有移动结束事件
*/

namespace xx {
    [External]
    struct Random1 { }
}

[TypeId(1)]
class Foo {
    xx.Random1 rnd;
}

[TypeId(2)]
class Node {
    Shared<Node> child;
}




[Include]
[Desc("坐标")]
struct Point {
    float x, y;
};

[Include]
[Desc("移动路线 -- 点")]
struct PathwayPoint {
    [Desc("坐标")]
    Point pos;
    [Desc("角度")]
    float a;
    [Desc("距离")]
    float d;
};

[Include]
[Desc("曲线途经点")]
struct CurvePoint {
    float x;
    float y;
    [Desc("张力")]
    float tension;
    [Desc("切片数")]
    int numSegments;
};

[Include, TypeId(66)]
[Desc("移动路线")]
class Pathway {
    [Desc("是否闭合( 是 则 最后一个点 的下一个指向 第一个点 )")]
    bool isLoop;

    [Desc("点集合")]
    List<PathwayPoint> points;
}



/*******************************************************************************************/
// 精灵帧动画 存盘文件相关
/*******************************************************************************************/

[Struct, Desc("帧动画 存盘文件 .frames")]
class File_Frames {
    [Desc("帧动画集合")]
    List<FrameAnim> frameAnims;
    [Desc("图位于哪些 plist")]
    List<string> plists;
}

[Struct, Desc("帧动画")]
class FrameAnim {
    [Desc("动画名")]
    string name;
    [Desc("总时长( 秒 )")]
    float totalSeconds;
    [Desc("时间点--帧 集合")]
    List<TimePoint_Frame> frames;
}

[Struct, Desc("时间点--帧")]
class TimePoint_Frame {
    [Desc("起始时间( 秒 )")]
    float time;
    [Desc("帧名")]
    string picName;
    // todo: offset?
}


/*******************************************************************************************/
// 动画 存盘文件相关
/*******************************************************************************************/

[Struct, Desc("动画配置 存盘文件 *.anims")]
class File_Anims {
    [Desc("指向原始资源文件名( atlas/spine, c3b, frames )")]
    string resFileName;
    [Desc("动画集合")]    // 手工附加该成员 以防止被序列化
    List<Anim> anims;
    float shadowX, shadowY;
    float shadowScale;
}

[Struct, Desc("针对  等动画文件, 附加 碰撞 & 锁定 等数据")]
class Anim {
    [Desc("动画名")]
    string name;
    [Desc("总时长( 秒 )")]
    float totalSeconds;
    [Desc("时间点--锁定点线 集合")]
    List<TimePoint_LockPoints> lps;
    [Desc("时间点--碰撞圆 集合")]
    List<TimePoint_CDCircles> cds;
}

[Struct, Desc("时间点--锁定点线")]
class TimePoint_LockPoints {
    [Desc("起始时间( 秒 )")]
    float time;
    [Desc("主锁定点。如果出屏幕，则锁定 锁定线与屏幕边缘形成的 交点")]
    LockPoint mainLockPoint;
    [Desc("锁定线")]
    List<LockPoint> lockPoints;
}

[Struct, Desc("锁定点")]
class LockPoint {
    float x;
    float y;
}

[Struct, Desc("时间点--碰撞圆集合")]
class TimePoint_CDCircles {
    [Desc("起始时间( 秒 )")]
    float time;
    [Desc("最大碰撞圆范围（外包围圆），用于碰撞检测粗判")]
    CDCircle maxCDCircle;
    [Desc("具体碰撞圆列表，用于碰撞检测遍历细判")]
    List<CDCircle> cdCircles;
}

[Struct, Include, Desc("碰撞圆")]
class CDCircle {
    float x;
    float y;
    float r;
}


/*******************************************************************************************/
// 动作 存盘文件相关
/*******************************************************************************************/

[Struct, Desc("动作配置 存盘文件 *.acts")]
class File_Actions {
    [Desc("存储 动画配置 文件名. 反序列化后用于填充 anims 部分的内容")]
    string animsFileName;
    // 代码中抠洞附加该成员变量
    //File_Anims anims;
    [Desc("动作集合")]
    List<Action> actions;
}

[Struct, Desc("针对 atlas/spine, c3b, frames 等动画文件, 附加 移动 & 动画切换 等数据")]
class Action {
    [Desc("动作名")]
    string name;
    [Desc("总时长( 秒 )")]
    float totalSeconds;
    [Desc("时间点--移动速度 集合")]
    List<TimePoint_Anim> anims;
    [Desc("时间点--移动速度 集合")]
    List<TimePoint_Speed> speeds;
}

[Struct, Desc("时间点--动画切换( 下标值域为负可用于表达事件 )")]
class TimePoint_Anim {
    [Desc("起始时间( 秒 )")]
    float time;
    [Desc("切换到指定动画的下标")]
    int animIndex;
}

[Struct, Desc("时间点--移动速度")]
class TimePoint_Speed {
    [Desc("起始时间( 秒 )")]
    float time;
    [Desc("每秒移动距离(米)")]
    float speed;
}



/*******************************************************************************************/
// 移动路线 存盘文件
/*******************************************************************************************/

[Struct, Desc("移动路线 存盘文件 *.pathway")]
class File_pathway {
    [Desc("是否闭合. 直线无法闭合。将于头尾多填2点，绘制后裁切掉以确保曲线形状正确")]
    bool isLoop;

    [Desc("点集合. 2 个点为直线，更多点为曲线串联")]
    List<CurvePoint> points;
}






/*******************************************************************************************/
// Fish 相关
/*******************************************************************************************/

[Include, TypeId(100)]
class Fish {
    Point pos;
    float angle;
    float scaleX = 1;
    float scaleY = 1;
    float animElapsedSeconds = 0;
    float totalElapsedSeconds = 0;
    uint pathwayI = 0;
    float pathwayD = 0;
    float speedScale = 1;
    float timeScale = 1;
    uint lpsCursor = 0;
    uint cdsCursor = 0;
    uint ssCursor = 0;
    float speed = 0;
    bool loop = true;
    int id;
    int indexAtContainer;
    long coin;
    double effectiveTime;
    string actionName;
    string fileName;
    string pathwayName;
    string luaName;
    byte[] luaData;
    Shared<Pathway> pathway;
    List<Shared<Fish>> children;
    Point offset;
    File_Action file;
}

[Include, TypeId(101)]
class Player {
    int id;
    string nickname;
    long coin;
    bool autoLock;
    bool autoFire;
    int autoIncId;
    int sitId;
    bool bankrupt;
    List<Shared<Cannon>> cannons;
    List<Shared<Stuff>> stuffs;
    Weak<Fish> aimFish;
}

[Include, TypeId(102)]
class Cannon {
    int id;
    int typeId;
    List<Shared<Bullet>> bullets;
}

[Include, TypeId(103)]
class Bullet {
    int id;
    long coin;
}

[Include, TypeId(104)]
class Stuff {
    int id;
    int typeId;
    Point pos;
    double effectiveTime;
}

[Include, TypeId(105)]
class Root {
    float dtPool;
    int frame;
    List<Shared<Player>> players;
    List<Shared<Fish>> fishs;
}

[Include, TypeId(300)]
class SimpleBullet : Bullet {
    int angle;
    Point pos;
}

[Include, TypeId(301)]
class TrackBullet : SimpleBullet {
    
}

