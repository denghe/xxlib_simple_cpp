#pragma once
#include "xx_object.h"
#include "TimeLineConfig_class_lite.h.inc"  // user create it for extend include files
namespace TimeLineConfig {
	struct PkgGenMd5 {
		inline static const std::string value = "#*MD5<1ea2dbf166048fd0115d8d06baa48b27>*#";
    };
	struct PkgGenTypes {
        static void RegisterTo(xx::ObjectHelper& oh);
    };

}
namespace xx {
}
namespace TimeLineConfig {

    // 碰撞圆
    struct CDCircle {
        XX_GENCODE_STRUCT_H(CDCircle)
        float x = 0;
        float y = 0;
        float r = 0;
    };
    // 锁定点
    struct LockPoint {
        XX_GENCODE_STRUCT_H(LockPoint)
        float x = 0;
        float y = 0;
    };
    // 碰撞圆集合
    struct CDCircles {
        XX_GENCODE_STRUCT_H(CDCircles)
        // 最大碰撞圆范围（外包围圆），用于碰撞检测粗判
        TimeLineConfig::CDCircle maxCDCircle;
        // 具体碰撞圆列表，用于碰撞检测遍历细判
        std::vector<TimeLineConfig::CDCircle> cdCircles;
    };
    // 锁定点集合
    struct LockPoints {
        XX_GENCODE_STRUCT_H(LockPoints)
        // 主锁定点。如果出屏幕，则锁定 锁定线与屏幕边缘形成的 交点
        TimeLineConfig::LockPoint mainLockPoint;
        // 锁定线
        std::vector<TimeLineConfig::LockPoint> lockPoints;
    };
    // 时间点
    struct TimePoint {
        XX_GENCODE_STRUCT_H(TimePoint)
        // 起始时间( 秒 )
        float time = 0;
        // 图片切换事件( spine, c3b, server 忽略 )
        std::optional<std::string> pic;
        // 锁定点&线切换事件
        std::optional<TimeLineConfig::LockPoints> lps;
        // 碰撞圈切换事件
        std::optional<TimeLineConfig::CDCircles> cdcs;
        // 移动速度切换事件
        std::optional<float> speed;
    };
    // 时间线
    struct TimeLine {
        XX_GENCODE_STRUCT_H(TimeLine)
        // 总时长( 秒 )
        float totalSeconds = 0;
        // 时间点集合
        std::vector<TimeLineConfig::TimePoint> timePoints;
    };
}
namespace xx {
	template<>
	struct StringFuncsEx<TimeLineConfig::CDCircle, void> {
		static void Append(ObjectHelper &oh, TimeLineConfig::CDCircle const& in);
		static void AppendCore(ObjectHelper &oh, TimeLineConfig::CDCircle const& in);
    };
	template<>
	struct DataFuncsEx<TimeLineConfig::CDCircle, void> {
		static void Write(DataWriterEx& dw, TimeLineConfig::CDCircle const& in);
		static int Read(DataReaderEx& dr, TimeLineConfig::CDCircle& out);
	};
    template<>
	struct CloneFuncs<TimeLineConfig::CDCircle, void> {
		static void Clone1(ObjectHelper &oh, TimeLineConfig::CDCircle const& in, TimeLineConfig::CDCircle& out);
		static void Clone2(ObjectHelper &oh, TimeLineConfig::CDCircle const& in, TimeLineConfig::CDCircle& out);
	};
	template<>
	struct StringFuncsEx<TimeLineConfig::LockPoint, void> {
		static void Append(ObjectHelper &oh, TimeLineConfig::LockPoint const& in);
		static void AppendCore(ObjectHelper &oh, TimeLineConfig::LockPoint const& in);
    };
	template<>
	struct DataFuncsEx<TimeLineConfig::LockPoint, void> {
		static void Write(DataWriterEx& dw, TimeLineConfig::LockPoint const& in);
		static int Read(DataReaderEx& dr, TimeLineConfig::LockPoint& out);
	};
    template<>
	struct CloneFuncs<TimeLineConfig::LockPoint, void> {
		static void Clone1(ObjectHelper &oh, TimeLineConfig::LockPoint const& in, TimeLineConfig::LockPoint& out);
		static void Clone2(ObjectHelper &oh, TimeLineConfig::LockPoint const& in, TimeLineConfig::LockPoint& out);
	};
	template<>
	struct StringFuncsEx<TimeLineConfig::CDCircles, void> {
		static void Append(ObjectHelper &oh, TimeLineConfig::CDCircles const& in);
		static void AppendCore(ObjectHelper &oh, TimeLineConfig::CDCircles const& in);
    };
	template<>
	struct DataFuncsEx<TimeLineConfig::CDCircles, void> {
		static void Write(DataWriterEx& dw, TimeLineConfig::CDCircles const& in);
		static int Read(DataReaderEx& dr, TimeLineConfig::CDCircles& out);
	};
    template<>
	struct CloneFuncs<TimeLineConfig::CDCircles, void> {
		static void Clone1(ObjectHelper &oh, TimeLineConfig::CDCircles const& in, TimeLineConfig::CDCircles& out);
		static void Clone2(ObjectHelper &oh, TimeLineConfig::CDCircles const& in, TimeLineConfig::CDCircles& out);
	};
	template<>
	struct StringFuncsEx<TimeLineConfig::LockPoints, void> {
		static void Append(ObjectHelper &oh, TimeLineConfig::LockPoints const& in);
		static void AppendCore(ObjectHelper &oh, TimeLineConfig::LockPoints const& in);
    };
	template<>
	struct DataFuncsEx<TimeLineConfig::LockPoints, void> {
		static void Write(DataWriterEx& dw, TimeLineConfig::LockPoints const& in);
		static int Read(DataReaderEx& dr, TimeLineConfig::LockPoints& out);
	};
    template<>
	struct CloneFuncs<TimeLineConfig::LockPoints, void> {
		static void Clone1(ObjectHelper &oh, TimeLineConfig::LockPoints const& in, TimeLineConfig::LockPoints& out);
		static void Clone2(ObjectHelper &oh, TimeLineConfig::LockPoints const& in, TimeLineConfig::LockPoints& out);
	};
	template<>
	struct StringFuncsEx<TimeLineConfig::TimePoint, void> {
		static void Append(ObjectHelper &oh, TimeLineConfig::TimePoint const& in);
		static void AppendCore(ObjectHelper &oh, TimeLineConfig::TimePoint const& in);
    };
	template<>
	struct DataFuncsEx<TimeLineConfig::TimePoint, void> {
		static void Write(DataWriterEx& dw, TimeLineConfig::TimePoint const& in);
		static int Read(DataReaderEx& dr, TimeLineConfig::TimePoint& out);
	};
    template<>
	struct CloneFuncs<TimeLineConfig::TimePoint, void> {
		static void Clone1(ObjectHelper &oh, TimeLineConfig::TimePoint const& in, TimeLineConfig::TimePoint& out);
		static void Clone2(ObjectHelper &oh, TimeLineConfig::TimePoint const& in, TimeLineConfig::TimePoint& out);
	};
	template<>
	struct StringFuncsEx<TimeLineConfig::TimeLine, void> {
		static void Append(ObjectHelper &oh, TimeLineConfig::TimeLine const& in);
		static void AppendCore(ObjectHelper &oh, TimeLineConfig::TimeLine const& in);
    };
	template<>
	struct DataFuncsEx<TimeLineConfig::TimeLine, void> {
		static void Write(DataWriterEx& dw, TimeLineConfig::TimeLine const& in);
		static int Read(DataReaderEx& dr, TimeLineConfig::TimeLine& out);
	};
    template<>
	struct CloneFuncs<TimeLineConfig::TimeLine, void> {
		static void Clone1(ObjectHelper &oh, TimeLineConfig::TimeLine const& in, TimeLineConfig::TimeLine& out);
		static void Clone2(ObjectHelper &oh, TimeLineConfig::TimeLine const& in, TimeLineConfig::TimeLine& out);
	};
}