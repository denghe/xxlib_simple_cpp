#pragma once
#include "FF_class_lite.h"
#include "ajson.hpp"
AJSON(FF::Point, x, y);
AJSON(FF::TimePoint_Speed, time, speed);
AJSON(FF::TimePoint_Anim, time, animIndex);
AJSON(FF::PathwayPoint, pos, a, d);
AJSON(FF::Action, name, totalSeconds, anims, speeds);
AJSON(FF::LockPoint, x, y);
AJSON(FF::CDCircle, x, y, r);
AJSON(FF::File_Actions, animsFileName, actions);
AJSON(FF::TimePoint_CDCircles, time, maxCDCircle, cdCircles);
AJSON(FF::TimePoint_Frame, time, picName);
AJSON(FF::TimePoint_LockPoints, time, mainLockPoint, lockPoints);
AJSON(FF::Anim, name, totalSeconds, lps, cds);
AJSON(FF::FrameAnim, name, totalSeconds, frames);
AJSON(FF::CurvePoint, x, y, tension, numSegments);
AJSON(FF::File_Anims, resFileName, anims, shadowX, shadowY, shadowScale);
AJSON(FF::File_Frames, frameAnims, plists);
AJSON(FF::File_pathway, isLoop, points);
