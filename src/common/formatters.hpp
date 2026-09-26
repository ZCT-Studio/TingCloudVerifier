// Copyright 2026 ZCT-Studio
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

#ifndef TCV_COMMON_FORMATTERS_HPP
#define TCV_COMMON_FORMATTERS_HPP

#include <array>
#include <ctime>
#include <format>
#include <string>
#include <thread>

#include "ZCLibLog/formatters/basic_formatter.hpp"
#include "ZCLibLog/formatters/format_apis/stdcxx20format.hpp"

namespace tcv::logging {
    /**
     * @file formatters.hpp
     * @brief TingCloudVerifier 使用的 ZCLibLog formatter
     *
     * 输出格式：
     *   2026-09-26 15:22:31.802 [T1234] [INFO] [TingCloudVerifier] logger initialized
     *      时间戳(ms)            线程ID    等级   logger 名字          用户消息
     */

    struct stdcxx20 : ZCLibLog::format_apis::stdcxx20format {
        template <typename... Args>
        static std::string do_format(
            ZCLibLog::FLogPack pack,
            const std::format_string<Args...>& fmt,
            Args&&... args
        ) {
            // 用户消息
            std::string f_msg = std::format(fmt, std::forward<Args>(args)...);

            // 时间戳（毫秒精度）
            auto t = static_cast<std::time_t>(pack.time / 1000);
            const auto ms = static_cast<short>(pack.time % 1000);
            std::tm tm{};
            #if defined(_WIN32)
            localtime_s(&tm, &t);
            #elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
            localtime_r(&t, &tm);
            #else
            tm = *std::localtime(&t);
            #endif

            thread_local std::array<char, 64> time_buf;
            std::strftime(time_buf.data(), time_buf.size(), "%Y-%m-%d %H:%M:%S", &tm);

            // 线程 ID（hash 成数字，跨平台稳定）
            auto tid = std::hash<std::thread::id>{}(std::this_thread::get_id());

            // 等级名
            const char* level = "OUT";
            switch (pack.level) {
                case ZCLibLog::LogLevel::TRACE: level = "TRACE";
                    break;
                case ZCLibLog::LogLevel::DEBUG: level = "DEBUG";
                    break;
                case ZCLibLog::LogLevel::INFO: level = "INFO";
                    break;
                case ZCLibLog::LogLevel::WARN: level = "WARN";
                    break;
                case ZCLibLog::LogLevel::ERROR: level = "ERROR";
                    break;
                case ZCLibLog::LogLevel::FATAL: level = "FATAL";
                    break;
                default: break;
            }

            return std::format(
                "{}.{:03d} [T{}] [{}] [{}] {}",
                time_buf.data(),
                ms,
                tid,
                level,
                pack.name->c_str(),
                f_msg
            );
        }
    };
} // namespace tcv::logging

#endif // TCV_COMMON_FORMATTERS_HPP
