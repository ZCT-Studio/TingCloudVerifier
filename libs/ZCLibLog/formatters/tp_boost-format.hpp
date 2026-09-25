// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_FORMATTERS_TP_BOOST_FORMAT_HPP
#define ZCLIBLOG_FORMATTERS_TP_BOOST_FORMAT_HPP

#include "format_apis/traditional.hpp"
#include <boost/format.hpp>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace ZCLibLog {
    namespace formatters {
        /**
         * @struct boost_format
         * @brief 基于boost::format
         * @author wanjiangzhi
         */
        struct boost_format : format_apis::traditional  {
            template <typename... Args>
            static std::string do_format(FLogPack pack, const std::string& fmt, Args&&... args) {
                std::string f_msg;
                if (sizeof...(args) == 0) {
                    f_msg = fmt;
                }
                else {
                    boost::format f(fmt);
                    try {
                        apply(f, std::forward<Args>(args)...);
                    } catch (const std::exception& e) {
                        return e.what();
                    }
                    f_msg = f.str();
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

                return (boost::format("%1% [%2%] %3% %4%")
                    % t_oss.str()
                    % *pack.name
                    % level_str
                    % f_msg
                ).str();
            }
        private:
            template <typename T>
            static void apply(boost::format& f, T&& t) {
                f % std::forward<T>(t);
            }

            template <typename T, typename... Args>
            static void apply(boost::format& f, T&& t, Args&&... args) {
                f % std::forward<T>(t);
                apply(f, std::forward<Args>(args)...);
            }
        };
    }
}

#endif //ZCLIBLOG_FORMATTERS_TP_BOOST_FORMAT_HPP
