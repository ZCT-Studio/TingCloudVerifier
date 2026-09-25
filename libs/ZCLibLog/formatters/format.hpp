// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/5.
//

#ifndef ZCLIBLOG_FORMATTERS_FORMAT_HPP
#define ZCLIBLOG_FORMATTERS_FORMAT_HPP

#include "basic_formatter.hpp"
#include "format_apis/stdcxx20format.hpp"
#include <format>
#include <chrono>

// NOLINTNEXTLINE
namespace ZCLibLog::formatters {
    /**
     * @struct format
     * @brief 基于C++20的"std::format"的编译期格式化接口
     * @author wanjiangzhi
     */
    struct format : format_apis::stdcxx20format {
        template <typename... Args>
        static std::string do_format(FLogPack pack, std::format_string<Args...>&& fmt, Args&&... args) {
            std::string f_msg;
            if (sizeof...(args) == 0) {
                f_msg = fmt.get();
            }
            else {
                f_msg = std::format(fmt, std::forward<Args>(args)...);
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

            return std::format(
                "{:%Y-%m-%d %H:%M:%S} [{}] {} {}",
                std::chrono::time_point_cast<std::chrono::milliseconds>(tp),
                *pack.name,
                level_str,
                f_msg
            );
        }
    };
}


#endif // ZCLIBLOG_FORMATTERS_FORMAT_HPP
