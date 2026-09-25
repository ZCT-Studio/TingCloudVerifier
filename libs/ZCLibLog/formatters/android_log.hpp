// Licensed under the Apache License, Version 2.0

//
// Created by TingIA on 2026/4/4.
//

#ifndef ZCLIBLOG_FORMATTERS_ANDROID_LOG_HPP
#define ZCLIBLOG_FORMATTERS_ANDROID_LOG_HPP

#include "basic_formatter.hpp"
#include "format_apis/traditional.hpp"
#include <ctime>
#include <cstdio>
#include <array>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace formatters {
        /**
         * @struct android_log
         * @brief 为"__android_log_write"设计的基于C语言"snprintf"的格式化接口
         * @note 和android_log executor配套使用获得最佳体验
         * @author TingIAAI
         */
        struct android_log : format_apis::traditional  {
            template<typename... Args>
            static std::string do_format(FLogPack pack, const char* fmt, Args&&... args) {
                thread_local std::array<char, 4096> buffer;
                int len{};

                std::string f_msg;
                if (sizeof...(args) == 0) {
                    f_msg = fmt;
                }
                else {
                    len = std::snprintf(buffer.data(), buffer.size(), fmt, std::forward<Args>(args)...);
                    if (len < 0) return {};
                    if (len >= static_cast<int>(buffer.size())) len = buffer.size() - 1;
                    f_msg = std::string(buffer.data(), len);
                }
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

                thread_local std::array<char, 64> time;
                std::strftime(time.data(), time.size(), "%Y-%m-%d %H:%M:%S", &tm);
                thread_local std::array<char, 80> ms_time;
                std::snprintf(ms_time.data(), ms_time.size(), "%s.%03d", time.data(), ms);

                len = std::snprintf(buffer.data(), buffer.size(),
                                    "%s [%s] %s\n",
                                    ms_time.data(), pack.name->c_str(), f_msg.c_str());
                if (len < 0) return {};
                if (len >= static_cast<int>(buffer.size())) len = buffer.size() - 1;
                return {buffer.data(), buffer.data() + len};
            }
        };
    }
}

#endif // ZCLIBLOG_FORMATTERS_ANDROID_LOG_HPP
