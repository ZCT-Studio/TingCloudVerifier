// Copyright 2026 ZCT-Studio
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

//
// Created by wanjiangzhi on 2026/9/25.
//

#ifndef TCV_COMMON_LOGGER_HPP
#define TCV_COMMON_LOGGER_HPP

#pragma push_macro("ALL")
#pragma push_macro("ERROR")
#pragma push_macro("NO_ERROR")
#pragma push_macro("FATAL")
#pragma push_macro("DELETE")
#include "constants.hpp"
#ifdef ALL
#undef ALL
#endif
#ifdef ERROR
#undef ERROR
#endif
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#ifdef FATAL
#undef FATAL
#endif
#ifdef DELETE
#undef DELETE
#endif

#include <cctype>
#include <format>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include "ZCLibLog/logger_sync.hpp"
#include "ZCLibLog/logger_async.hpp"
#include "common/formatters.hpp"
#include "common/executors.hpp"

namespace tcv {
    using LoggerSyncT = ZCLibLog::LoggerSync<logging::stdcxx20>;
    using LoggerAsyncT = ZCLibLog::LoggerAsync<logging::stdcxx20>;

    inline ZCLibLog::LogLevel logLevelFromString(std::string_view s) {
        std::string up;
        up.reserve(s.size());
        for (char c : s) {
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                up.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
            }
        }
        if (up == "ALL") return ZCLibLog::LogLevel::ALL;
        else if (up == "TRACE") return ZCLibLog::LogLevel::TRACE;
        else if (up == "DEBUG") return ZCLibLog::LogLevel::DEBUG;
        else if (up == "INFO") return ZCLibLog::LogLevel::INFO;
        else if (up == "WARN") return ZCLibLog::LogLevel::WARN;
        else if (up == "ERROR") return ZCLibLog::LogLevel::ERROR;
        else if (up == "FATAL") return ZCLibLog::LogLevel::FATAL;
        else if (up == "OFF") return ZCLibLog::LogLevel::OFF;
        return ZCLibLog::LogLevel::INFO;
    }

    using LoggerVariant = std::variant<LoggerSyncT, LoggerAsyncT>;

    inline std::optional<LoggerVariant>& loggerStorage() {
        static std::optional<LoggerVariant> s;
        return s;
    }

    inline std::mutex& loggerMutex() {
        static std::mutex m;
        return m;
    }

    /**
     * @brief 类型擦除 wrapper——把 Sync / Async 统一成一个调用接口
     *
     * 内部持有 variant 指针，每个日志方法模板 std::visit 转发。
     */
    class Logger {
    public:
        explicit Logger(LoggerVariant* impl) : impl_(impl) {}

        template <typename... Args>
        void ALL(std::format_string<Args...> fmt, Args&&... args) {
            std::visit([&](auto& lg) { lg.ALL(fmt, std::forward<Args>(args)...); }, *impl_);
        }

        template <typename... Args>
        void TRACE(std::format_string<Args...> fmt, Args&&... args) {
            std::visit([&](auto& lg) { lg.TRACE(fmt, std::forward<Args>(args)...); }, *impl_);
        }

        template <typename... Args>
        void DEBUG(std::format_string<Args...> fmt, Args&&... args) {
            std::visit([&](auto& lg) { lg.DEBUG(fmt, std::forward<Args>(args)...); }, *impl_);
        }

        template <typename... Args>
        void INFO(std::format_string<Args...> fmt, Args&&... args) {
            std::visit([&](auto& lg) { lg.INFO(fmt, std::forward<Args>(args)...); }, *impl_);
        }

        template <typename... Args>
        void WARN(std::format_string<Args...> fmt, Args&&... args) {
            std::visit([&](auto& lg) { lg.WARN(fmt, std::forward<Args>(args)...); }, *impl_);
        }

        template <typename... Args>
        void ERROR(std::format_string<Args...> fmt, Args&&... args) {
            std::visit([&](auto& lg) { lg.ERROR(fmt, std::forward<Args>(args)...); }, *impl_);
        }

        template <typename... Args>
        void FATAL(std::format_string<Args...> fmt, Args&&... args) {
            std::visit([&](auto& lg) { lg.FATAL(fmt, std::forward<Args>(args)...); }, *impl_);
        }

        [[nodiscard]] const std::string& name() const {
            return std::visit([](auto& lg) -> const std::string& { return lg.name(); }, *impl_);
        }

        ZCLibLog::LogLevelCfg& config() {
            return std::visit([](auto& lg) -> ZCLibLog::LogLevelCfg& { return lg.config(); }, *impl_);
        }

        [[nodiscard]] bool hasExecutor() const {
            return std::visit([](auto& lg) { return lg.has_executor(); }, *impl_);
        }

        size_t bindExecutor(const ZCLibLog::executor& ex) {
            return std::visit([&](auto& lg) { return lg.bind_executor(ex); }, *impl_);
        }

    private:
        LoggerVariant* impl_;
    };

    inline void initLogger(const std::string& level, const std::string& file_path, const bool async = true) {
        std::lock_guard lk(loggerMutex());

        const auto lv = logLevelFromString(level);
        ZCLibLog::LogLevelCfg level_cfg(lv, ZCLibLog::LogLevel::OFF);

        auto ex = ZCLibLog::executor::make<tcv::logging::file_and_console>(file_path, false);
        std::initializer_list executors{std::move(ex)};

        if (async) {
            loggerStorage().emplace(std::in_place_index<1>, tcv::constants::PROJECT_NAME, executors, level_cfg);
        } else {
            loggerStorage().emplace(std::in_place_index<0>, tcv::constants::PROJECT_NAME, executors, level_cfg);
        }
    }

    inline Logger logger() {
        std::lock_guard lk(loggerMutex());
        auto& s = loggerStorage();
        if (!s) {
            s.emplace(
                std::in_place_index<0>,
                tcv::constants::PROJECT_NAME,
                std::initializer_list{
                    ZCLibLog::executor::make<tcv::logging::file_and_console>(std::string{}, true)
                }
            );
        }
        return Logger(&*s);
    }
} // namespace tcv

#endif // TCV_COMMON_LOGGER_HPP
