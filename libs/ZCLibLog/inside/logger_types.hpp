//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_LOGGER_TYPES_HPP
#define ZCLIBLOG_LOGGER_TYPES_HPP

#include <string>
#include <chrono>
#include <memory>
#include <utility>
#include <type_traits>

#include "logger_macros.h"

namespace ZCLibLog {
    /// @brief 日志等级的基础类型
    using LogLevelBase = uint16_t;

    /**
     * @brief 注册日志等级
     * @note 可自己在OFF前，ALL后添加
     */
    #define ZCLIBLOG_HELPER_LEVELS(Register) \
        Register(ALL,   0) \
        Register(TRACE, 1) \
        Register(DEBUG, 2) \
        Register(INFO,  3) \
        Register(WARN,  4) \
        Register(ERROR, 5) \
        Register(FATAL, 6) \
        Register(OFF, std::numeric_limits<LogLevelBase>::max())

    /// @brief 一些日志等级
    enum class LogLevel : LogLevelBase {
        #define ZCLIBLOG_HELPER_ENUM_CASE(name, value) name = value,
        ZCLIBLOG_HELPER_LEVELS(ZCLIBLOG_HELPER_ENUM_CASE)
        #undef ZCLIBLOG_HELPER_ENUM_CASE
    };

    /**
     * @brief 获取日志等级的文本字符串
     * @param level 等级
     * @return 日志等级的文本字符串
     */
    inline const char* LogLevelToString(const LogLevel level) {
        switch (level) {
                #define ZCLIBLOG_HELPER_ENUM_CASE(name, value) case LogLevel::name: return #name;
            ZCLIBLOG_HELPER_LEVELS(ZCLIBLOG_HELPER_ENUM_CASE)
                #undef ZCLIBLOG_HELPER_ENUM_CASE
        }
        return "UNKNOWN";
    }

    #undef ZCLIBLOG_HELPER_LEVELS

    /**
     * @struct LogPack
     * @brief Log格式化的参数包
     */
    struct LogPack {
        /// @brief 该日志的名字
        const std::string* name = {};
        /// @brief 该日志的时间戳
        uint64_t time = {};
        /// @brief 该日志的级别
        LogLevel level = {};
    };

    /**
     * @struct LogLevelCfg
     * @brief Log等级范围
     */
    struct LogLevelCfg {
        /// @brief 配置的最低等级
        LogLevel min_level;
        /// @brief 配置的最高等级
        LogLevel max_level;

        // ReSharper disable once CppNonExplicitConvertingConstructor
        /**
         * @brief 单等级构造
         * @param level 日志的最低等级
         */
        LogLevelCfg(const LogLevel level = {}) : min_level(level),
                                                 max_level(LogLevel::OFF) {}

        /**
         * @brief 等级范围构造
         * @param min_level 日志的最低等级
         * @param max_level 日志的最高等级
         */
        LogLevelCfg(const LogLevel min_level, const LogLevel max_level) : min_level(min_level),
                                                                          max_level(max_level) {}
    };

    /// @brief 执行器接收的等级
    using ELogLevel = const LogLevel;
    /// @brief 执行器接受的字符串
    using ELString = const std::string&;

    /**
     * @struct executor_api
     * @brief 执行器的基类抽象类
     */
    struct executor_api {
        /// @brief 自定义执行器需要重载的执行函数
        virtual void do_execute(ELString, ELogLevel) = 0;
        /// @brief 自定义执行器可能重载的析构函数
        virtual ~executor_api() = default;

    protected:
        /// @brief 简化写法
        using ELString = ELString;
        /// @brief 简化写法
        using ELogLevel = ELogLevel;
        /// @brief 简化写法
        using LogLevel = LogLevel;
    };

    /**
     * @brief 判断是否是基于executor api
     * @tparam Executor 需要判断的Executor类
     * @note 可和std::is_base_of一样使用::value成员
     */
    template <typename Executor>
    using is_executor_api = std::is_base_of<executor_api, Executor>;

    /**
     * @class executor
     * @brief 执行器包装类
     */
    class executor {
    public:
        using base_ptr_type = std::shared_ptr<executor_api>;
        using inside_type = base_ptr_type::element_type;

    private:
        base_ptr_type executor_ptr;

    public:
        executor() = default;
        // ReSharper disable once CppNonExplicitConvertingConstructor
        /**
         * @brief 通过裸指针构造
         * @param executor_ptr 执行器指针
         * @warning
         * 裸指针传入后禁止delete!\n
         * 禁止取右值对象的地址传入
         */
        executor(inside_type* executor_ptr) : executor_ptr(executor_ptr) {}

        /**
         * @brief 通过工厂函数构造
         * @tparam Executor 执行器类
         * @tparam Args 执行器类构造参数列表（自动推导）
         * @param args 执行器类构造参数
         * @return 包装执行器
         */
        template <typename Executor, typename... Args>
        static executor make(Args&&... args) {
            static_assert(is_executor_api<Executor>::value, "Executor must be a real executor");

            executor made;
            made.executor_ptr = std::move(std::make_shared<Executor>(std::forward<Args>(args)...));

            return std::move(made);
        }

        /// @brief 获取内部执行器的指针
        ZCLibLog_NODISCARD inside_type* get() const noexcept {
            return executor_ptr.get();
        }

        /// @brief 获取执行器是否独占状态
        ZCLibLog_NODISCARD bool unique() const noexcept {
            // return executor_ptr.unique();
            // unique成员函数在MSVC下不存在？不知为何通不过多平台构建测试
            return executor_ptr.use_count() == 1;
            // 这里使用等价逻辑代替
        }

        /// @brief 获取执行器使用计数
        ZCLibLog_NODISCARD long use_count() const noexcept {
            return executor_ptr.use_count();
        }

        /// @brief 访问内部执行器内部成员
        inside_type* operator->() const noexcept {
            return executor_ptr.operator->();
        }

        /// @brief 获取内部执行器对象
        inside_type& operator*() const noexcept {
            return executor_ptr.operator*();
        }

        // ReSharper disable once CppNonExplicitConversionOperator
        /// @brief 判断执行器是否不为空
        operator bool() const noexcept {
            return executor_ptr.get() != nullptr;
        }
    };

    /// @brief 格式化接受的数据包
    using FLogPack = const LogPack&;
    /// @brief 格格式化输出的字符串
    using FLString = std::string;

    /**
     * @struct format_api
     * @brief 定义了一些简化写法
     */
    struct format_api {
    protected:
        /// @brief 简化写法
        using FLogPack = FLogPack;
        /// @brief 简化写法
        using FLString = FLString;
        /// @brief 简化写法
        using LogLevel = LogLevel;
    };

    /**
     * @struct is_format_api
     * @brief 判断是否是基于format api
     * @tparam FormatAPI 需要判断的FormatAPI类
     * @note 可和使用::value成员
     */
    template <typename FormatAPI>
    struct is_format_api
    : std::integral_constant<
        bool,
        std::is_base_of<format_api, FormatAPI>::value &&
        !std::is_same<FormatAPI, format_api>::value
    > {};

    /**
     * @struct is_formatter
     * @brief 判断是否是是基于format api的formatter
     * @tparam Formatter 需要判断的Formatter类
     * @note 可使用::value成员
     */
    template <typename Formatter>
    struct is_formatter
    : std::integral_constant<
        bool,
        std::is_base_of<format_api, typename Formatter::Format_API>::value &&
        !std::is_same<typename Formatter::Format_API, format_api>::value &&
        std::is_base_of<typename Formatter::Format_API, Formatter>::value &&
        !std::is_same<typename Formatter::Format_API, Formatter>::value
    > {};

    /**
     * @class LogTagBase
     * @brief 根基Tag类，构造无用，仅继承
     * @tparam Logger 日志器
     */
    template <typename Logger>
    class LogTagBase {
    protected:
        /// @brief 获取当前的日志信息包
        ZCLibLog_NODISCARD LogPack get_log_pack() const {
            const auto now = std::chrono::system_clock::now();
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()
            ).count();

            LogPack p;
            p.name = &m_logger->name();
            p.level = m_level;
            p.time = ms;

            return p;
        }

        /// @brief 检查是否可执行
        ZCLibLog_NODISCARD bool check_executable() const {
            if (
                m_logger->has_executor() &&
                m_logger->be_executable(level())
            )
                return true;
            return {};
        }

        const Logger* const m_logger{};
        const LogLevel m_level{};
    public:
        /// @brief 获取当前Tag的等级
        ZCLibLog_NODISCARD const LogLevel& level() const noexcept {
            return m_level;
        }

        /**
         * @brief 构造Tag
         * @param logger 日志器指针（引用非拥有）
         * @param level 当前Tag的日志等级
         */
        LogTagBase(const Logger* const logger, const LogLevel level) : m_logger(logger),
                                                                    m_level(level) {}
    };

    /**
     * @class LogTag
     * @brief 泛型模板，无法特化
     * @tparam FormatterAPI 格式化API
     * @tparam Logger
     */
    template <typename FormatterAPI, typename Logger>
    struct LogTag : LogTagBase<Logger> {
        using LogTagBase<Logger>::LogTagBase;
        static_assert(
            is_format_api<FormatterAPI>::value,
            "Formatter must be format_api"
        );
    };
}

#endif //ZCLIBLOG_LOGGER_TYPES_HPP
