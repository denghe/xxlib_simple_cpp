﻿#include "FF_class_lite.h"
#include "FF_class_lite.cpp.inc"
namespace FF {
	void PkgGenTypes::RegisterTo(::xx::ObjManager& om) {
	    om.Register<::FF::Foo>();
	    om.Register<::FF::Foo2>();
	    om.Register<::FF::Pathway>();
	    om.Register<::FF::Fish>();
	    om.Register<::FF::Player>();
	    om.Register<::FF::Cannon>();
	    om.Register<::FF::Bullet>();
	    om.Register<::FF::Stuff>();
	    om.Register<::FF::Root>();
	    om.Register<::FF::SimpleBullet>();
	    om.Register<::FF::TrackBullet>();
	}
}

namespace xx {
	void ObjFuncs<::FF::Point, void>::Write(::xx::ObjManager& om, ::FF::Point const& in) {
        om.Write(in.x);
        om.Write(in.y);
    }
	int ObjFuncs<::FF::Point, void>::Read(::xx::ObjManager& om, ::FF::Point& out) {
        if (int r = om.Read(out.x)) return r;
        if (int r = om.Read(out.y)) return r;
        return 0;
    }
	void ObjFuncs<::FF::Point, void>::ToString(ObjManager &om, ::FF::Point const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::Point, void>::ToStringCore(ObjManager &om, ::FF::Point const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"x\":", in.x); 
        om.Append(",\"y\":", in.y);
    }
    void ObjFuncs<::FF::Point>::Clone1(::xx::ObjManager& om, ::FF::Point const& in, ::FF::Point &out) {
        om.Clone1(in.x, out.x);
        om.Clone1(in.y, out.y);
    }
    void ObjFuncs<::FF::Point>::Clone2(::xx::ObjManager& om, ::FF::Point const& in, ::FF::Point &out) {
        om.Clone2(in.x, out.x);
        om.Clone2(in.y, out.y);
    }
    void ObjFuncs<::FF::Point>::RecursiveReset(::xx::ObjManager& om, ::FF::Point& in) {
        om.RecursiveReset(in.x);
        om.RecursiveReset(in.y);
    }
	void ObjFuncs<::FF::LockPoint, void>::Write(::xx::ObjManager& om, ::FF::LockPoint const& in) {
        om.Write(in.x);
        om.Write(in.y);
    }
	int ObjFuncs<::FF::LockPoint, void>::Read(::xx::ObjManager& om, ::FF::LockPoint& out) {
        if (int r = om.Read(out.x)) return r;
        if (int r = om.Read(out.y)) return r;
        return 0;
    }
	void ObjFuncs<::FF::LockPoint, void>::ToString(ObjManager &om, ::FF::LockPoint const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::LockPoint, void>::ToStringCore(ObjManager &om, ::FF::LockPoint const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"x\":", in.x); 
        om.Append(",\"y\":", in.y);
    }
    void ObjFuncs<::FF::LockPoint>::Clone1(::xx::ObjManager& om, ::FF::LockPoint const& in, ::FF::LockPoint &out) {
        om.Clone1(in.x, out.x);
        om.Clone1(in.y, out.y);
    }
    void ObjFuncs<::FF::LockPoint>::Clone2(::xx::ObjManager& om, ::FF::LockPoint const& in, ::FF::LockPoint &out) {
        om.Clone2(in.x, out.x);
        om.Clone2(in.y, out.y);
    }
    void ObjFuncs<::FF::LockPoint>::RecursiveReset(::xx::ObjManager& om, ::FF::LockPoint& in) {
        om.RecursiveReset(in.x);
        om.RecursiveReset(in.y);
    }
	void ObjFuncs<::FF::CDCircle, void>::Write(::xx::ObjManager& om, ::FF::CDCircle const& in) {
        om.Write(in.x);
        om.Write(in.y);
        om.Write(in.r);
    }
	int ObjFuncs<::FF::CDCircle, void>::Read(::xx::ObjManager& om, ::FF::CDCircle& out) {
        if (int r = om.Read(out.x)) return r;
        if (int r = om.Read(out.y)) return r;
        if (int r = om.Read(out.r)) return r;
        return 0;
    }
	void ObjFuncs<::FF::CDCircle, void>::ToString(ObjManager &om, ::FF::CDCircle const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::CDCircle, void>::ToStringCore(ObjManager &om, ::FF::CDCircle const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"x\":", in.x); 
        om.Append(",\"y\":", in.y);
        om.Append(",\"r\":", in.r);
    }
    void ObjFuncs<::FF::CDCircle>::Clone1(::xx::ObjManager& om, ::FF::CDCircle const& in, ::FF::CDCircle &out) {
        om.Clone1(in.x, out.x);
        om.Clone1(in.y, out.y);
        om.Clone1(in.r, out.r);
    }
    void ObjFuncs<::FF::CDCircle>::Clone2(::xx::ObjManager& om, ::FF::CDCircle const& in, ::FF::CDCircle &out) {
        om.Clone2(in.x, out.x);
        om.Clone2(in.y, out.y);
        om.Clone2(in.r, out.r);
    }
    void ObjFuncs<::FF::CDCircle>::RecursiveReset(::xx::ObjManager& om, ::FF::CDCircle& in) {
        om.RecursiveReset(in.x);
        om.RecursiveReset(in.y);
        om.RecursiveReset(in.r);
    }
	void ObjFuncs<::FF::TimePoint_CDCircles, void>::Write(::xx::ObjManager& om, ::FF::TimePoint_CDCircles const& in) {
        om.Write(in.time);
        om.Write(in.maxCDCircle);
        om.Write(in.cdCircles);
    }
	int ObjFuncs<::FF::TimePoint_CDCircles, void>::Read(::xx::ObjManager& om, ::FF::TimePoint_CDCircles& out) {
        if (int r = om.Read(out.time)) return r;
        if (int r = om.Read(out.maxCDCircle)) return r;
        if (int r = om.Read(out.cdCircles)) return r;
        return 0;
    }
	void ObjFuncs<::FF::TimePoint_CDCircles, void>::ToString(ObjManager &om, ::FF::TimePoint_CDCircles const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::TimePoint_CDCircles, void>::ToStringCore(ObjManager &om, ::FF::TimePoint_CDCircles const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"time\":", in.time); 
        om.Append(",\"maxCDCircle\":", in.maxCDCircle);
        om.Append(",\"cdCircles\":", in.cdCircles);
    }
    void ObjFuncs<::FF::TimePoint_CDCircles>::Clone1(::xx::ObjManager& om, ::FF::TimePoint_CDCircles const& in, ::FF::TimePoint_CDCircles &out) {
        om.Clone1(in.time, out.time);
        om.Clone1(in.maxCDCircle, out.maxCDCircle);
        om.Clone1(in.cdCircles, out.cdCircles);
    }
    void ObjFuncs<::FF::TimePoint_CDCircles>::Clone2(::xx::ObjManager& om, ::FF::TimePoint_CDCircles const& in, ::FF::TimePoint_CDCircles &out) {
        om.Clone2(in.time, out.time);
        om.Clone2(in.maxCDCircle, out.maxCDCircle);
        om.Clone2(in.cdCircles, out.cdCircles);
    }
    void ObjFuncs<::FF::TimePoint_CDCircles>::RecursiveReset(::xx::ObjManager& om, ::FF::TimePoint_CDCircles& in) {
        om.RecursiveReset(in.time);
        om.RecursiveReset(in.maxCDCircle);
        om.RecursiveReset(in.cdCircles);
    }
	void ObjFuncs<::FF::TimePoint_LockPoints, void>::Write(::xx::ObjManager& om, ::FF::TimePoint_LockPoints const& in) {
        om.Write(in.time);
        om.Write(in.mainLockPoint);
        om.Write(in.lockPoints);
    }
	int ObjFuncs<::FF::TimePoint_LockPoints, void>::Read(::xx::ObjManager& om, ::FF::TimePoint_LockPoints& out) {
        if (int r = om.Read(out.time)) return r;
        if (int r = om.Read(out.mainLockPoint)) return r;
        if (int r = om.Read(out.lockPoints)) return r;
        return 0;
    }
	void ObjFuncs<::FF::TimePoint_LockPoints, void>::ToString(ObjManager &om, ::FF::TimePoint_LockPoints const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::TimePoint_LockPoints, void>::ToStringCore(ObjManager &om, ::FF::TimePoint_LockPoints const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"time\":", in.time); 
        om.Append(",\"mainLockPoint\":", in.mainLockPoint);
        om.Append(",\"lockPoints\":", in.lockPoints);
    }
    void ObjFuncs<::FF::TimePoint_LockPoints>::Clone1(::xx::ObjManager& om, ::FF::TimePoint_LockPoints const& in, ::FF::TimePoint_LockPoints &out) {
        om.Clone1(in.time, out.time);
        om.Clone1(in.mainLockPoint, out.mainLockPoint);
        om.Clone1(in.lockPoints, out.lockPoints);
    }
    void ObjFuncs<::FF::TimePoint_LockPoints>::Clone2(::xx::ObjManager& om, ::FF::TimePoint_LockPoints const& in, ::FF::TimePoint_LockPoints &out) {
        om.Clone2(in.time, out.time);
        om.Clone2(in.mainLockPoint, out.mainLockPoint);
        om.Clone2(in.lockPoints, out.lockPoints);
    }
    void ObjFuncs<::FF::TimePoint_LockPoints>::RecursiveReset(::xx::ObjManager& om, ::FF::TimePoint_LockPoints& in) {
        om.RecursiveReset(in.time);
        om.RecursiveReset(in.mainLockPoint);
        om.RecursiveReset(in.lockPoints);
    }
	void ObjFuncs<::FF::TimePoint_Speed, void>::Write(::xx::ObjManager& om, ::FF::TimePoint_Speed const& in) {
        om.Write(in.time);
        om.Write(in.speed);
    }
	int ObjFuncs<::FF::TimePoint_Speed, void>::Read(::xx::ObjManager& om, ::FF::TimePoint_Speed& out) {
        if (int r = om.Read(out.time)) return r;
        if (int r = om.Read(out.speed)) return r;
        return 0;
    }
	void ObjFuncs<::FF::TimePoint_Speed, void>::ToString(ObjManager &om, ::FF::TimePoint_Speed const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::TimePoint_Speed, void>::ToStringCore(ObjManager &om, ::FF::TimePoint_Speed const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"time\":", in.time); 
        om.Append(",\"speed\":", in.speed);
    }
    void ObjFuncs<::FF::TimePoint_Speed>::Clone1(::xx::ObjManager& om, ::FF::TimePoint_Speed const& in, ::FF::TimePoint_Speed &out) {
        om.Clone1(in.time, out.time);
        om.Clone1(in.speed, out.speed);
    }
    void ObjFuncs<::FF::TimePoint_Speed>::Clone2(::xx::ObjManager& om, ::FF::TimePoint_Speed const& in, ::FF::TimePoint_Speed &out) {
        om.Clone2(in.time, out.time);
        om.Clone2(in.speed, out.speed);
    }
    void ObjFuncs<::FF::TimePoint_Speed>::RecursiveReset(::xx::ObjManager& om, ::FF::TimePoint_Speed& in) {
        om.RecursiveReset(in.time);
        om.RecursiveReset(in.speed);
    }
	void ObjFuncs<::FF::PathwayPoint, void>::Write(::xx::ObjManager& om, ::FF::PathwayPoint const& in) {
        om.Write(in.pos);
        om.Write(in.a);
        om.Write(in.d);
    }
	int ObjFuncs<::FF::PathwayPoint, void>::Read(::xx::ObjManager& om, ::FF::PathwayPoint& out) {
        if (int r = om.Read(out.pos)) return r;
        if (int r = om.Read(out.a)) return r;
        if (int r = om.Read(out.d)) return r;
        return 0;
    }
	void ObjFuncs<::FF::PathwayPoint, void>::ToString(ObjManager &om, ::FF::PathwayPoint const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::PathwayPoint, void>::ToStringCore(ObjManager &om, ::FF::PathwayPoint const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"pos\":", in.pos); 
        om.Append(",\"a\":", in.a);
        om.Append(",\"d\":", in.d);
    }
    void ObjFuncs<::FF::PathwayPoint>::Clone1(::xx::ObjManager& om, ::FF::PathwayPoint const& in, ::FF::PathwayPoint &out) {
        om.Clone1(in.pos, out.pos);
        om.Clone1(in.a, out.a);
        om.Clone1(in.d, out.d);
    }
    void ObjFuncs<::FF::PathwayPoint>::Clone2(::xx::ObjManager& om, ::FF::PathwayPoint const& in, ::FF::PathwayPoint &out) {
        om.Clone2(in.pos, out.pos);
        om.Clone2(in.a, out.a);
        om.Clone2(in.d, out.d);
    }
    void ObjFuncs<::FF::PathwayPoint>::RecursiveReset(::xx::ObjManager& om, ::FF::PathwayPoint& in) {
        om.RecursiveReset(in.pos);
        om.RecursiveReset(in.a);
        om.RecursiveReset(in.d);
    }
	void ObjFuncs<::FF::Action_AnimExt, void>::Write(::xx::ObjManager& om, ::FF::Action_AnimExt const& in) {
        om.Write(in.name);
        om.Write(in.totalSeconds);
        om.Write(in.lps);
        om.Write(in.cds);
        om.Write(in.ss);
    }
	int ObjFuncs<::FF::Action_AnimExt, void>::Read(::xx::ObjManager& om, ::FF::Action_AnimExt& out) {
        if (int r = om.Read(out.name)) return r;
        if (int r = om.Read(out.totalSeconds)) return r;
        if (int r = om.Read(out.lps)) return r;
        if (int r = om.Read(out.cds)) return r;
        if (int r = om.Read(out.ss)) return r;
        return 0;
    }
	void ObjFuncs<::FF::Action_AnimExt, void>::ToString(ObjManager &om, ::FF::Action_AnimExt const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::Action_AnimExt, void>::ToStringCore(ObjManager &om, ::FF::Action_AnimExt const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"name\":", in.name); 
        om.Append(",\"totalSeconds\":", in.totalSeconds);
        om.Append(",\"lps\":", in.lps);
        om.Append(",\"cds\":", in.cds);
        om.Append(",\"ss\":", in.ss);
    }
    void ObjFuncs<::FF::Action_AnimExt>::Clone1(::xx::ObjManager& om, ::FF::Action_AnimExt const& in, ::FF::Action_AnimExt &out) {
        om.Clone1(in.name, out.name);
        om.Clone1(in.totalSeconds, out.totalSeconds);
        om.Clone1(in.lps, out.lps);
        om.Clone1(in.cds, out.cds);
        om.Clone1(in.ss, out.ss);
    }
    void ObjFuncs<::FF::Action_AnimExt>::Clone2(::xx::ObjManager& om, ::FF::Action_AnimExt const& in, ::FF::Action_AnimExt &out) {
        om.Clone2(in.name, out.name);
        om.Clone2(in.totalSeconds, out.totalSeconds);
        om.Clone2(in.lps, out.lps);
        om.Clone2(in.cds, out.cds);
        om.Clone2(in.ss, out.ss);
    }
    void ObjFuncs<::FF::Action_AnimExt>::RecursiveReset(::xx::ObjManager& om, ::FF::Action_AnimExt& in) {
        om.RecursiveReset(in.name);
        om.RecursiveReset(in.totalSeconds);
        om.RecursiveReset(in.lps);
        om.RecursiveReset(in.cds);
        om.RecursiveReset(in.ss);
    }
	void ObjFuncs<::FF::File_AnimExt, void>::Write(::xx::ObjManager& om, ::FF::File_AnimExt const& in) {
        om.Write(in.actions);
        om.Write(in.shadowX);
        om.Write(in.shadowY);
        om.Write(in.shadowScale);
    }
	int ObjFuncs<::FF::File_AnimExt, void>::Read(::xx::ObjManager& om, ::FF::File_AnimExt& out) {
        if (int r = om.Read(out.actions)) return r;
        if (int r = om.Read(out.shadowX)) return r;
        if (int r = om.Read(out.shadowY)) return r;
        if (int r = om.Read(out.shadowScale)) return r;
        return 0;
    }
	void ObjFuncs<::FF::File_AnimExt, void>::ToString(ObjManager &om, ::FF::File_AnimExt const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::File_AnimExt, void>::ToStringCore(ObjManager &om, ::FF::File_AnimExt const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"actions\":", in.actions); 
        om.Append(",\"shadowX\":", in.shadowX);
        om.Append(",\"shadowY\":", in.shadowY);
        om.Append(",\"shadowScale\":", in.shadowScale);
    }
    void ObjFuncs<::FF::File_AnimExt>::Clone1(::xx::ObjManager& om, ::FF::File_AnimExt const& in, ::FF::File_AnimExt &out) {
        om.Clone1(in.actions, out.actions);
        om.Clone1(in.shadowX, out.shadowX);
        om.Clone1(in.shadowY, out.shadowY);
        om.Clone1(in.shadowScale, out.shadowScale);
    }
    void ObjFuncs<::FF::File_AnimExt>::Clone2(::xx::ObjManager& om, ::FF::File_AnimExt const& in, ::FF::File_AnimExt &out) {
        om.Clone2(in.actions, out.actions);
        om.Clone2(in.shadowX, out.shadowX);
        om.Clone2(in.shadowY, out.shadowY);
        om.Clone2(in.shadowScale, out.shadowScale);
    }
    void ObjFuncs<::FF::File_AnimExt>::RecursiveReset(::xx::ObjManager& om, ::FF::File_AnimExt& in) {
        om.RecursiveReset(in.actions);
        om.RecursiveReset(in.shadowX);
        om.RecursiveReset(in.shadowY);
        om.RecursiveReset(in.shadowScale);
    }
	void ObjFuncs<::FF::TimePoint_Frame, void>::Write(::xx::ObjManager& om, ::FF::TimePoint_Frame const& in) {
        om.Write(in.time);
        om.Write(in.picName);
    }
	int ObjFuncs<::FF::TimePoint_Frame, void>::Read(::xx::ObjManager& om, ::FF::TimePoint_Frame& out) {
        if (int r = om.Read(out.time)) return r;
        if (int r = om.Read(out.picName)) return r;
        return 0;
    }
	void ObjFuncs<::FF::TimePoint_Frame, void>::ToString(ObjManager &om, ::FF::TimePoint_Frame const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::TimePoint_Frame, void>::ToStringCore(ObjManager &om, ::FF::TimePoint_Frame const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"time\":", in.time); 
        om.Append(",\"picName\":", in.picName);
    }
    void ObjFuncs<::FF::TimePoint_Frame>::Clone1(::xx::ObjManager& om, ::FF::TimePoint_Frame const& in, ::FF::TimePoint_Frame &out) {
        om.Clone1(in.time, out.time);
        om.Clone1(in.picName, out.picName);
    }
    void ObjFuncs<::FF::TimePoint_Frame>::Clone2(::xx::ObjManager& om, ::FF::TimePoint_Frame const& in, ::FF::TimePoint_Frame &out) {
        om.Clone2(in.time, out.time);
        om.Clone2(in.picName, out.picName);
    }
    void ObjFuncs<::FF::TimePoint_Frame>::RecursiveReset(::xx::ObjManager& om, ::FF::TimePoint_Frame& in) {
        om.RecursiveReset(in.time);
        om.RecursiveReset(in.picName);
    }
	void ObjFuncs<::FF::CurvePoint, void>::Write(::xx::ObjManager& om, ::FF::CurvePoint const& in) {
        om.Write(in.x);
        om.Write(in.y);
        om.Write(in.tension);
        om.Write(in.numSegments);
    }
	int ObjFuncs<::FF::CurvePoint, void>::Read(::xx::ObjManager& om, ::FF::CurvePoint& out) {
        if (int r = om.Read(out.x)) return r;
        if (int r = om.Read(out.y)) return r;
        if (int r = om.Read(out.tension)) return r;
        if (int r = om.Read(out.numSegments)) return r;
        return 0;
    }
	void ObjFuncs<::FF::CurvePoint, void>::ToString(ObjManager &om, ::FF::CurvePoint const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::CurvePoint, void>::ToStringCore(ObjManager &om, ::FF::CurvePoint const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"x\":", in.x); 
        om.Append(",\"y\":", in.y);
        om.Append(",\"tension\":", in.tension);
        om.Append(",\"numSegments\":", in.numSegments);
    }
    void ObjFuncs<::FF::CurvePoint>::Clone1(::xx::ObjManager& om, ::FF::CurvePoint const& in, ::FF::CurvePoint &out) {
        om.Clone1(in.x, out.x);
        om.Clone1(in.y, out.y);
        om.Clone1(in.tension, out.tension);
        om.Clone1(in.numSegments, out.numSegments);
    }
    void ObjFuncs<::FF::CurvePoint>::Clone2(::xx::ObjManager& om, ::FF::CurvePoint const& in, ::FF::CurvePoint &out) {
        om.Clone2(in.x, out.x);
        om.Clone2(in.y, out.y);
        om.Clone2(in.tension, out.tension);
        om.Clone2(in.numSegments, out.numSegments);
    }
    void ObjFuncs<::FF::CurvePoint>::RecursiveReset(::xx::ObjManager& om, ::FF::CurvePoint& in) {
        om.RecursiveReset(in.x);
        om.RecursiveReset(in.y);
        om.RecursiveReset(in.tension);
        om.RecursiveReset(in.numSegments);
    }
	void ObjFuncs<::FF::Action_Frames, void>::Write(::xx::ObjManager& om, ::FF::Action_Frames const& in) {
        om.Write(in.name);
        om.Write(in.totalSeconds);
        om.Write(in.frames);
    }
	int ObjFuncs<::FF::Action_Frames, void>::Read(::xx::ObjManager& om, ::FF::Action_Frames& out) {
        if (int r = om.Read(out.name)) return r;
        if (int r = om.Read(out.totalSeconds)) return r;
        if (int r = om.Read(out.frames)) return r;
        return 0;
    }
	void ObjFuncs<::FF::Action_Frames, void>::ToString(ObjManager &om, ::FF::Action_Frames const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::Action_Frames, void>::ToStringCore(ObjManager &om, ::FF::Action_Frames const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"name\":", in.name); 
        om.Append(",\"totalSeconds\":", in.totalSeconds);
        om.Append(",\"frames\":", in.frames);
    }
    void ObjFuncs<::FF::Action_Frames>::Clone1(::xx::ObjManager& om, ::FF::Action_Frames const& in, ::FF::Action_Frames &out) {
        om.Clone1(in.name, out.name);
        om.Clone1(in.totalSeconds, out.totalSeconds);
        om.Clone1(in.frames, out.frames);
    }
    void ObjFuncs<::FF::Action_Frames>::Clone2(::xx::ObjManager& om, ::FF::Action_Frames const& in, ::FF::Action_Frames &out) {
        om.Clone2(in.name, out.name);
        om.Clone2(in.totalSeconds, out.totalSeconds);
        om.Clone2(in.frames, out.frames);
    }
    void ObjFuncs<::FF::Action_Frames>::RecursiveReset(::xx::ObjManager& om, ::FF::Action_Frames& in) {
        om.RecursiveReset(in.name);
        om.RecursiveReset(in.totalSeconds);
        om.RecursiveReset(in.frames);
    }
	void ObjFuncs<::FF::File_Frames, void>::Write(::xx::ObjManager& om, ::FF::File_Frames const& in) {
        om.Write(in.actions);
        om.Write(in.plists);
    }
	int ObjFuncs<::FF::File_Frames, void>::Read(::xx::ObjManager& om, ::FF::File_Frames& out) {
        if (int r = om.Read(out.actions)) return r;
        if (int r = om.Read(out.plists)) return r;
        return 0;
    }
	void ObjFuncs<::FF::File_Frames, void>::ToString(ObjManager &om, ::FF::File_Frames const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::File_Frames, void>::ToStringCore(ObjManager &om, ::FF::File_Frames const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"actions\":", in.actions); 
        om.Append(",\"plists\":", in.plists);
    }
    void ObjFuncs<::FF::File_Frames>::Clone1(::xx::ObjManager& om, ::FF::File_Frames const& in, ::FF::File_Frames &out) {
        om.Clone1(in.actions, out.actions);
        om.Clone1(in.plists, out.plists);
    }
    void ObjFuncs<::FF::File_Frames>::Clone2(::xx::ObjManager& om, ::FF::File_Frames const& in, ::FF::File_Frames &out) {
        om.Clone2(in.actions, out.actions);
        om.Clone2(in.plists, out.plists);
    }
    void ObjFuncs<::FF::File_Frames>::RecursiveReset(::xx::ObjManager& om, ::FF::File_Frames& in) {
        om.RecursiveReset(in.actions);
        om.RecursiveReset(in.plists);
    }
	void ObjFuncs<::FF::File_pathway, void>::Write(::xx::ObjManager& om, ::FF::File_pathway const& in) {
        om.Write(in.isLoop);
        om.Write(in.points);
    }
	int ObjFuncs<::FF::File_pathway, void>::Read(::xx::ObjManager& om, ::FF::File_pathway& out) {
        if (int r = om.Read(out.isLoop)) return r;
        if (int r = om.Read(out.points)) return r;
        return 0;
    }
	void ObjFuncs<::FF::File_pathway, void>::ToString(ObjManager &om, ::FF::File_pathway const& in) {
        om.str->push_back('{');
        ToStringCore(om, in);
        om.str->push_back('}');
    }
	void ObjFuncs<::FF::File_pathway, void>::ToStringCore(ObjManager &om, ::FF::File_pathway const& in) {
        auto sizeBak = om.str->size();
        om.Append("\"isLoop\":", in.isLoop); 
        om.Append(",\"points\":", in.points);
    }
    void ObjFuncs<::FF::File_pathway>::Clone1(::xx::ObjManager& om, ::FF::File_pathway const& in, ::FF::File_pathway &out) {
        om.Clone1(in.isLoop, out.isLoop);
        om.Clone1(in.points, out.points);
    }
    void ObjFuncs<::FF::File_pathway>::Clone2(::xx::ObjManager& om, ::FF::File_pathway const& in, ::FF::File_pathway &out) {
        om.Clone2(in.isLoop, out.isLoop);
        om.Clone2(in.points, out.points);
    }
    void ObjFuncs<::FF::File_pathway>::RecursiveReset(::xx::ObjManager& om, ::FF::File_pathway& in) {
        om.RecursiveReset(in.isLoop);
        om.RecursiveReset(in.points);
    }
}
namespace FF {
    Point::Point(Point&& o) noexcept {
        this->operator=(std::move(o));
    }
    Point& Point::operator=(Point&& o) noexcept {
        std::swap(this->x, o.x);
        std::swap(this->y, o.y);
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
    CDCircle::CDCircle(CDCircle&& o) noexcept {
        this->operator=(std::move(o));
    }
    CDCircle& CDCircle::operator=(CDCircle&& o) noexcept {
        std::swap(this->x, o.x);
        std::swap(this->y, o.y);
        std::swap(this->r, o.r);
        return *this;
    }
    TimePoint_CDCircles::TimePoint_CDCircles(TimePoint_CDCircles&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimePoint_CDCircles& TimePoint_CDCircles::operator=(TimePoint_CDCircles&& o) noexcept {
        std::swap(this->time, o.time);
        std::swap(this->maxCDCircle, o.maxCDCircle);
        std::swap(this->cdCircles, o.cdCircles);
        return *this;
    }
    Bullet::Bullet(Bullet&& o) noexcept {
        this->operator=(std::move(o));
    }
    Bullet& Bullet::operator=(Bullet&& o) noexcept {
        std::swap(this->id, o.id);
        std::swap(this->coin, o.coin);
        return *this;
    }
    void Bullet::Write(::xx::ObjManager& om) const {
        om.Write(this->id);
        om.Write(this->coin);
    }
    int Bullet::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->id)) return r;
        if (int r = om.Read(this->coin)) return r;
        return 0;
    }
    void Bullet::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Bullet::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"id\":", this->id);
        om.Append(",\"coin\":", this->coin);
    }
    void Bullet::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Bullet*)tar;
        om.Clone1(this->id, out->id);
        om.Clone1(this->coin, out->coin);
    }
    void Bullet::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Bullet*)tar;
        om.Clone2(this->id, out->id);
        om.Clone2(this->coin, out->coin);
    }
    void Bullet::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->id);
        om.RecursiveReset(this->coin);
    }
    TimePoint_LockPoints::TimePoint_LockPoints(TimePoint_LockPoints&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimePoint_LockPoints& TimePoint_LockPoints::operator=(TimePoint_LockPoints&& o) noexcept {
        std::swap(this->time, o.time);
        std::swap(this->mainLockPoint, o.mainLockPoint);
        std::swap(this->lockPoints, o.lockPoints);
        return *this;
    }
    TimePoint_Speed::TimePoint_Speed(TimePoint_Speed&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimePoint_Speed& TimePoint_Speed::operator=(TimePoint_Speed&& o) noexcept {
        std::swap(this->time, o.time);
        std::swap(this->speed, o.speed);
        return *this;
    }
    PathwayPoint::PathwayPoint(PathwayPoint&& o) noexcept {
        this->operator=(std::move(o));
    }
    PathwayPoint& PathwayPoint::operator=(PathwayPoint&& o) noexcept {
        std::swap(this->pos, o.pos);
        std::swap(this->a, o.a);
        std::swap(this->d, o.d);
        return *this;
    }
    Action_AnimExt::Action_AnimExt(Action_AnimExt&& o) noexcept {
        this->operator=(std::move(o));
    }
    Action_AnimExt& Action_AnimExt::operator=(Action_AnimExt&& o) noexcept {
        std::swap(this->name, o.name);
        std::swap(this->totalSeconds, o.totalSeconds);
        std::swap(this->lps, o.lps);
        std::swap(this->cds, o.cds);
        std::swap(this->ss, o.ss);
        return *this;
    }
    Pathway::Pathway(Pathway&& o) noexcept {
        this->operator=(std::move(o));
    }
    Pathway& Pathway::operator=(Pathway&& o) noexcept {
        std::swap(this->isLoop, o.isLoop);
        std::swap(this->points, o.points);
        return *this;
    }
    void Pathway::Write(::xx::ObjManager& om) const {
        om.Write(this->isLoop);
        om.Write(this->points);
    }
    int Pathway::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->isLoop)) return r;
        if (int r = om.Read(this->points)) return r;
        return 0;
    }
    void Pathway::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Pathway::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"isLoop\":", this->isLoop);
        om.Append(",\"points\":", this->points);
    }
    void Pathway::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Pathway*)tar;
        om.Clone1(this->isLoop, out->isLoop);
        om.Clone1(this->points, out->points);
    }
    void Pathway::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Pathway*)tar;
        om.Clone2(this->isLoop, out->isLoop);
        om.Clone2(this->points, out->points);
    }
    void Pathway::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->isLoop);
        om.RecursiveReset(this->points);
    }
    File_AnimExt::File_AnimExt(File_AnimExt&& o) noexcept {
        this->operator=(std::move(o));
    }
    File_AnimExt& File_AnimExt::operator=(File_AnimExt&& o) noexcept {
        std::swap(this->actions, o.actions);
        std::swap(this->shadowX, o.shadowX);
        std::swap(this->shadowY, o.shadowY);
        std::swap(this->shadowScale, o.shadowScale);
        return *this;
    }
    Fish::Fish(Fish&& o) noexcept {
        this->operator=(std::move(o));
    }
    Fish& Fish::operator=(Fish&& o) noexcept {
        std::swap(this->pos, o.pos);
        std::swap(this->angle, o.angle);
        std::swap(this->scaleX, o.scaleX);
        std::swap(this->scaleY, o.scaleY);
        std::swap(this->animElapsedSeconds, o.animElapsedSeconds);
        std::swap(this->totalElapsedSeconds, o.totalElapsedSeconds);
        std::swap(this->pathwayI, o.pathwayI);
        std::swap(this->pathwayD, o.pathwayD);
        std::swap(this->speedScale, o.speedScale);
        std::swap(this->timeScale, o.timeScale);
        std::swap(this->lpsCursor, o.lpsCursor);
        std::swap(this->cdsCursor, o.cdsCursor);
        std::swap(this->ssCursor, o.ssCursor);
        std::swap(this->speed, o.speed);
        std::swap(this->loop, o.loop);
        std::swap(this->id, o.id);
        std::swap(this->indexAtContainer, o.indexAtContainer);
        std::swap(this->coin, o.coin);
        std::swap(this->effectiveTime, o.effectiveTime);
        std::swap(this->actionName, o.actionName);
        std::swap(this->fileName, o.fileName);
        std::swap(this->pathwayName, o.pathwayName);
        std::swap(this->luaName, o.luaName);
        std::swap(this->luaData, o.luaData);
        std::swap(this->pathway, o.pathway);
        std::swap(this->children, o.children);
        std::swap(this->offset, o.offset);
        std::swap(this->file, o.file);
        return *this;
    }
    void Fish::Write(::xx::ObjManager& om) const {
        om.Write(this->pos);
        om.Write(this->angle);
        om.Write(this->scaleX);
        om.Write(this->scaleY);
        om.Write(this->animElapsedSeconds);
        om.Write(this->totalElapsedSeconds);
        om.Write(this->pathwayI);
        om.Write(this->pathwayD);
        om.Write(this->speedScale);
        om.Write(this->timeScale);
        om.Write(this->lpsCursor);
        om.Write(this->cdsCursor);
        om.Write(this->ssCursor);
        om.Write(this->speed);
        om.Write(this->loop);
        om.Write(this->id);
        om.Write(this->indexAtContainer);
        om.Write(this->coin);
        om.Write(this->effectiveTime);
        om.Write(this->actionName);
        om.Write(this->fileName);
        om.Write(this->pathwayName);
        om.Write(this->luaName);
        om.Write(this->luaData);
        om.Write(this->pathway);
        om.Write(this->children);
        om.Write(this->offset);
        om.Write(this->file);
    }
    int Fish::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->pos)) return r;
        if (int r = om.Read(this->angle)) return r;
        if (int r = om.Read(this->scaleX)) return r;
        if (int r = om.Read(this->scaleY)) return r;
        if (int r = om.Read(this->animElapsedSeconds)) return r;
        if (int r = om.Read(this->totalElapsedSeconds)) return r;
        if (int r = om.Read(this->pathwayI)) return r;
        if (int r = om.Read(this->pathwayD)) return r;
        if (int r = om.Read(this->speedScale)) return r;
        if (int r = om.Read(this->timeScale)) return r;
        if (int r = om.Read(this->lpsCursor)) return r;
        if (int r = om.Read(this->cdsCursor)) return r;
        if (int r = om.Read(this->ssCursor)) return r;
        if (int r = om.Read(this->speed)) return r;
        if (int r = om.Read(this->loop)) return r;
        if (int r = om.Read(this->id)) return r;
        if (int r = om.Read(this->indexAtContainer)) return r;
        if (int r = om.Read(this->coin)) return r;
        if (int r = om.Read(this->effectiveTime)) return r;
        if (int r = om.Read(this->actionName)) return r;
        if (int r = om.Read(this->fileName)) return r;
        if (int r = om.Read(this->pathwayName)) return r;
        if (int r = om.Read(this->luaName)) return r;
        if (int r = om.Read(this->luaData)) return r;
        if (int r = om.Read(this->pathway)) return r;
        if (int r = om.Read(this->children)) return r;
        if (int r = om.Read(this->offset)) return r;
        if (int r = om.Read(this->file)) return r;
        return 0;
    }
    void Fish::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Fish::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"pos\":", this->pos);
        om.Append(",\"angle\":", this->angle);
        om.Append(",\"scaleX\":", this->scaleX);
        om.Append(",\"scaleY\":", this->scaleY);
        om.Append(",\"animElapsedSeconds\":", this->animElapsedSeconds);
        om.Append(",\"totalElapsedSeconds\":", this->totalElapsedSeconds);
        om.Append(",\"pathwayI\":", this->pathwayI);
        om.Append(",\"pathwayD\":", this->pathwayD);
        om.Append(",\"speedScale\":", this->speedScale);
        om.Append(",\"timeScale\":", this->timeScale);
        om.Append(",\"lpsCursor\":", this->lpsCursor);
        om.Append(",\"cdsCursor\":", this->cdsCursor);
        om.Append(",\"ssCursor\":", this->ssCursor);
        om.Append(",\"speed\":", this->speed);
        om.Append(",\"loop\":", this->loop);
        om.Append(",\"id\":", this->id);
        om.Append(",\"indexAtContainer\":", this->indexAtContainer);
        om.Append(",\"coin\":", this->coin);
        om.Append(",\"effectiveTime\":", this->effectiveTime);
        om.Append(",\"actionName\":", this->actionName);
        om.Append(",\"fileName\":", this->fileName);
        om.Append(",\"pathwayName\":", this->pathwayName);
        om.Append(",\"luaName\":", this->luaName);
        om.Append(",\"luaData\":", this->luaData);
        om.Append(",\"pathway\":", this->pathway);
        om.Append(",\"children\":", this->children);
        om.Append(",\"offset\":", this->offset);
        om.Append(",\"file\":", this->file);
    }
    void Fish::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Fish*)tar;
        om.Clone1(this->pos, out->pos);
        om.Clone1(this->angle, out->angle);
        om.Clone1(this->scaleX, out->scaleX);
        om.Clone1(this->scaleY, out->scaleY);
        om.Clone1(this->animElapsedSeconds, out->animElapsedSeconds);
        om.Clone1(this->totalElapsedSeconds, out->totalElapsedSeconds);
        om.Clone1(this->pathwayI, out->pathwayI);
        om.Clone1(this->pathwayD, out->pathwayD);
        om.Clone1(this->speedScale, out->speedScale);
        om.Clone1(this->timeScale, out->timeScale);
        om.Clone1(this->lpsCursor, out->lpsCursor);
        om.Clone1(this->cdsCursor, out->cdsCursor);
        om.Clone1(this->ssCursor, out->ssCursor);
        om.Clone1(this->speed, out->speed);
        om.Clone1(this->loop, out->loop);
        om.Clone1(this->id, out->id);
        om.Clone1(this->indexAtContainer, out->indexAtContainer);
        om.Clone1(this->coin, out->coin);
        om.Clone1(this->effectiveTime, out->effectiveTime);
        om.Clone1(this->actionName, out->actionName);
        om.Clone1(this->fileName, out->fileName);
        om.Clone1(this->pathwayName, out->pathwayName);
        om.Clone1(this->luaName, out->luaName);
        om.Clone1(this->luaData, out->luaData);
        om.Clone1(this->pathway, out->pathway);
        om.Clone1(this->children, out->children);
        om.Clone1(this->offset, out->offset);
        om.Clone1(this->file, out->file);
    }
    void Fish::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Fish*)tar;
        om.Clone2(this->pos, out->pos);
        om.Clone2(this->angle, out->angle);
        om.Clone2(this->scaleX, out->scaleX);
        om.Clone2(this->scaleY, out->scaleY);
        om.Clone2(this->animElapsedSeconds, out->animElapsedSeconds);
        om.Clone2(this->totalElapsedSeconds, out->totalElapsedSeconds);
        om.Clone2(this->pathwayI, out->pathwayI);
        om.Clone2(this->pathwayD, out->pathwayD);
        om.Clone2(this->speedScale, out->speedScale);
        om.Clone2(this->timeScale, out->timeScale);
        om.Clone2(this->lpsCursor, out->lpsCursor);
        om.Clone2(this->cdsCursor, out->cdsCursor);
        om.Clone2(this->ssCursor, out->ssCursor);
        om.Clone2(this->speed, out->speed);
        om.Clone2(this->loop, out->loop);
        om.Clone2(this->id, out->id);
        om.Clone2(this->indexAtContainer, out->indexAtContainer);
        om.Clone2(this->coin, out->coin);
        om.Clone2(this->effectiveTime, out->effectiveTime);
        om.Clone2(this->actionName, out->actionName);
        om.Clone2(this->fileName, out->fileName);
        om.Clone2(this->pathwayName, out->pathwayName);
        om.Clone2(this->luaName, out->luaName);
        om.Clone2(this->luaData, out->luaData);
        om.Clone2(this->pathway, out->pathway);
        om.Clone2(this->children, out->children);
        om.Clone2(this->offset, out->offset);
        om.Clone2(this->file, out->file);
    }
    void Fish::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->pos);
        om.RecursiveReset(this->angle);
        om.RecursiveReset(this->scaleX);
        om.RecursiveReset(this->scaleY);
        om.RecursiveReset(this->animElapsedSeconds);
        om.RecursiveReset(this->totalElapsedSeconds);
        om.RecursiveReset(this->pathwayI);
        om.RecursiveReset(this->pathwayD);
        om.RecursiveReset(this->speedScale);
        om.RecursiveReset(this->timeScale);
        om.RecursiveReset(this->lpsCursor);
        om.RecursiveReset(this->cdsCursor);
        om.RecursiveReset(this->ssCursor);
        om.RecursiveReset(this->speed);
        om.RecursiveReset(this->loop);
        om.RecursiveReset(this->id);
        om.RecursiveReset(this->indexAtContainer);
        om.RecursiveReset(this->coin);
        om.RecursiveReset(this->effectiveTime);
        om.RecursiveReset(this->actionName);
        om.RecursiveReset(this->fileName);
        om.RecursiveReset(this->pathwayName);
        om.RecursiveReset(this->luaName);
        om.RecursiveReset(this->luaData);
        om.RecursiveReset(this->pathway);
        om.RecursiveReset(this->children);
        om.RecursiveReset(this->offset);
        om.RecursiveReset(this->file);
    }
    Stuff::Stuff(Stuff&& o) noexcept {
        this->operator=(std::move(o));
    }
    Stuff& Stuff::operator=(Stuff&& o) noexcept {
        std::swap(this->id, o.id);
        std::swap(this->typeId, o.typeId);
        std::swap(this->pos, o.pos);
        std::swap(this->effectiveTime, o.effectiveTime);
        return *this;
    }
    void Stuff::Write(::xx::ObjManager& om) const {
        om.Write(this->id);
        om.Write(this->typeId);
        om.Write(this->pos);
        om.Write(this->effectiveTime);
    }
    int Stuff::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->id)) return r;
        if (int r = om.Read(this->typeId)) return r;
        if (int r = om.Read(this->pos)) return r;
        if (int r = om.Read(this->effectiveTime)) return r;
        return 0;
    }
    void Stuff::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Stuff::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"id\":", this->id);
        om.Append(",\"typeId\":", this->typeId);
        om.Append(",\"pos\":", this->pos);
        om.Append(",\"effectiveTime\":", this->effectiveTime);
    }
    void Stuff::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Stuff*)tar;
        om.Clone1(this->id, out->id);
        om.Clone1(this->typeId, out->typeId);
        om.Clone1(this->pos, out->pos);
        om.Clone1(this->effectiveTime, out->effectiveTime);
    }
    void Stuff::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Stuff*)tar;
        om.Clone2(this->id, out->id);
        om.Clone2(this->typeId, out->typeId);
        om.Clone2(this->pos, out->pos);
        om.Clone2(this->effectiveTime, out->effectiveTime);
    }
    void Stuff::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->id);
        om.RecursiveReset(this->typeId);
        om.RecursiveReset(this->pos);
        om.RecursiveReset(this->effectiveTime);
    }
    Cannon::Cannon(Cannon&& o) noexcept {
        this->operator=(std::move(o));
    }
    Cannon& Cannon::operator=(Cannon&& o) noexcept {
        std::swap(this->id, o.id);
        std::swap(this->typeId, o.typeId);
        std::swap(this->bullets, o.bullets);
        return *this;
    }
    void Cannon::Write(::xx::ObjManager& om) const {
        om.Write(this->id);
        om.Write(this->typeId);
        om.Write(this->bullets);
    }
    int Cannon::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->id)) return r;
        if (int r = om.Read(this->typeId)) return r;
        if (int r = om.Read(this->bullets)) return r;
        return 0;
    }
    void Cannon::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Cannon::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"id\":", this->id);
        om.Append(",\"typeId\":", this->typeId);
        om.Append(",\"bullets\":", this->bullets);
    }
    void Cannon::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Cannon*)tar;
        om.Clone1(this->id, out->id);
        om.Clone1(this->typeId, out->typeId);
        om.Clone1(this->bullets, out->bullets);
    }
    void Cannon::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Cannon*)tar;
        om.Clone2(this->id, out->id);
        om.Clone2(this->typeId, out->typeId);
        om.Clone2(this->bullets, out->bullets);
    }
    void Cannon::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->id);
        om.RecursiveReset(this->typeId);
        om.RecursiveReset(this->bullets);
    }
    TimePoint_Frame::TimePoint_Frame(TimePoint_Frame&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimePoint_Frame& TimePoint_Frame::operator=(TimePoint_Frame&& o) noexcept {
        std::swap(this->time, o.time);
        std::swap(this->picName, o.picName);
        return *this;
    }
    Player::Player(Player&& o) noexcept {
        this->operator=(std::move(o));
    }
    Player& Player::operator=(Player&& o) noexcept {
        std::swap(this->id, o.id);
        std::swap(this->nickname, o.nickname);
        std::swap(this->coin, o.coin);
        std::swap(this->autoLock, o.autoLock);
        std::swap(this->autoFire, o.autoFire);
        std::swap(this->autoIncId, o.autoIncId);
        std::swap(this->sitId, o.sitId);
        std::swap(this->bankrupt, o.bankrupt);
        std::swap(this->cannons, o.cannons);
        std::swap(this->stuffs, o.stuffs);
        std::swap(this->aimFish, o.aimFish);
        return *this;
    }
    void Player::Write(::xx::ObjManager& om) const {
        om.Write(this->id);
        om.Write(this->nickname);
        om.Write(this->coin);
        om.Write(this->autoLock);
        om.Write(this->autoFire);
        om.Write(this->autoIncId);
        om.Write(this->sitId);
        om.Write(this->bankrupt);
        om.Write(this->cannons);
        om.Write(this->stuffs);
        om.Write(this->aimFish);
    }
    int Player::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->id)) return r;
        if (int r = om.Read(this->nickname)) return r;
        if (int r = om.Read(this->coin)) return r;
        if (int r = om.Read(this->autoLock)) return r;
        if (int r = om.Read(this->autoFire)) return r;
        if (int r = om.Read(this->autoIncId)) return r;
        if (int r = om.Read(this->sitId)) return r;
        if (int r = om.Read(this->bankrupt)) return r;
        if (int r = om.Read(this->cannons)) return r;
        if (int r = om.Read(this->stuffs)) return r;
        if (int r = om.Read(this->aimFish)) return r;
        return 0;
    }
    void Player::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Player::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"id\":", this->id);
        om.Append(",\"nickname\":", this->nickname);
        om.Append(",\"coin\":", this->coin);
        om.Append(",\"autoLock\":", this->autoLock);
        om.Append(",\"autoFire\":", this->autoFire);
        om.Append(",\"autoIncId\":", this->autoIncId);
        om.Append(",\"sitId\":", this->sitId);
        om.Append(",\"bankrupt\":", this->bankrupt);
        om.Append(",\"cannons\":", this->cannons);
        om.Append(",\"stuffs\":", this->stuffs);
        om.Append(",\"aimFish\":", this->aimFish);
    }
    void Player::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Player*)tar;
        om.Clone1(this->id, out->id);
        om.Clone1(this->nickname, out->nickname);
        om.Clone1(this->coin, out->coin);
        om.Clone1(this->autoLock, out->autoLock);
        om.Clone1(this->autoFire, out->autoFire);
        om.Clone1(this->autoIncId, out->autoIncId);
        om.Clone1(this->sitId, out->sitId);
        om.Clone1(this->bankrupt, out->bankrupt);
        om.Clone1(this->cannons, out->cannons);
        om.Clone1(this->stuffs, out->stuffs);
        om.Clone1(this->aimFish, out->aimFish);
    }
    void Player::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Player*)tar;
        om.Clone2(this->id, out->id);
        om.Clone2(this->nickname, out->nickname);
        om.Clone2(this->coin, out->coin);
        om.Clone2(this->autoLock, out->autoLock);
        om.Clone2(this->autoFire, out->autoFire);
        om.Clone2(this->autoIncId, out->autoIncId);
        om.Clone2(this->sitId, out->sitId);
        om.Clone2(this->bankrupt, out->bankrupt);
        om.Clone2(this->cannons, out->cannons);
        om.Clone2(this->stuffs, out->stuffs);
        om.Clone2(this->aimFish, out->aimFish);
    }
    void Player::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->id);
        om.RecursiveReset(this->nickname);
        om.RecursiveReset(this->coin);
        om.RecursiveReset(this->autoLock);
        om.RecursiveReset(this->autoFire);
        om.RecursiveReset(this->autoIncId);
        om.RecursiveReset(this->sitId);
        om.RecursiveReset(this->bankrupt);
        om.RecursiveReset(this->cannons);
        om.RecursiveReset(this->stuffs);
        om.RecursiveReset(this->aimFish);
    }
    CurvePoint::CurvePoint(CurvePoint&& o) noexcept {
        this->operator=(std::move(o));
    }
    CurvePoint& CurvePoint::operator=(CurvePoint&& o) noexcept {
        std::swap(this->x, o.x);
        std::swap(this->y, o.y);
        std::swap(this->tension, o.tension);
        std::swap(this->numSegments, o.numSegments);
        return *this;
    }
    SimpleBullet::SimpleBullet(SimpleBullet&& o) noexcept {
        this->operator=(std::move(o));
    }
    SimpleBullet& SimpleBullet::operator=(SimpleBullet&& o) noexcept {
        this->BaseType::operator=(std::move(o));
        std::swap(this->angle, o.angle);
        std::swap(this->pos, o.pos);
        return *this;
    }
    void SimpleBullet::Write(::xx::ObjManager& om) const {
        this->BaseType::Write(om);
        om.Write(this->angle);
        om.Write(this->pos);
    }
    int SimpleBullet::Read(::xx::ObjManager& om) {
        if (int r = this->BaseType::Read(om)) return r;
        if (int r = om.Read(this->angle)) return r;
        if (int r = om.Read(this->pos)) return r;
        return 0;
    }
    void SimpleBullet::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
        this->BaseType::ToStringCore(om);
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void SimpleBullet::ToStringCore(::xx::ObjManager& om) const {
        this->BaseType::ToStringCore(om);
        om.Append(",\"angle\":", this->angle);
        om.Append(",\"pos\":", this->pos);
    }
    void SimpleBullet::Clone1(::xx::ObjManager& om, void* const &tar) const {
        this->BaseType::Clone1(om, tar);
        auto out = (::FF::SimpleBullet*)tar;
        om.Clone1(this->angle, out->angle);
        om.Clone1(this->pos, out->pos);
    }
    void SimpleBullet::Clone2(::xx::ObjManager& om, void* const &tar) const {
        this->BaseType::Clone2(om, tar);
        auto out = (::FF::SimpleBullet*)tar;
        om.Clone2(this->angle, out->angle);
        om.Clone2(this->pos, out->pos);
    }
    void SimpleBullet::RecursiveReset(::xx::ObjManager& om) {
        this->BaseType::RecursiveReset(om);
        om.RecursiveReset(this->angle);
        om.RecursiveReset(this->pos);
    }
    Action_Frames::Action_Frames(Action_Frames&& o) noexcept {
        this->operator=(std::move(o));
    }
    Action_Frames& Action_Frames::operator=(Action_Frames&& o) noexcept {
        std::swap(this->name, o.name);
        std::swap(this->totalSeconds, o.totalSeconds);
        std::swap(this->frames, o.frames);
        return *this;
    }
    Root::Root(Root&& o) noexcept {
        this->operator=(std::move(o));
    }
    Root& Root::operator=(Root&& o) noexcept {
        std::swap(this->dtPool, o.dtPool);
        std::swap(this->frame, o.frame);
        std::swap(this->players, o.players);
        std::swap(this->fishs, o.fishs);
        return *this;
    }
    void Root::Write(::xx::ObjManager& om) const {
        om.Write(this->dtPool);
        om.Write(this->frame);
        om.Write(this->players);
        om.Write(this->fishs);
    }
    int Root::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->dtPool)) return r;
        if (int r = om.Read(this->frame)) return r;
        if (int r = om.Read(this->players)) return r;
        if (int r = om.Read(this->fishs)) return r;
        return 0;
    }
    void Root::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Root::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"dtPool\":", this->dtPool);
        om.Append(",\"frame\":", this->frame);
        om.Append(",\"players\":", this->players);
        om.Append(",\"fishs\":", this->fishs);
    }
    void Root::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Root*)tar;
        om.Clone1(this->dtPool, out->dtPool);
        om.Clone1(this->frame, out->frame);
        om.Clone1(this->players, out->players);
        om.Clone1(this->fishs, out->fishs);
    }
    void Root::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Root*)tar;
        om.Clone2(this->dtPool, out->dtPool);
        om.Clone2(this->frame, out->frame);
        om.Clone2(this->players, out->players);
        om.Clone2(this->fishs, out->fishs);
    }
    void Root::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->dtPool);
        om.RecursiveReset(this->frame);
        om.RecursiveReset(this->players);
        om.RecursiveReset(this->fishs);
    }
    Foo::Foo(Foo&& o) noexcept {
        this->operator=(std::move(o));
    }
    Foo& Foo::operator=(Foo&& o) noexcept {
        std::swap(this->children, o.children);
        std::swap(this->a, o.a);
        std::swap(this->b, o.b);
        std::swap(this->c, o.c);
        std::swap(this->d, o.d);
        std::swap(this->e, o.e);
        std::swap(this->f, o.f);
        std::swap(this->a1, o.a1);
        std::swap(this->b1, o.b1);
        std::swap(this->c1, o.c1);
        std::swap(this->d1, o.d1);
        std::swap(this->e1, o.e1);
        std::swap(this->f1, o.f1);
        std::swap(this->a2, o.a2);
        std::swap(this->b2, o.b2);
        std::swap(this->c2, o.c2);
        std::swap(this->d2, o.d2);
        std::swap(this->e2, o.e2);
        std::swap(this->f2, o.f2);
        return *this;
    }
    void Foo::Write(::xx::ObjManager& om) const {
        auto bak = om.data->WriteJump(sizeof(uint32_t));
        om.Write(this->children);
        om.Write(this->a);
        om.Write(this->b);
        om.Write(this->c);
        om.Write(this->d);
        om.Write(this->e);
        om.Write(this->f);
        om.Write(this->a1);
        om.Write(this->b1);
        om.Write(this->c1);
        om.Write(this->d1);
        om.Write(this->e1);
        om.Write(this->f1);
        om.Write(this->a2);
        om.Write(this->b2);
        om.Write(this->c2);
        om.Write(this->d2);
        om.Write(this->e2);
        om.Write(this->f2);
        om.data->WriteFixedAt(bak, (uint32_t)(om.data->len - bak));
    }
    int Foo::Read(::xx::ObjManager& om) {
        uint32_t siz;
        if (int r = om.data->ReadFixed(siz)) return r;
        auto endOffset = om.data->offset - sizeof(siz) + siz;

        if (om.data->offset >= endOffset) this->children.clear();
        else if (int r = om.Read(this->children)) return r;
        if (om.data->offset >= endOffset) this->a = 1;
        else if (int r = om.Read(this->a)) return r;
        if (om.data->offset >= endOffset) this->b = 2.3f;
        else if (int r = om.Read(this->b)) return r;
        if (om.data->offset >= endOffset) this->c = "asdf";
        else if (int r = om.Read(this->c)) return r;
        if (om.data->offset >= endOffset) this->d.Reset();
        else if (int r = om.Read(this->d)) return r;
        if (om.data->offset >= endOffset) this->e.Reset();
        else if (int r = om.Read(this->e)) return r;
        if (om.data->offset >= endOffset) this->f.reset();
        else if (int r = om.Read(this->f)) return r;
        if (om.data->offset >= endOffset) this->a1 = 1;
        else if (int r = om.Read(this->a1)) return r;
        if (om.data->offset >= endOffset) this->b1 = 2.3f;
        else if (int r = om.Read(this->b1)) return r;
        if (om.data->offset >= endOffset) this->c1 = "asdf";
        else if (int r = om.Read(this->c1)) return r;
        if (om.data->offset >= endOffset) this->d1.Reset();
        else if (int r = om.Read(this->d1)) return r;
        if (om.data->offset >= endOffset) this->e1.Reset();
        else if (int r = om.Read(this->e1)) return r;
        if (om.data->offset >= endOffset) this->f1.reset();
        else if (int r = om.Read(this->f1)) return r;
        if (om.data->offset >= endOffset) this->a2 = 1;
        else if (int r = om.Read(this->a2)) return r;
        if (om.data->offset >= endOffset) this->b2 = 2.3f;
        else if (int r = om.Read(this->b2)) return r;
        if (om.data->offset >= endOffset) this->c2 = "asdf";
        else if (int r = om.Read(this->c2)) return r;
        if (om.data->offset >= endOffset) this->d2.Reset();
        else if (int r = om.Read(this->d2)) return r;
        if (om.data->offset >= endOffset) this->e2.Reset();
        else if (int r = om.Read(this->e2)) return r;
        if (om.data->offset >= endOffset) this->f2.reset();
        else if (int r = om.Read(this->f2)) return r;

        if (om.data->offset > endOffset) return __LINE__;
        else om.data->offset = endOffset;
        return 0;
    }
    void Foo::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Foo::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"children\":", this->children);
        om.Append(",\"a\":", this->a);
        om.Append(",\"b\":", this->b);
        om.Append(",\"c\":", this->c);
        om.Append(",\"d\":", this->d);
        om.Append(",\"e\":", this->e);
        om.Append(",\"f\":", this->f);
        om.Append(",\"a1\":", this->a1);
        om.Append(",\"b1\":", this->b1);
        om.Append(",\"c1\":", this->c1);
        om.Append(",\"d1\":", this->d1);
        om.Append(",\"e1\":", this->e1);
        om.Append(",\"f1\":", this->f1);
        om.Append(",\"a2\":", this->a2);
        om.Append(",\"b2\":", this->b2);
        om.Append(",\"c2\":", this->c2);
        om.Append(",\"d2\":", this->d2);
        om.Append(",\"e2\":", this->e2);
        om.Append(",\"f2\":", this->f2);
    }
    void Foo::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Foo*)tar;
        om.Clone1(this->children, out->children);
        om.Clone1(this->a, out->a);
        om.Clone1(this->b, out->b);
        om.Clone1(this->c, out->c);
        om.Clone1(this->d, out->d);
        om.Clone1(this->e, out->e);
        om.Clone1(this->f, out->f);
        om.Clone1(this->a1, out->a1);
        om.Clone1(this->b1, out->b1);
        om.Clone1(this->c1, out->c1);
        om.Clone1(this->d1, out->d1);
        om.Clone1(this->e1, out->e1);
        om.Clone1(this->f1, out->f1);
        om.Clone1(this->a2, out->a2);
        om.Clone1(this->b2, out->b2);
        om.Clone1(this->c2, out->c2);
        om.Clone1(this->d2, out->d2);
        om.Clone1(this->e2, out->e2);
        om.Clone1(this->f2, out->f2);
    }
    void Foo::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Foo*)tar;
        om.Clone2(this->children, out->children);
        om.Clone2(this->a, out->a);
        om.Clone2(this->b, out->b);
        om.Clone2(this->c, out->c);
        om.Clone2(this->d, out->d);
        om.Clone2(this->e, out->e);
        om.Clone2(this->f, out->f);
        om.Clone2(this->a1, out->a1);
        om.Clone2(this->b1, out->b1);
        om.Clone2(this->c1, out->c1);
        om.Clone2(this->d1, out->d1);
        om.Clone2(this->e1, out->e1);
        om.Clone2(this->f1, out->f1);
        om.Clone2(this->a2, out->a2);
        om.Clone2(this->b2, out->b2);
        om.Clone2(this->c2, out->c2);
        om.Clone2(this->d2, out->d2);
        om.Clone2(this->e2, out->e2);
        om.Clone2(this->f2, out->f2);
    }
    void Foo::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->children);
        om.RecursiveReset(this->a);
        om.RecursiveReset(this->b);
        om.RecursiveReset(this->c);
        om.RecursiveReset(this->d);
        om.RecursiveReset(this->e);
        om.RecursiveReset(this->f);
        om.RecursiveReset(this->a1);
        om.RecursiveReset(this->b1);
        om.RecursiveReset(this->c1);
        om.RecursiveReset(this->d1);
        om.RecursiveReset(this->e1);
        om.RecursiveReset(this->f1);
        om.RecursiveReset(this->a2);
        om.RecursiveReset(this->b2);
        om.RecursiveReset(this->c2);
        om.RecursiveReset(this->d2);
        om.RecursiveReset(this->e2);
        om.RecursiveReset(this->f2);
    }
    File_Frames::File_Frames(File_Frames&& o) noexcept {
        this->operator=(std::move(o));
    }
    File_Frames& File_Frames::operator=(File_Frames&& o) noexcept {
        std::swap(this->actions, o.actions);
        std::swap(this->plists, o.plists);
        return *this;
    }
    Foo2::Foo2(Foo2&& o) noexcept {
        this->operator=(std::move(o));
    }
    Foo2& Foo2::operator=(Foo2&& o) noexcept {
        std::swap(this->children, o.children);
        std::swap(this->a, o.a);
        std::swap(this->b, o.b);
        std::swap(this->c, o.c);
        std::swap(this->d, o.d);
        std::swap(this->e, o.e);
        std::swap(this->f, o.f);
        std::swap(this->a1, o.a1);
        std::swap(this->b1, o.b1);
        std::swap(this->c1, o.c1);
        std::swap(this->d1, o.d1);
        std::swap(this->e1, o.e1);
        std::swap(this->f1, o.f1);
        std::swap(this->a2, o.a2);
        std::swap(this->b2, o.b2);
        std::swap(this->c2, o.c2);
        std::swap(this->d2, o.d2);
        std::swap(this->e2, o.e2);
        std::swap(this->f2, o.f2);
        return *this;
    }
    void Foo2::Write(::xx::ObjManager& om) const {
        om.Write(this->children);
        om.Write(this->a);
        om.Write(this->b);
        om.Write(this->c);
        om.Write(this->d);
        om.Write(this->e);
        om.Write(this->f);
        om.Write(this->a1);
        om.Write(this->b1);
        om.Write(this->c1);
        om.Write(this->d1);
        om.Write(this->e1);
        om.Write(this->f1);
        om.Write(this->a2);
        om.Write(this->b2);
        om.Write(this->c2);
        om.Write(this->d2);
        om.Write(this->e2);
        om.Write(this->f2);
    }
    int Foo2::Read(::xx::ObjManager& om) {
        if (int r = om.Read(this->children)) return r;
        if (int r = om.Read(this->a)) return r;
        if (int r = om.Read(this->b)) return r;
        if (int r = om.Read(this->c)) return r;
        if (int r = om.Read(this->d)) return r;
        if (int r = om.Read(this->e)) return r;
        if (int r = om.Read(this->f)) return r;
        if (int r = om.Read(this->a1)) return r;
        if (int r = om.Read(this->b1)) return r;
        if (int r = om.Read(this->c1)) return r;
        if (int r = om.Read(this->d1)) return r;
        if (int r = om.Read(this->e1)) return r;
        if (int r = om.Read(this->f1)) return r;
        if (int r = om.Read(this->a2)) return r;
        if (int r = om.Read(this->b2)) return r;
        if (int r = om.Read(this->c2)) return r;
        if (int r = om.Read(this->d2)) return r;
        if (int r = om.Read(this->e2)) return r;
        if (int r = om.Read(this->f2)) return r;
        return 0;
    }
    void Foo2::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void Foo2::ToStringCore(::xx::ObjManager& om) const {
        om.Append(",\"children\":", this->children);
        om.Append(",\"a\":", this->a);
        om.Append(",\"b\":", this->b);
        om.Append(",\"c\":", this->c);
        om.Append(",\"d\":", this->d);
        om.Append(",\"e\":", this->e);
        om.Append(",\"f\":", this->f);
        om.Append(",\"a1\":", this->a1);
        om.Append(",\"b1\":", this->b1);
        om.Append(",\"c1\":", this->c1);
        om.Append(",\"d1\":", this->d1);
        om.Append(",\"e1\":", this->e1);
        om.Append(",\"f1\":", this->f1);
        om.Append(",\"a2\":", this->a2);
        om.Append(",\"b2\":", this->b2);
        om.Append(",\"c2\":", this->c2);
        om.Append(",\"d2\":", this->d2);
        om.Append(",\"e2\":", this->e2);
        om.Append(",\"f2\":", this->f2);
    }
    void Foo2::Clone1(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Foo2*)tar;
        om.Clone1(this->children, out->children);
        om.Clone1(this->a, out->a);
        om.Clone1(this->b, out->b);
        om.Clone1(this->c, out->c);
        om.Clone1(this->d, out->d);
        om.Clone1(this->e, out->e);
        om.Clone1(this->f, out->f);
        om.Clone1(this->a1, out->a1);
        om.Clone1(this->b1, out->b1);
        om.Clone1(this->c1, out->c1);
        om.Clone1(this->d1, out->d1);
        om.Clone1(this->e1, out->e1);
        om.Clone1(this->f1, out->f1);
        om.Clone1(this->a2, out->a2);
        om.Clone1(this->b2, out->b2);
        om.Clone1(this->c2, out->c2);
        om.Clone1(this->d2, out->d2);
        om.Clone1(this->e2, out->e2);
        om.Clone1(this->f2, out->f2);
    }
    void Foo2::Clone2(::xx::ObjManager& om, void* const &tar) const {
        auto out = (::FF::Foo2*)tar;
        om.Clone2(this->children, out->children);
        om.Clone2(this->a, out->a);
        om.Clone2(this->b, out->b);
        om.Clone2(this->c, out->c);
        om.Clone2(this->d, out->d);
        om.Clone2(this->e, out->e);
        om.Clone2(this->f, out->f);
        om.Clone2(this->a1, out->a1);
        om.Clone2(this->b1, out->b1);
        om.Clone2(this->c1, out->c1);
        om.Clone2(this->d1, out->d1);
        om.Clone2(this->e1, out->e1);
        om.Clone2(this->f1, out->f1);
        om.Clone2(this->a2, out->a2);
        om.Clone2(this->b2, out->b2);
        om.Clone2(this->c2, out->c2);
        om.Clone2(this->d2, out->d2);
        om.Clone2(this->e2, out->e2);
        om.Clone2(this->f2, out->f2);
    }
    void Foo2::RecursiveReset(::xx::ObjManager& om) {
        om.RecursiveReset(this->children);
        om.RecursiveReset(this->a);
        om.RecursiveReset(this->b);
        om.RecursiveReset(this->c);
        om.RecursiveReset(this->d);
        om.RecursiveReset(this->e);
        om.RecursiveReset(this->f);
        om.RecursiveReset(this->a1);
        om.RecursiveReset(this->b1);
        om.RecursiveReset(this->c1);
        om.RecursiveReset(this->d1);
        om.RecursiveReset(this->e1);
        om.RecursiveReset(this->f1);
        om.RecursiveReset(this->a2);
        om.RecursiveReset(this->b2);
        om.RecursiveReset(this->c2);
        om.RecursiveReset(this->d2);
        om.RecursiveReset(this->e2);
        om.RecursiveReset(this->f2);
    }
    File_pathway::File_pathway(File_pathway&& o) noexcept {
        this->operator=(std::move(o));
    }
    File_pathway& File_pathway::operator=(File_pathway&& o) noexcept {
        std::swap(this->isLoop, o.isLoop);
        std::swap(this->points, o.points);
        return *this;
    }
    TrackBullet::TrackBullet(TrackBullet&& o) noexcept {
        this->operator=(std::move(o));
    }
    TrackBullet& TrackBullet::operator=(TrackBullet&& o) noexcept {
        this->BaseType::operator=(std::move(o));
        return *this;
    }
    void TrackBullet::Write(::xx::ObjManager& om) const {
        this->BaseType::Write(om);
    }
    int TrackBullet::Read(::xx::ObjManager& om) {
        if (int r = this->BaseType::Read(om)) return r;
        return 0;
    }
    void TrackBullet::ToString(::xx::ObjManager& om) const {
        om.Append("{\"__typeId__\":", this->ObjBase::GetTypeId());
        this->BaseType::ToStringCore(om);
		this->ToStringCore(om);
		om.str->push_back('}');
    }
    void TrackBullet::ToStringCore(::xx::ObjManager& om) const {
        this->BaseType::ToStringCore(om);
    }
    void TrackBullet::Clone1(::xx::ObjManager& om, void* const &tar) const {
        this->BaseType::Clone1(om, tar);
        auto out = (::FF::TrackBullet*)tar;
    }
    void TrackBullet::Clone2(::xx::ObjManager& om, void* const &tar) const {
        this->BaseType::Clone2(om, tar);
        auto out = (::FF::TrackBullet*)tar;
    }
    void TrackBullet::RecursiveReset(::xx::ObjManager& om) {
        this->BaseType::RecursiveReset(om);
    }
}
