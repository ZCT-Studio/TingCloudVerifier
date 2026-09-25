// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/11.
//

#ifndef ZCLIBLOG_FORMAT_APIS_TP_ABSL_STRFORMAT_HPP
#define ZCLIBLOG_FORMAT_APIS_TP_ABSL_STRFORMAT_HPP

#include "basic_format_api.hpp"

#include <absl/strings/str_format.h>

namespace ZCLibLog {
    namespace format_apis {
        /**
         * @struct absl_str_format
         * @brief absl format API
         * @note 使用absl::FormatSpec<Args...>作为fmt
         */
        struct absl_str_format : format_api {
            ZCLibLog_Format_API(absl_str_format);
        };
    }
    template <typename Logger>
    struct LogTag<format_apis::absl_str_format, Logger> : LogTagBase<Logger> {
        using LogTagBase<Logger>::LogTagBase;

        /**
         * @brief 输出日志
         * @tparam Args 格式化可变参数类型（自动推导）
         * @param fmt 格式字符串（必须为字面量）
         * @param args 格式化可变参数
         */
        template <typename... Args>
        void operator()(const absl::FormatSpec<Args...>& fmt, const Args&... args) const {
            if (!this->check_executable()) return;
            const std::string Formatted = Logger::m_formatter::do_format(
                this->get_log_pack(),
                fmt,
                args...
            );
            this->m_logger->execute(Formatted, this->level());
        }
    };
}

#endif //ZCLIBLOG_FORMAT_APIS_TP_ABSL_STRFORMAT_HPP
