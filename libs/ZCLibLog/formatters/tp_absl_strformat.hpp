// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/11.
//

#ifndef ZCLIBLOG_FORMATTERS_TP_ABSL_STRFORMAT_HPP
#define ZCLIBLOG_FORMATTERS_TP_ABSL_STRFORMAT_HPP

#include "basic_formatter.hpp"
#include "format_apis/tp_absl_strformat.hpp"
#include <absl/strings/str_format.h>
#include <iomanip>
#include <ctime>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace formatters {
        /**
         * @struct absl_str_format
         * @brief 基于absl::StrFormat
         * @author wanjiangzhi
         */
        struct absl_str_format : format_apis::absl_str_format  {
            template <typename... Args>
            static std::string do_format(FLogPack pack, const absl::FormatSpec<Args...>& fmt, const Args&... args) {
                std::string f_msg;
                try {
                    f_msg = absl::StrFormat(fmt, args...);
                } catch (const std::exception& e) {
                    return e.what();
                }

                const auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(pack.time));

                const char* level_str;
                switch (pack.level) {
                    case LogLevel::TRACE: level_str = "[TRACE]";
                        break;
                    case LogLevel::DEBUG: level_str = "[DEBUG]";
                        break;
                    case LogLevel::INFO: level_str = "[INFO]";
                        break;
                    case LogLevel::WARN: level_str = "[WARN]";
                        break;
                    case LogLevel::ERROR: level_str = "[ERROR]";
                        break;
                    case LogLevel::FATAL: level_str = "[FATAL]";
                        break;
                    default: level_str = "[OUT]";
                        break;
                }

                std::time_t tt = std::chrono::system_clock::to_time_t(tp);
                std::tm tm{};

                #if defined(_WIN32)
                localtime_s(&tm, &tt);
                #elif defined(__linux__) || defined(__APPLE__) || defined(__unix__)
                localtime_r(&tt, &tm);
                #else
                tm = *std::localtime(&tt);
                #endif

                std::ostringstream t_oss;
                t_oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");

                return absl::StrFormat("%s [%s] %s %s", t_oss.str(), *pack.name, level_str, f_msg);
            }
        };
    }
}

#endif //ZCLIBLOG_FORMATTERS_TP_ABSL_STRFORMAT_HPP
