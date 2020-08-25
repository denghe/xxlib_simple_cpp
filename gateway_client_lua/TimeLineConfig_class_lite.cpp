#include "TimeLineConfig_class_lite.h"
#include "TimeLineConfig_class_lite.cpp.inc"
namespace TimeLineConfig {
	void PkgGenTypes::RegisterTo(xx::ObjectHelper& oh) {
	}
}

namespace xx {
    void CloneFuncs<TimeLineConfig::CDCircle>::Clone1(xx::ObjectHelper &oh, TimeLineConfig::CDCircle const& in, TimeLineConfig::CDCircle &out) {
        CloneFuncs<float>::Clone1(oh, in.x, out.x);
        CloneFuncs<float>::Clone1(oh, in.y, out.y);
        CloneFuncs<float>::Clone1(oh, in.r, out.r);
    }
    void CloneFuncs<TimeLineConfig::CDCircle>::Clone2(xx::ObjectHelper &oh, TimeLineConfig::CDCircle const& in, TimeLineConfig::CDCircle &out) {
        CloneFuncs<float>::Clone2(oh, in.x, out.x);
        CloneFuncs<float>::Clone2(oh, in.y, out.y);
        CloneFuncs<float>::Clone2(oh, in.r, out.r);
    }
	void DataFuncsEx<TimeLineConfig::CDCircle, void>::Write(DataWriterEx& dw, TimeLineConfig::CDCircle const& in) {
        dw.Write(in.x);
        dw.Write(in.y);
        dw.Write(in.r);
    }
	int DataFuncsEx<TimeLineConfig::CDCircle, void>::Read(DataReaderEx& d, TimeLineConfig::CDCircle& out) {
        if (int r = d.Read(out.x)) return r;
        if (int r = d.Read(out.y)) return r;
        if (int r = d.Read(out.r)) return r;
        return 0;
    }
	void StringFuncsEx<TimeLineConfig::CDCircle, void>::Append(ObjectHelper &oh, TimeLineConfig::CDCircle const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<TimeLineConfig::CDCircle, void>::AppendCore(ObjectHelper &oh, TimeLineConfig::CDCircle const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"x\":", in.x); 
        xx::AppendEx(oh, ",\"y\":", in.y);
        xx::AppendEx(oh, ",\"r\":", in.r);
    }
    void CloneFuncs<TimeLineConfig::LockPoint>::Clone1(xx::ObjectHelper &oh, TimeLineConfig::LockPoint const& in, TimeLineConfig::LockPoint &out) {
        CloneFuncs<float>::Clone1(oh, in.x, out.x);
        CloneFuncs<float>::Clone1(oh, in.y, out.y);
    }
    void CloneFuncs<TimeLineConfig::LockPoint>::Clone2(xx::ObjectHelper &oh, TimeLineConfig::LockPoint const& in, TimeLineConfig::LockPoint &out) {
        CloneFuncs<float>::Clone2(oh, in.x, out.x);
        CloneFuncs<float>::Clone2(oh, in.y, out.y);
    }
	void DataFuncsEx<TimeLineConfig::LockPoint, void>::Write(DataWriterEx& dw, TimeLineConfig::LockPoint const& in) {
        dw.Write(in.x);
        dw.Write(in.y);
    }
	int DataFuncsEx<TimeLineConfig::LockPoint, void>::Read(DataReaderEx& d, TimeLineConfig::LockPoint& out) {
        if (int r = d.Read(out.x)) return r;
        if (int r = d.Read(out.y)) return r;
        return 0;
    }
	void StringFuncsEx<TimeLineConfig::LockPoint, void>::Append(ObjectHelper &oh, TimeLineConfig::LockPoint const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<TimeLineConfig::LockPoint, void>::AppendCore(ObjectHelper &oh, TimeLineConfig::LockPoint const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"x\":", in.x); 
        xx::AppendEx(oh, ",\"y\":", in.y);
    }
    void CloneFuncs<TimeLineConfig::CDCircles>::Clone1(xx::ObjectHelper &oh, TimeLineConfig::CDCircles const& in, TimeLineConfig::CDCircles &out) {
        CloneFuncs<TimeLineConfig::CDCircle>::Clone1(oh, in.maxCDCircle, out.maxCDCircle);
        CloneFuncs<std::vector<TimeLineConfig::CDCircle>>::Clone1(oh, in.cdCircles, out.cdCircles);
    }
    void CloneFuncs<TimeLineConfig::CDCircles>::Clone2(xx::ObjectHelper &oh, TimeLineConfig::CDCircles const& in, TimeLineConfig::CDCircles &out) {
        CloneFuncs<TimeLineConfig::CDCircle>::Clone2(oh, in.maxCDCircle, out.maxCDCircle);
        CloneFuncs<std::vector<TimeLineConfig::CDCircle>>::Clone2(oh, in.cdCircles, out.cdCircles);
    }
	void DataFuncsEx<TimeLineConfig::CDCircles, void>::Write(DataWriterEx& dw, TimeLineConfig::CDCircles const& in) {
        dw.Write(in.maxCDCircle);
        dw.Write(in.cdCircles);
    }
	int DataFuncsEx<TimeLineConfig::CDCircles, void>::Read(DataReaderEx& d, TimeLineConfig::CDCircles& out) {
        if (int r = d.Read(out.maxCDCircle)) return r;
        if (int r = d.Read(out.cdCircles)) return r;
        return 0;
    }
	void StringFuncsEx<TimeLineConfig::CDCircles, void>::Append(ObjectHelper &oh, TimeLineConfig::CDCircles const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<TimeLineConfig::CDCircles, void>::AppendCore(ObjectHelper &oh, TimeLineConfig::CDCircles const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"maxCDCircle\":", in.maxCDCircle); 
        xx::AppendEx(oh, ",\"cdCircles\":", in.cdCircles);
    }
    void CloneFuncs<TimeLineConfig::LockPoints>::Clone1(xx::ObjectHelper &oh, TimeLineConfig::LockPoints const& in, TimeLineConfig::LockPoints &out) {
        CloneFuncs<TimeLineConfig::LockPoint>::Clone1(oh, in.mainLockPoint, out.mainLockPoint);
        CloneFuncs<std::vector<TimeLineConfig::LockPoint>>::Clone1(oh, in.lockPoints, out.lockPoints);
    }
    void CloneFuncs<TimeLineConfig::LockPoints>::Clone2(xx::ObjectHelper &oh, TimeLineConfig::LockPoints const& in, TimeLineConfig::LockPoints &out) {
        CloneFuncs<TimeLineConfig::LockPoint>::Clone2(oh, in.mainLockPoint, out.mainLockPoint);
        CloneFuncs<std::vector<TimeLineConfig::LockPoint>>::Clone2(oh, in.lockPoints, out.lockPoints);
    }
	void DataFuncsEx<TimeLineConfig::LockPoints, void>::Write(DataWriterEx& dw, TimeLineConfig::LockPoints const& in) {
        dw.Write(in.mainLockPoint);
        dw.Write(in.lockPoints);
    }
	int DataFuncsEx<TimeLineConfig::LockPoints, void>::Read(DataReaderEx& d, TimeLineConfig::LockPoints& out) {
        if (int r = d.Read(out.mainLockPoint)) return r;
        if (int r = d.Read(out.lockPoints)) return r;
        return 0;
    }
	void StringFuncsEx<TimeLineConfig::LockPoints, void>::Append(ObjectHelper &oh, TimeLineConfig::LockPoints const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<TimeLineConfig::LockPoints, void>::AppendCore(ObjectHelper &oh, TimeLineConfig::LockPoints const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"mainLockPoint\":", in.mainLockPoint); 
        xx::AppendEx(oh, ",\"lockPoints\":", in.lockPoints);
    }
    void CloneFuncs<TimeLineConfig::TimePoint>::Clone1(xx::ObjectHelper &oh, TimeLineConfig::TimePoint const& in, TimeLineConfig::TimePoint &out) {
        CloneFuncs<float>::Clone1(oh, in.time, out.time);
        CloneFuncs<std::optional<std::string>>::Clone1(oh, in.pic, out.pic);
        CloneFuncs<std::optional<TimeLineConfig::LockPoints>>::Clone1(oh, in.lps, out.lps);
        CloneFuncs<std::optional<TimeLineConfig::CDCircles>>::Clone1(oh, in.cdcs, out.cdcs);
        CloneFuncs<std::optional<float>>::Clone1(oh, in.speed, out.speed);
    }
    void CloneFuncs<TimeLineConfig::TimePoint>::Clone2(xx::ObjectHelper &oh, TimeLineConfig::TimePoint const& in, TimeLineConfig::TimePoint &out) {
        CloneFuncs<float>::Clone2(oh, in.time, out.time);
        CloneFuncs<std::optional<std::string>>::Clone2(oh, in.pic, out.pic);
        CloneFuncs<std::optional<TimeLineConfig::LockPoints>>::Clone2(oh, in.lps, out.lps);
        CloneFuncs<std::optional<TimeLineConfig::CDCircles>>::Clone2(oh, in.cdcs, out.cdcs);
        CloneFuncs<std::optional<float>>::Clone2(oh, in.speed, out.speed);
    }
	void DataFuncsEx<TimeLineConfig::TimePoint, void>::Write(DataWriterEx& dw, TimeLineConfig::TimePoint const& in) {
        dw.Write(in.time);
        dw.Write(in.pic);
        dw.Write(in.lps);
        dw.Write(in.cdcs);
        dw.Write(in.speed);
    }
	int DataFuncsEx<TimeLineConfig::TimePoint, void>::Read(DataReaderEx& d, TimeLineConfig::TimePoint& out) {
        if (int r = d.Read(out.time)) return r;
        if (int r = d.Read(out.pic)) return r;
        if (int r = d.Read(out.lps)) return r;
        if (int r = d.Read(out.cdcs)) return r;
        if (int r = d.Read(out.speed)) return r;
        return 0;
    }
	void StringFuncsEx<TimeLineConfig::TimePoint, void>::Append(ObjectHelper &oh, TimeLineConfig::TimePoint const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<TimeLineConfig::TimePoint, void>::AppendCore(ObjectHelper &oh, TimeLineConfig::TimePoint const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"time\":", in.time); 
        xx::AppendEx(oh, ",\"pic\":", in.pic);
        xx::AppendEx(oh, ",\"lps\":", in.lps);
        xx::AppendEx(oh, ",\"cdcs\":", in.cdcs);
        xx::AppendEx(oh, ",\"speed\":", in.speed);
    }
    void CloneFuncs<TimeLineConfig::TimeLine>::Clone1(xx::ObjectHelper &oh, TimeLineConfig::TimeLine const& in, TimeLineConfig::TimeLine &out) {
        CloneFuncs<float>::Clone1(oh, in.totalSeconds, out.totalSeconds);
        CloneFuncs<std::vector<TimeLineConfig::TimePoint>>::Clone1(oh, in.timePoints, out.timePoints);
    }
    void CloneFuncs<TimeLineConfig::TimeLine>::Clone2(xx::ObjectHelper &oh, TimeLineConfig::TimeLine const& in, TimeLineConfig::TimeLine &out) {
        CloneFuncs<float>::Clone2(oh, in.totalSeconds, out.totalSeconds);
        CloneFuncs<std::vector<TimeLineConfig::TimePoint>>::Clone2(oh, in.timePoints, out.timePoints);
    }
	void DataFuncsEx<TimeLineConfig::TimeLine, void>::Write(DataWriterEx& dw, TimeLineConfig::TimeLine const& in) {
        dw.Write(in.totalSeconds);
        dw.Write(in.timePoints);
    }
	int DataFuncsEx<TimeLineConfig::TimeLine, void>::Read(DataReaderEx& d, TimeLineConfig::TimeLine& out) {
        if (int r = d.Read(out.totalSeconds)) return r;
        if (int r = d.Read(out.timePoints)) return r;
        return 0;
    }
	void StringFuncsEx<TimeLineConfig::TimeLine, void>::Append(ObjectHelper &oh, TimeLineConfig::TimeLine const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<TimeLineConfig::TimeLine, void>::AppendCore(ObjectHelper &oh, TimeLineConfig::TimeLine const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"totalSeconds\":", in.totalSeconds); 
        xx::AppendEx(oh, ",\"timePoints\":", in.timePoints);
    }
}
namespace TimeLineConfig {
    CDCircle::CDCircle(CDCircle&& o) noexcept {
        this->operator=(std::move(o));
    }
    CDCircle& CDCircle::operator=(CDCircle&& o) noexcept {
        std::swap(this->x, o.x);
        std::swap(this->y, o.y);
        std::swap(this->r, o.r);
        return *this;
    }
    LockPoint::LockPoint(LockPoint&& o) noexcept {
        this->operator=(std::move(o));
    }
    LockPoint& LockPoint::operator=(LockPoint&& o) noexcept {
        std::swap(this->x, o.x);
        std::swap(this->y, o.y);
        return *this;
    }
    CDCircles::CDCircles(CDCircles&& o) noexcept {
        this->operator=(std::move(o));
    }
    CDCircles& CDCircles::operator=(CDCircles&& o) noexcept {
        std::swap(this->maxCDCircle, o.maxCDCircle);
        std::swap(this->cdCircles, o.cdCircles);
        return *this;
    }
    LockPoints::LockPoints(LockPoints&& o) noexcept {
        this->operator=(std::move(o));
    }
    LockPoints& LockPoints::operator=(LockPoints&& o) noexcept {
        std::swap(this->mainLockPoint, o.mainLockPoint);
        std::swap(this->lockPoints, o.lockPoints);
        return *this;
    }
    TimePoint::TimePoint(TimePoint&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimePoint& TimePoint::operator=(TimePoint&& o) noexcept {
        std::swap(this->time, o.time);
        std::swap(this->pic, o.pic);
        std::swap(this->lps, o.lps);
        std::swap(this->cdcs, o.cdcs);
        std::swap(this->speed, o.speed);
        return *this;
    }
    TimeLine::TimeLine(TimeLine&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimeLine& TimeLine::operator=(TimeLine&& o) noexcept {
        std::swap(this->totalSeconds, o.totalSeconds);
        std::swap(this->timePoints, o.timePoints);
        return *this;
    }
}
