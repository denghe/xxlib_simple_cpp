#pragma once
#include "xx_logger.h"

struct MyLogger : xx::Logger {
    explicit MyLogger(size_t const &capMB = 8, char const* const& cfgName = "xxlog.config") : xx::Logger(capMB, cfgName) {
        cfg.outputConsole = false;
    }
};
inline MyLogger myLogger;

#define LOG_INFO(...) myLogger.Log(xx::LogLevels::INFO, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...) myLogger.Log(xx::LogLevels::WARN, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...) myLogger.Log(xx::LogLevels::ERROR, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
#define LOG_TRACE(...) myLogger.Log(xx::LogLevels::TRACE, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
#define LOG_DEBUG(...) myLogger.Log(xx::LogLevels::DEBUG, __LINE__, xx::CutPath(__FILE__), __FUNCTION__, __VA_ARGS__)
