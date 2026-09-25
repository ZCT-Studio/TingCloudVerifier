// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_SYNC_HPP
#define ZCLIBLOG_LOGGER_SYNC_HPP

#include "logger_base.hpp"

namespace ZCLibLog {
    /**
     * @class LoggerSync
     * @brief 同步日志器
     * @tparam Formatter 绑定的格式化器
     */
    template <
        typename Formatter
        #if ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF
        = formatters::csnprintf
        #endif
    >
    struct LoggerSync : BaseLogger<Formatter> {
        using BaseLogger<Formatter>::BaseLogger;
        using Self = LoggerSync;

        void execute(const std::string& message, const LogLevel level) const {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            if (!message.empty()) {
                // ReSharper disable once CppUseElementsView
                // ReSharper disable once CppUseStructuredBinding
                for (const auto& the_executor_pair : this->m_executors) {
                    the_executor_pair.second->do_execute(message, level);
                }
            }
        }

        using Tag = LogTag<typename Formatter::Format_API, Self>;

        /// @brief ALL级别Tag
        Tag ALL{this, LogLevel::ALL};
        /// @brief TRACE级别Tag
        Tag TRACE{this, LogLevel::TRACE};
        /// @brief DEBUG级别Tag
        Tag DEBUG{this, LogLevel::DEBUG};
        /// @brief INFO级别Tag
        Tag INFO{this, LogLevel::INFO};
        /// @brief WARN级别Tag
        Tag WARN{this, LogLevel::WARN};
        /// @brief ERROR级别Tag
        Tag ERROR{this, LogLevel::ERROR};
        /// @brief FATAL级别Tag
        Tag FATAL{this, LogLevel::FATAL};
    };
}

#endif //ZCLIBLOG_LOGGER_SYNC_HPP
