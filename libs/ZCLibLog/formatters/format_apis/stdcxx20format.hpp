// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_FORMAT_APIS_STDCXX20FORMAT_HPP
#define ZCLIBLOG_FORMAT_APIS_STDCXX20FORMAT_HPP

#include "basic_format_api.hpp"

#include <format>

namespace ZCLibLog {
    namespace format_apis {
        /**
         * @struct stdcxx20format
         * @brief C++20 format API
         * @note 使用std::format_string<Args...>作为fmt
         */
        struct stdcxx20format : format_api {
            ZCLibLog_Format_API(stdcxx20format);
        };
    }
    template <typename Logger>
    struct LogTag<format_apis::stdcxx20format, Logger> : LogTagBase<Logger> {
        using LogTagBase<Logger>::LogTagBase;

        /**
         * @brief 输出日志
         * @tparam Args 格式化可变参数类型（自动推导）
         * @param fmt 格式字符串（必须为字面量）
         * @param args 格式化可变参数
         */
        template <typename... Args>
        void operator()(const std::format_string<Args...>& fmt, Args&&... args) const {
            if (!this->check_executable()) return;
            const std::string Formatted = Logger::m_formatter::do_format(
                this->get_log_pack(),
                fmt,
                std::forward<Args>(args)...
            );
            this->m_logger->execute(Formatted, this->level());
        }
    };
}

#endif //ZCLIBLOG_FORMAT_APIS_STDCXX20FORMAT_HPP
