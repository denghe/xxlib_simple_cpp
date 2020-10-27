#pragma once
#include "FileExts_class_lite.h.inc"
#include "ajson.hpp"
AJSON(FileExts::LockPoint, x, y);
AJSON(FileExts::CDCircle, x, y, r);
AJSON(FileExts::TimePoint_LockPoints, time, mainLockPoint, lockPoints);
AJSON(FileExts::TimePoint_CDCircles, time, maxCDCircle, cdCircles);
AJSON(FileExts::TimePoint_Speed, time, speed);
AJSON(FileExts::TimePoint_Frame, time, picName);
AJSON(FileExts::Action, name, totalSeconds, lps, cds, ss, fs);
AJSON(FileExts::File_Anim, fileName, actions);
AJSON(FileExts::File_frames, fileName, actions);
AJSON(FileExts::File_atlas_ext, fileName, actions);
AJSON(FileExts::File_c3b_ext, fileName, actions);
