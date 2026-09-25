// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_LOGGER_SHORTCUTS_HPP
#define ZCLIBLOG_LOGGER_SHORTCUTS_HPP

#include "logger_sync.hpp"
#include "logger_async.hpp"
#include "executors/cstdio.hpp"
#include "inside/logger_constants.hpp"

#define ZCLibLog_LOG_ALL(...) DefaultLoggerSync().ALL(__VA_ARGS__)
#define ZCLibLog_LOG_TRACE(...) DefaultLoggerSync().TRACE(__VA_ARGS__)
#define ZCLibLog_LOG_DEBUG(...) DefaultLoggerSync().DEBUG(__VA_ARGS__)
#define ZCLibLog_LOG_INFO(...) DefaultLoggerSync().INFO(__VA_ARGS__)
#define ZCLibLog_LOG_WARN(...) DefaultLoggerSync().WARN(__VA_ARGS__)
#define ZCLibLog_LOG_ERROR(...) DefaultLoggerSync().ERROR(__VA_ARGS__)
#define ZCLibLog_LOG_FATAL(...) DefaultLoggerSync().FATAL(__VA_ARGS__)

#define ZCLibLog_ALOG_ALL(...) DefaultLoggerAsync().ALL(__VA_ARGS__)
#define ZCLibLog_ALOG_TRACE(...) DefaultLoggerAsync().TRACE(__VA_ARGS__)
#define ZCLibLog_ALOG_DEBUG(...) DefaultLoggerAsync().DEBUG(__VA_ARGS__)
#define ZCLibLog_ALOG_INFO(...) DefaultLoggerAsync().INFO(__VA_ARGS__)
#define ZCLibLog_ALOG_WARN(...) DefaultLoggerAsync().WARN(__VA_ARGS__)
#define ZCLibLog_ALOG_ERROR(...) DefaultLoggerAsync().ERROR(__VA_ARGS__)
#define ZCLibLog_ALOG_FATAL(...) DefaultLoggerAsync().FATAL(__VA_ARGS__)

namespace ZCLibLog {
    namespace shortcuts  {
        inline LoggerSync<>& DefaultLoggerSync() {
            static LoggerSync<> logger{
                Constants::ProjectName,
                {
                    executor::make<executors::cstdio>()
                }
            };
            return logger;
        }
        inline LoggerAsync<>& DefaultLoggerAsync() {
            static LoggerAsync<> logger{
                Constants::ProjectName,
                {
                    executor::make<executors::cstdio>()
                }
            };
            return logger;
        }
    }
}

#endif //ZCLIBLOG_LOGGER_SHORTCUTS_HPP
