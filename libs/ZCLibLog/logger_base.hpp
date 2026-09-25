// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_LOGGER_BASE_HPP
#define ZCLIBLOG_LOGGER_BASE_HPP

#include "inside/logger_precompile.hpp" // NOLINT

#include <algorithm>
#include <stdexcept>
#include <vector>

// ReSharper disable CppUnusedIncludeDirective

#include "inside/logger_types.hpp"

#include "inside/logger_constants.hpp"

#include "logger_configurations.h"
#if ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF
#include "formatters/csnprintf.hpp"
#endif

// ReSharper enable CppUnusedIncludeDirective

#if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_MUTEX
#ifndef ZCLibLog_MUTEX
#if ZCLibLog_CPP >= 17
#include <shared_mutex>
#define ZCLibLog_MUTEX std::shared_mutex
#else
#define ZCLibLog_MUTEX std::mutex
#endif
#endif
#endif

namespace ZCLibLog {
    /**
     * @class BaseLogger
     * @brief 基本日志器，无执行
     * @tparam Formatter 绑定的格式化器
     */
    template <
        typename Formatter
        #if ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF
        = formatters::csnprintf
        #endif
    >
    class BaseLogger {
        using Self = BaseLogger;
        // ReSharper disable CppUseTypeTraitAlias
        static_assert(is_formatter<Formatter>::value,
                  "Formatter must be format_api");
        // ReSharper enable CppUseTypeTraitAlias
    protected:
        #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_MUTEX
        mutable ZCLibLog_MUTEX m_mutex;
        #endif

        using executor_pair = std::pair<size_t, executor>;

        std::string m_name;
        std::vector<executor_pair> m_executors;
        size_t m_nextID{};

        LogLevelCfg m_config;
    public:
        using m_formatter = Formatter;

        /// @brief 获取日志器的名字
        ZCLibLog_NODISCARD const std::string& name() const noexcept {
            return m_name;
        }

        /// @brief 获取并可修改日志器的等级配置
        ZCLibLog_NODISCARD LogLevelCfg& config() noexcept {
            return m_config;
        }

        /**
         * @brief 检查等级是否可执行
         * @param level 要检查的等级
         * @return 是否可执行
         */
        ZCLibLog_NODISCARD bool be_executable(const LogLevel level) const noexcept {
            return m_config.min_level <= level && level <= m_config.max_level;
        }

        /// @brief 判断是否有执行器
        ZCLibLog_NODISCARD bool has_executor() const {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            return !m_executors.empty();
        }

        /**
         * @brief 绑定执行器
         * @param ex 执行器常量引用
         * @return 执行器在日志器的id
         */
        size_t bind_executor(const executor& ex) {
            if (!ex) throw std::invalid_argument("executor is nullptr");
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            m_executors.emplace_back(m_nextID, ex);
            return m_nextID++;
        }

        /**
         * @brief 解绑执行器
         * @param id 执行器在日志器的id
         */
        void debind_executor(size_t id) {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            m_executors.erase(
                std::remove_if(
                    m_executors.begin(),
                    m_executors.end(),
                    [id](const executor_pair& p) { return p.first == id; }
                ),
                m_executors.end()
            );
        }

        /// @brief 清空所有执行器
        void clear_executors() {
            #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_MUTEX
            std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
            #endif
            m_executors.clear();
            m_nextID = {};
        }

        // ReSharper disable once CppNonExplicitConvertingConstructor
        /**
         * @brief 构造同步日志器
         * @param name 日志器名字
         * @param executor_ptrs 日志器预绑定执行器
         * @param config 日志器等级配置
         */
        BaseLogger(
            std::string name,
            const std::initializer_list<executor>& executor_ptrs = {},
            const LogLevelCfg config = {}
        ) : m_name(std::move(name)),
            m_config(config) {
            for (const auto& executor_ptr : executor_ptrs) {
                bind_executor(executor_ptr);
            }
        }
    };

}

#endif //ZCLIBLOG_LOGGER_BASE_HPP
