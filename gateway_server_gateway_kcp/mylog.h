#pragma once
#include "xx_logger.h"
inline xx::Logger __xxLogger;
#define LOG_INFO(...) __xxLogger.Log(xx::LogLevels::INFO, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...) __xxLogger.Log(xx::LogLevels::WARN, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...) __xxLogger.Log(xx::LogLevels::ERROR, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
#define LOG_TRACE(...) __xxLogger.Log(xx::LogLevels::TRACE, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...) __xxLogger.Log(xx::LogLevels::DEBUG, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
