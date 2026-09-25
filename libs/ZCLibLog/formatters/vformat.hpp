// Licensed under the Apache License, Version 2.0

//
// Created by TingIA on 2026/4/4.
//

#ifndef ZCLIBLOG_FORMATTERS_VFORMAT_HPP
#define ZCLIBLOG_FORMATTERS_VFORMAT_HPP

#include "basic_formatter.hpp"
#include "format_apis/traditional.hpp"
#include <format>
#include <chrono>

// NOLINTNEXTLINE
namespace ZCLibLog::formatters {
    /**
     * @struct vformat
     * @brief 基于C++20的"std::vformat"+"std::format"的动态格式化接口
     * @author TingIAAI
     */
    struct vformat : format_apis::traditional  {
        template <typename... Args>
        static std::string do_format(FLogPack pack, const std::string_view fmt, Args&&... args) {
            std::string f_msg;
            if (sizeof...(args) == 0) {
                f_msg = fmt;
            }
            else {
                try {
                    f_msg = std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...));
                } catch (const std::format_error& e) {
                    return e.what();
                }
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


#endif // ZCLIBLOG_FORMATTERS_VFORMAT_HPP
