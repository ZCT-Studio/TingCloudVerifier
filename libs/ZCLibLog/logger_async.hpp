// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_ASYNC_HPP
#define ZCLIBLOG_LOGGER_ASYNC_HPP

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>

#include "logger_base.hpp"

#if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
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
     * @namespace ZCLibLog::details
     * @brief 内部使用的工具
     */
    namespace details {
        /**
         * @class ThreadPool
         * @brief 内部使用的线程池
         */
        class ThreadPool {
        public:
            /**
             * @brief 构造线程池
             * @param numThreads 线程数量（默认1）
             */
            explicit ThreadPool(const size_t numThreads = 1) : stop(false) {
                for (size_t i = 0; i < numThreads; ++i) {
                    workers.emplace_back(
                        [this] {
                            while (true) {
                                std::function<void()> task;
                                {
                                    std::unique_lock<std::mutex> lock(mtx);
                                    cv.wait(lock, [this] { return stop || !tasks.empty(); });
                                    if (stop && tasks.empty()) return;
                                    task = std::move(tasks.front());
                                    tasks.pop();
                                }
                                task();
                            }
                        }
                    );
                }
            }

            /// @brief 析构线程池
            ~ThreadPool() {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    stop = true;
                }
                cv.notify_all();
                for (auto& t : workers) t.join();
            }

            /**
             * @brief 提交任务到线程池
             * @param task 要提交的任务
             */
            void submit(std::function<void()> task) {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    tasks.push(std::move(task));
                }
                cv.notify_one();
            }

        private:
            std::vector<std::thread> workers;
            std::queue<std::function<void()>> tasks;
            std::mutex mtx;
            std::condition_variable cv;
            std::atomic<bool> stop;
        };
    }

    /// @brief 全局异步线程池
    static details::ThreadPool LoggerAsync_ThreadPool{ZCLIBLOG_LOGGER_CONFIGURATIONS_ASYNC_THREAD_NUM};

    /**
     * @class LoggerAsync
     * @brief 异步日志器
     * @tparam Formatter 绑定的格式化器
     */
    template <
        typename Formatter
        #if ZCLIBLOG_LOGGER_CONFIGURATIONS_DEFAULT_CSNPRINTF
        = formatters::csnprintf
        #endif
    >
    struct LoggerAsync : BaseLogger<Formatter> {
        using BaseLogger<Formatter>::BaseLogger;
        using Self = LoggerAsync;

        /**
         * @brief 调用执行器处理日志信息和等级
         * @param message 日志信息
         * @param level 日志等级
         */
        void execute(std::string& message, const LogLevel level) const {
            if (!message.empty()) {
                auto message_p = std::make_shared<std::string>(std::move(message));
                LoggerAsync_ThreadPool.submit(
                    [this, message_p, level] {
                        #if ZCLIBLOG_LOGGER_CONFIGURATIONS_LOGGER_ASYNC_MUTEX
                        std::lock_guard<ZCLibLog_MUTEX> lock(m_mutex);
                        #endif
                        // ReSharper disable once CppUseStructuredBinding
                        // ReSharper disable once CppUseElementsView
                        for (const auto& the_executor_pair : this->m_executors) {
                            the_executor_pair.second->do_execute(*message_p, level);
                        }
                    }
                );
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

#endif //ZCLIBLOG_LOGGER_ASYNC_HPP
