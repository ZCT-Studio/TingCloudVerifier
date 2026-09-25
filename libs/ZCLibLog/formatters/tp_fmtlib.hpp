// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_FORMATTERS_TP_FMTLIB_HPP
#define ZCLIBLOG_FORMATTERS_TP_FMTLIB_HPP

#include "basic_formatter.hpp"
#include "format_apis/tp_fmtlib.hpp"
#include "format_apis/traditional.hpp"
#include <fmt/format.h>
#include <fmt/chrono.h>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace formatters {
        /**
         * @namespace ZCLibLog::formatters::fmtlib
         * @brief 基于第三方的fmtlib
         * @author wanjiangzhi
         */
        namespace fmtlib {
            /**
             * @struct format
             * @brief 基于第三方的fmtlib
             * @author wanjiangzhi
             */
            struct format : format_apis::fmtlib {
                template <typename... Args>
                static std::string do_format(FLogPack pack, const fmt::format_string<Args...>& fmt, Args&&... args) {
                    std::string f_msg;
                    if (sizeof...(args) == 0) {
                        f_msg = fmt.get().data();
                    } else {
                        try {
                            f_msg = fmt::format(fmt, std::forward<Args>(args)...);
                        } catch (fmt::format_error& e) {
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

                    return fmt::format(
                        "{:%Y-%m-%d %H:%M:%S} [{}] {} {}",
                        std::chrono::time_point_cast<std::chrono::milliseconds>(tp),
                        *pack.name,
                        level_str,
                        f_msg
                    );
                }
            };
            /**
             * @struct vformat
             * @brief 基于第三方的fmtlib
             * @author wanjiangzhi
             */
            struct vformat : format_apis::traditional {
                template <typename... Args>
                static std::string do_format(FLogPack pack, const fmt::string_view fmt, Args&&... args) {
                    std::string f_msg;
                    if (sizeof...(args) == 0) {
                        f_msg = fmt.data();
                    } else {
                        try {
                            f_msg = fmt::vformat(fmt, fmt::make_format_args(stdargs...));
                        } catch (fmt::format_error& e) {
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

                    return fmt::format(
                        "{:%Y-%m-%d %H:%M:%S} [{}] {} {}",
                        std::chrono::time_point_cast<std::chrono::milliseconds>(tp),
                        *pack.name,
                        level_str,
                        f_msg
                    );
                }
            };
        }
    }
}

#endif //ZCLIBLOG_FORMATTERS_TP_FMTLIB_HPP
