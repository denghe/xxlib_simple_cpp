#pragma warning disable 0169, 0414
using TemplateLibrary;
/*
对于 spine / 3d 来说，应该记录 从某时间起，用什么 CDLP 数据，从某时间起，移动速度变为多少。这是 2 个分离的时间点事件
frames 同理，图片张数和 CDLP 和 移动速度 并没有必然因果关系，frames 只记录有哪些图，每张图滞留时长。
这样一来，就能把 CDLP 数据独立出来，甚至在 spine/3d/frames 之间共享

时间轴效果图. 如下图所示，有 4 种数据在时间轴上存在，彼此之间脱耦

|......|......|......|......|......|......|......|......|......|......|......|...
| pic         pic           pic                             pic
| cd                        cd
| lp
| speed       speed             speed  speed  speed  speed
|......|......|......|......|......|......|......|......|......|......|......|...

这 4 种数据存储容器是分离的。但具备类似的数据结构，即：起始时间点，事件，并且具有相同的结束时间。
故 为方便使用，加载到内存之后可将数据合并
*/


[Struct, Desc("碰撞圆")]
class CDCircle {
    float x;
    float y;
    float r;
};

[Struct, Desc("碰撞圆集合")]
class CDCircles {
    [Desc("最大碰撞圆范围（外包围圆），用于碰撞检测粗判")]
    CDCircle maxCDCircle;
    [Desc("具体碰撞圆列表，用于碰撞检测遍历细判")]
    List<CDCircle> cdCircles;
};

[Struct, Desc("锁定点")]
class LockPoint {
    float x;
    float y;
};

[Struct, Desc("锁定点集合")]
class LockPoints {
    [Desc("主锁定点。如果出屏幕，则锁定 锁定线与屏幕边缘形成的 交点")]
    LockPoint mainLockPoint;
    [Desc("锁定线")]
    List<LockPoint> lockPoints;
};

[Struct, Desc("时间点")]
class TimePoint {
    [Desc("起始时间( 秒 )")]
    float time;
    [Desc("图片切换事件( spine, c3b, server 忽略 )")]
    Nullable<string> pic;
    [Desc("锁定点&线切换事件")]
    Nullable<LockPoints> lps;
    [Desc("碰撞圈切换事件")]
    Nullable<CDCircles> cdcs;
    [Desc("移动速度切换事件")]
    Nullable<float> speed;
};

[Struct, Desc("时间线")]
class TimeLine {
    [Desc("总时长( 秒 )")]
    float totalSeconds;
    [Desc("时间点集合")]
    List<TimePoint> timePoints;
};
