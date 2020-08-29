#include "FileExts_class_lite.h"
#include "FileExts_class_lite.cpp.inc"
namespace FileExts {
	void PkgGenTypes::RegisterTo(xx::ObjectHelper& oh) {
	}
}

namespace xx {
    void CloneFuncs<FileExts::LockPoint>::Clone1(xx::ObjectHelper &oh, FileExts::LockPoint const& in, FileExts::LockPoint &out) {
        CloneFuncs<float>::Clone1(oh, in.x, out.x);
        CloneFuncs<float>::Clone1(oh, in.y, out.y);
    }
    void CloneFuncs<FileExts::LockPoint>::Clone2(xx::ObjectHelper &oh, FileExts::LockPoint const& in, FileExts::LockPoint &out) {
        CloneFuncs<float>::Clone2(oh, in.x, out.x);
        CloneFuncs<float>::Clone2(oh, in.y, out.y);
    }
	void DataFuncsEx<FileExts::LockPoint, void>::Write(DataWriterEx& dw, FileExts::LockPoint const& in) {
        dw.Write(in.x);
        dw.Write(in.y);
    }
	int DataFuncsEx<FileExts::LockPoint, void>::Read(DataReaderEx& d, FileExts::LockPoint& out) {
        if (int r = d.Read(out.x)) return r;
        if (int r = d.Read(out.y)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::LockPoint, void>::Append(ObjectHelper &oh, FileExts::LockPoint const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::LockPoint, void>::AppendCore(ObjectHelper &oh, FileExts::LockPoint const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"x\":", in.x); 
        xx::AppendEx(oh, ",\"y\":", in.y);
    }
    void CloneFuncs<FileExts::CDCircle>::Clone1(xx::ObjectHelper &oh, FileExts::CDCircle const& in, FileExts::CDCircle &out) {
        CloneFuncs<float>::Clone1(oh, in.x, out.x);
        CloneFuncs<float>::Clone1(oh, in.y, out.y);
        CloneFuncs<float>::Clone1(oh, in.r, out.r);
    }
    void CloneFuncs<FileExts::CDCircle>::Clone2(xx::ObjectHelper &oh, FileExts::CDCircle const& in, FileExts::CDCircle &out) {
        CloneFuncs<float>::Clone2(oh, in.x, out.x);
        CloneFuncs<float>::Clone2(oh, in.y, out.y);
        CloneFuncs<float>::Clone2(oh, in.r, out.r);
    }
	void DataFuncsEx<FileExts::CDCircle, void>::Write(DataWriterEx& dw, FileExts::CDCircle const& in) {
        dw.Write(in.x);
        dw.Write(in.y);
        dw.Write(in.r);
    }
	int DataFuncsEx<FileExts::CDCircle, void>::Read(DataReaderEx& d, FileExts::CDCircle& out) {
        if (int r = d.Read(out.x)) return r;
        if (int r = d.Read(out.y)) return r;
        if (int r = d.Read(out.r)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::CDCircle, void>::Append(ObjectHelper &oh, FileExts::CDCircle const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::CDCircle, void>::AppendCore(ObjectHelper &oh, FileExts::CDCircle const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"x\":", in.x); 
        xx::AppendEx(oh, ",\"y\":", in.y);
        xx::AppendEx(oh, ",\"r\":", in.r);
    }
    void CloneFuncs<FileExts::TimePoint_LockPoints>::Clone1(xx::ObjectHelper &oh, FileExts::TimePoint_LockPoints const& in, FileExts::TimePoint_LockPoints &out) {
        CloneFuncs<float>::Clone1(oh, in.time, out.time);
        CloneFuncs<FileExts::LockPoint>::Clone1(oh, in.mainLockPoint, out.mainLockPoint);
        CloneFuncs<std::vector<FileExts::LockPoint>>::Clone1(oh, in.lockPoints, out.lockPoints);
    }
    void CloneFuncs<FileExts::TimePoint_LockPoints>::Clone2(xx::ObjectHelper &oh, FileExts::TimePoint_LockPoints const& in, FileExts::TimePoint_LockPoints &out) {
        CloneFuncs<float>::Clone2(oh, in.time, out.time);
        CloneFuncs<FileExts::LockPoint>::Clone2(oh, in.mainLockPoint, out.mainLockPoint);
        CloneFuncs<std::vector<FileExts::LockPoint>>::Clone2(oh, in.lockPoints, out.lockPoints);
    }
	void DataFuncsEx<FileExts::TimePoint_LockPoints, void>::Write(DataWriterEx& dw, FileExts::TimePoint_LockPoints const& in) {
        dw.Write(in.time);
        dw.Write(in.mainLockPoint);
        dw.Write(in.lockPoints);
    }
	int DataFuncsEx<FileExts::TimePoint_LockPoints, void>::Read(DataReaderEx& d, FileExts::TimePoint_LockPoints& out) {
        if (int r = d.Read(out.time)) return r;
        if (int r = d.Read(out.mainLockPoint)) return r;
        if (int r = d.Read(out.lockPoints)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::TimePoint_LockPoints, void>::Append(ObjectHelper &oh, FileExts::TimePoint_LockPoints const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::TimePoint_LockPoints, void>::AppendCore(ObjectHelper &oh, FileExts::TimePoint_LockPoints const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"time\":", in.time); 
        xx::AppendEx(oh, ",\"mainLockPoint\":", in.mainLockPoint);
        xx::AppendEx(oh, ",\"lockPoints\":", in.lockPoints);
    }
    void CloneFuncs<FileExts::TimePoint_CDCircles>::Clone1(xx::ObjectHelper &oh, FileExts::TimePoint_CDCircles const& in, FileExts::TimePoint_CDCircles &out) {
        CloneFuncs<float>::Clone1(oh, in.time, out.time);
        CloneFuncs<FileExts::CDCircle>::Clone1(oh, in.maxCDCircle, out.maxCDCircle);
        CloneFuncs<std::vector<FileExts::CDCircle>>::Clone1(oh, in.cdCircles, out.cdCircles);
    }
    void CloneFuncs<FileExts::TimePoint_CDCircles>::Clone2(xx::ObjectHelper &oh, FileExts::TimePoint_CDCircles const& in, FileExts::TimePoint_CDCircles &out) {
        CloneFuncs<float>::Clone2(oh, in.time, out.time);
        CloneFuncs<FileExts::CDCircle>::Clone2(oh, in.maxCDCircle, out.maxCDCircle);
        CloneFuncs<std::vector<FileExts::CDCircle>>::Clone2(oh, in.cdCircles, out.cdCircles);
    }
	void DataFuncsEx<FileExts::TimePoint_CDCircles, void>::Write(DataWriterEx& dw, FileExts::TimePoint_CDCircles const& in) {
        dw.Write(in.time);
        dw.Write(in.maxCDCircle);
        dw.Write(in.cdCircles);
    }
	int DataFuncsEx<FileExts::TimePoint_CDCircles, void>::Read(DataReaderEx& d, FileExts::TimePoint_CDCircles& out) {
        if (int r = d.Read(out.time)) return r;
        if (int r = d.Read(out.maxCDCircle)) return r;
        if (int r = d.Read(out.cdCircles)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::TimePoint_CDCircles, void>::Append(ObjectHelper &oh, FileExts::TimePoint_CDCircles const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::TimePoint_CDCircles, void>::AppendCore(ObjectHelper &oh, FileExts::TimePoint_CDCircles const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"time\":", in.time); 
        xx::AppendEx(oh, ",\"maxCDCircle\":", in.maxCDCircle);
        xx::AppendEx(oh, ",\"cdCircles\":", in.cdCircles);
    }
    void CloneFuncs<FileExts::TimePoint_Speed>::Clone1(xx::ObjectHelper &oh, FileExts::TimePoint_Speed const& in, FileExts::TimePoint_Speed &out) {
        CloneFuncs<float>::Clone1(oh, in.time, out.time);
        CloneFuncs<float>::Clone1(oh, in.speed, out.speed);
    }
    void CloneFuncs<FileExts::TimePoint_Speed>::Clone2(xx::ObjectHelper &oh, FileExts::TimePoint_Speed const& in, FileExts::TimePoint_Speed &out) {
        CloneFuncs<float>::Clone2(oh, in.time, out.time);
        CloneFuncs<float>::Clone2(oh, in.speed, out.speed);
    }
	void DataFuncsEx<FileExts::TimePoint_Speed, void>::Write(DataWriterEx& dw, FileExts::TimePoint_Speed const& in) {
        dw.Write(in.time);
        dw.Write(in.speed);
    }
	int DataFuncsEx<FileExts::TimePoint_Speed, void>::Read(DataReaderEx& d, FileExts::TimePoint_Speed& out) {
        if (int r = d.Read(out.time)) return r;
        if (int r = d.Read(out.speed)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::TimePoint_Speed, void>::Append(ObjectHelper &oh, FileExts::TimePoint_Speed const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::TimePoint_Speed, void>::AppendCore(ObjectHelper &oh, FileExts::TimePoint_Speed const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"time\":", in.time); 
        xx::AppendEx(oh, ",\"speed\":", in.speed);
    }
    void CloneFuncs<FileExts::TimePoint_Frame>::Clone1(xx::ObjectHelper &oh, FileExts::TimePoint_Frame const& in, FileExts::TimePoint_Frame &out) {
        CloneFuncs<float>::Clone1(oh, in.time, out.time);
        CloneFuncs<std::string>::Clone1(oh, in.picName, out.picName);
    }
    void CloneFuncs<FileExts::TimePoint_Frame>::Clone2(xx::ObjectHelper &oh, FileExts::TimePoint_Frame const& in, FileExts::TimePoint_Frame &out) {
        CloneFuncs<float>::Clone2(oh, in.time, out.time);
        CloneFuncs<std::string>::Clone2(oh, in.picName, out.picName);
    }
	void DataFuncsEx<FileExts::TimePoint_Frame, void>::Write(DataWriterEx& dw, FileExts::TimePoint_Frame const& in) {
        dw.Write(in.time);
        dw.Write(in.picName);
    }
	int DataFuncsEx<FileExts::TimePoint_Frame, void>::Read(DataReaderEx& d, FileExts::TimePoint_Frame& out) {
        if (int r = d.Read(out.time)) return r;
        if (int r = d.Read(out.picName)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::TimePoint_Frame, void>::Append(ObjectHelper &oh, FileExts::TimePoint_Frame const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::TimePoint_Frame, void>::AppendCore(ObjectHelper &oh, FileExts::TimePoint_Frame const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"time\":", in.time); 
        xx::AppendEx(oh, ",\"picName\":", in.picName);
    }
    void CloneFuncs<FileExts::Action>::Clone1(xx::ObjectHelper &oh, FileExts::Action const& in, FileExts::Action &out) {
        CloneFuncs<std::string>::Clone1(oh, in.name, out.name);
        CloneFuncs<float>::Clone1(oh, in.totalSeconds, out.totalSeconds);
        CloneFuncs<std::vector<FileExts::TimePoint_LockPoints>>::Clone1(oh, in.lps, out.lps);
        CloneFuncs<std::vector<FileExts::TimePoint_CDCircles>>::Clone1(oh, in.cds, out.cds);
        CloneFuncs<std::vector<FileExts::TimePoint_Speed>>::Clone1(oh, in.ss, out.ss);
        CloneFuncs<std::vector<FileExts::TimePoint_Frame>>::Clone1(oh, in.fs, out.fs);
    }
    void CloneFuncs<FileExts::Action>::Clone2(xx::ObjectHelper &oh, FileExts::Action const& in, FileExts::Action &out) {
        CloneFuncs<std::string>::Clone2(oh, in.name, out.name);
        CloneFuncs<float>::Clone2(oh, in.totalSeconds, out.totalSeconds);
        CloneFuncs<std::vector<FileExts::TimePoint_LockPoints>>::Clone2(oh, in.lps, out.lps);
        CloneFuncs<std::vector<FileExts::TimePoint_CDCircles>>::Clone2(oh, in.cds, out.cds);
        CloneFuncs<std::vector<FileExts::TimePoint_Speed>>::Clone2(oh, in.ss, out.ss);
        CloneFuncs<std::vector<FileExts::TimePoint_Frame>>::Clone2(oh, in.fs, out.fs);
    }
	void DataFuncsEx<FileExts::Action, void>::Write(DataWriterEx& dw, FileExts::Action const& in) {
        dw.Write(in.name);
        dw.Write(in.totalSeconds);
        dw.Write(in.lps);
        dw.Write(in.cds);
        dw.Write(in.ss);
        dw.Write(in.fs);
    }
	int DataFuncsEx<FileExts::Action, void>::Read(DataReaderEx& d, FileExts::Action& out) {
        if (int r = d.Read(out.name)) return r;
        if (int r = d.Read(out.totalSeconds)) return r;
        if (int r = d.Read(out.lps)) return r;
        if (int r = d.Read(out.cds)) return r;
        if (int r = d.Read(out.ss)) return r;
        if (int r = d.Read(out.fs)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::Action, void>::Append(ObjectHelper &oh, FileExts::Action const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::Action, void>::AppendCore(ObjectHelper &oh, FileExts::Action const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"name\":", in.name); 
        xx::AppendEx(oh, ",\"totalSeconds\":", in.totalSeconds);
        xx::AppendEx(oh, ",\"lps\":", in.lps);
        xx::AppendEx(oh, ",\"cds\":", in.cds);
        xx::AppendEx(oh, ",\"ss\":", in.ss);
        xx::AppendEx(oh, ",\"fs\":", in.fs);
    }
    void CloneFuncs<FileExts::File_Anim>::Clone1(xx::ObjectHelper &oh, FileExts::File_Anim const& in, FileExts::File_Anim &out) {
        CloneFuncs<std::string>::Clone1(oh, in.fileName, out.fileName);
        CloneFuncs<std::vector<FileExts::Action>>::Clone1(oh, in.actions, out.actions);
    }
    void CloneFuncs<FileExts::File_Anim>::Clone2(xx::ObjectHelper &oh, FileExts::File_Anim const& in, FileExts::File_Anim &out) {
        CloneFuncs<std::string>::Clone2(oh, in.fileName, out.fileName);
        CloneFuncs<std::vector<FileExts::Action>>::Clone2(oh, in.actions, out.actions);
    }
	void DataFuncsEx<FileExts::File_Anim, void>::Write(DataWriterEx& dw, FileExts::File_Anim const& in) {
        dw.Write(in.fileName);
        dw.Write(in.actions);
    }
	int DataFuncsEx<FileExts::File_Anim, void>::Read(DataReaderEx& d, FileExts::File_Anim& out) {
        if (int r = d.Read(out.fileName)) return r;
        if (int r = d.Read(out.actions)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::File_Anim, void>::Append(ObjectHelper &oh, FileExts::File_Anim const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::File_Anim, void>::AppendCore(ObjectHelper &oh, FileExts::File_Anim const& in) {
        auto sizeBak = oh.s.size();
        if (sizeBak == oh.s.size()) {
            oh.s.push_back(',');
        }
        xx::AppendEx(oh, "\"fileName\":", in.fileName); 
        xx::AppendEx(oh, ",\"actions\":", in.actions);
    }
    void CloneFuncs<FileExts::File_frames>::Clone1(xx::ObjectHelper &oh, FileExts::File_frames const& in, FileExts::File_frames &out) {
        CloneFuncs<FileExts::File_Anim>::Clone1(oh, in, out);
    }
    void CloneFuncs<FileExts::File_frames>::Clone2(xx::ObjectHelper &oh, FileExts::File_frames const& in, FileExts::File_frames &out) {
        CloneFuncs<FileExts::File_Anim>::Clone2(oh, in, out);
    }
	void DataFuncsEx<FileExts::File_frames, void>::Write(DataWriterEx& dw, FileExts::File_frames const& in) {
        DataFuncsEx<FileExts::File_Anim>::Write(dw, in);
    }
	int DataFuncsEx<FileExts::File_frames, void>::Read(DataReaderEx& d, FileExts::File_frames& out) {
        if (int r = DataFuncsEx<FileExts::File_Anim>::Read(d, out)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::File_frames, void>::Append(ObjectHelper &oh, FileExts::File_frames const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::File_frames, void>::AppendCore(ObjectHelper &oh, FileExts::File_frames const& in) {
        auto sizeBak = oh.s.size();
        StringFuncsEx<FileExts::File_Anim>::AppendCore(oh, in);
    }
    void CloneFuncs<FileExts::File_atlas_ext>::Clone1(xx::ObjectHelper &oh, FileExts::File_atlas_ext const& in, FileExts::File_atlas_ext &out) {
        CloneFuncs<FileExts::File_Anim>::Clone1(oh, in, out);
    }
    void CloneFuncs<FileExts::File_atlas_ext>::Clone2(xx::ObjectHelper &oh, FileExts::File_atlas_ext const& in, FileExts::File_atlas_ext &out) {
        CloneFuncs<FileExts::File_Anim>::Clone2(oh, in, out);
    }
	void DataFuncsEx<FileExts::File_atlas_ext, void>::Write(DataWriterEx& dw, FileExts::File_atlas_ext const& in) {
        DataFuncsEx<FileExts::File_Anim>::Write(dw, in);
    }
	int DataFuncsEx<FileExts::File_atlas_ext, void>::Read(DataReaderEx& d, FileExts::File_atlas_ext& out) {
        if (int r = DataFuncsEx<FileExts::File_Anim>::Read(d, out)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::File_atlas_ext, void>::Append(ObjectHelper &oh, FileExts::File_atlas_ext const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::File_atlas_ext, void>::AppendCore(ObjectHelper &oh, FileExts::File_atlas_ext const& in) {
        auto sizeBak = oh.s.size();
        StringFuncsEx<FileExts::File_Anim>::AppendCore(oh, in);
    }
    void CloneFuncs<FileExts::File_c3b_ext>::Clone1(xx::ObjectHelper &oh, FileExts::File_c3b_ext const& in, FileExts::File_c3b_ext &out) {
        CloneFuncs<FileExts::File_Anim>::Clone1(oh, in, out);
    }
    void CloneFuncs<FileExts::File_c3b_ext>::Clone2(xx::ObjectHelper &oh, FileExts::File_c3b_ext const& in, FileExts::File_c3b_ext &out) {
        CloneFuncs<FileExts::File_Anim>::Clone2(oh, in, out);
    }
	void DataFuncsEx<FileExts::File_c3b_ext, void>::Write(DataWriterEx& dw, FileExts::File_c3b_ext const& in) {
        DataFuncsEx<FileExts::File_Anim>::Write(dw, in);
    }
	int DataFuncsEx<FileExts::File_c3b_ext, void>::Read(DataReaderEx& d, FileExts::File_c3b_ext& out) {
        if (int r = DataFuncsEx<FileExts::File_Anim>::Read(d, out)) return r;
        return 0;
    }
	void StringFuncsEx<FileExts::File_c3b_ext, void>::Append(ObjectHelper &oh, FileExts::File_c3b_ext const& in) {
        oh.s.push_back('{');
        AppendCore(oh, in);
        oh.s.push_back('}');
    }
	void StringFuncsEx<FileExts::File_c3b_ext, void>::AppendCore(ObjectHelper &oh, FileExts::File_c3b_ext const& in) {
        auto sizeBak = oh.s.size();
        StringFuncsEx<FileExts::File_Anim>::AppendCore(oh, in);
    }
}
namespace FileExts {
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
    TimePoint_LockPoints::TimePoint_LockPoints(TimePoint_LockPoints&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimePoint_LockPoints& TimePoint_LockPoints::operator=(TimePoint_LockPoints&& o) noexcept {
        std::swap(this->time, o.time);
        std::swap(this->mainLockPoint, o.mainLockPoint);
        std::swap(this->lockPoints, o.lockPoints);
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
    TimePoint_Speed::TimePoint_Speed(TimePoint_Speed&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimePoint_Speed& TimePoint_Speed::operator=(TimePoint_Speed&& o) noexcept {
        std::swap(this->time, o.time);
        std::swap(this->speed, o.speed);
        return *this;
    }
    TimePoint_Frame::TimePoint_Frame(TimePoint_Frame&& o) noexcept {
        this->operator=(std::move(o));
    }
    TimePoint_Frame& TimePoint_Frame::operator=(TimePoint_Frame&& o) noexcept {
        std::swap(this->time, o.time);
        std::swap(this->picName, o.picName);
        return *this;
    }
    Action::Action(Action&& o) noexcept {
        this->operator=(std::move(o));
    }
    Action& Action::operator=(Action&& o) noexcept {
        std::swap(this->name, o.name);
        std::swap(this->totalSeconds, o.totalSeconds);
        std::swap(this->lps, o.lps);
        std::swap(this->cds, o.cds);
        std::swap(this->ss, o.ss);
        std::swap(this->fs, o.fs);
        return *this;
    }
    File_Anim::File_Anim(File_Anim&& o) noexcept {
        this->operator=(std::move(o));
    }
    File_Anim& File_Anim::operator=(File_Anim&& o) noexcept {
        std::swap(this->fileName, o.fileName);
        std::swap(this->actions, o.actions);
        return *this;
    }
    File_frames::File_frames(File_frames&& o) noexcept {
        this->operator=(std::move(o));
    }
    File_frames& File_frames::operator=(File_frames&& o) noexcept {
        this->FileExts::File_Anim::operator=(std::move(o));
        return *this;
    }
    File_atlas_ext::File_atlas_ext(File_atlas_ext&& o) noexcept {
        this->operator=(std::move(o));
    }
    File_atlas_ext& File_atlas_ext::operator=(File_atlas_ext&& o) noexcept {
        this->FileExts::File_Anim::operator=(std::move(o));
        return *this;
    }
    File_c3b_ext::File_c3b_ext(File_c3b_ext&& o) noexcept {
        this->operator=(std::move(o));
    }
    File_c3b_ext& File_c3b_ext::operator=(File_c3b_ext&& o) noexcept {
        this->FileExts::File_Anim::operator=(std::move(o));
        return *this;
    }
}
