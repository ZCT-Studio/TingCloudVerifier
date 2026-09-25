// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_FORMAT_APIS_TRADITIONAL_HPP
#define ZCLIBLOG_FORMAT_APIS_TRADITIONAL_HPP

#include "basic_format_api.hpp"

namespace ZCLibLog {
    namespace format_apis {
        /**
         * @struct traditional
         * @brief 传统格式化API
         * @note 使用常规类型(const char*, std::string_view, std::string)fmt
         */
        struct traditional : format_api {
            ZCLibLog_Format_API(traditional);
        };
    }
    template <typename Logger>
    struct LogTag<format_apis::traditional, Logger> : LogTagBase<Logger> {
        using LogTagBase<Logger>::LogTagBase;

        /**
         * @brief 输出日志
         * @tparam Fmt 格式字符串的类型（自动推导）
         * @tparam Args 格式化可变参数类型（自动推导）
         * @param fmt 格式字符串
         * @param args 格式化可变参数
         */
        template <typename Fmt, typename... Args>
        void operator()(Fmt&& fmt, Args&&... args) const {
            if (!this->check_executable()) return;
            std::string Formatted = Logger::m_formatter::do_format(
                this->get_log_pack(),
                std::forward<Fmt>(fmt),
                std::forward<Args>(args)...
            );
            this->m_logger->execute(Formatted, this->level());
        }
    };
}

#endif //ZCLIBLOG_FORMAT_APIS_TRADITIONAL_HPP
