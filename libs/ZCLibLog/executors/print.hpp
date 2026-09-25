// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_EXECUTORS_PRINT_HPP
#define ZCLIBLOG_EXECUTORS_PRINT_HPP

#include "basic_executor.hpp"
#include <print>

// NOLINTNEXTLINE
namespace ZCLibLog::executors {
    /**
     * @struct println
     * @brief C++"println"打印执行器
     * @author wanjiangzhi
     */
    struct println : executor_api {
        explicit println() {}
        void do_execute(ELString msg, ELogLevel) override {
            std::println("{}", msg);
        }
    };

    /**
     * @struct print
     * @brief C++"print"打印执行器
     * @author wanjiangzhi
     */
    struct print : executor_api {
        explicit print() {}
        void do_execute(ELString msg, ELogLevel) override {
            std::print("{}\n", msg);
        }
    };
}

#endif //ZCLIBLOG_EXECUTORS_PRINT_HPP
