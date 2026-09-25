// Licensed under the Apache License, Version 2.0

//
// Created by TingIA on 2026/4/4.
//

#ifndef ZCLIBLOG_EXECUTORS_CPUTS_HPP
#define ZCLIBLOG_EXECUTORS_CPUTS_HPP

#include "basic_executor.hpp"
#include <cstdio>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        /**
         * @struct cputs
         * @brief C语言"puts"打印执行器
         * @author wanjiangzhi
         */
        struct cputs : executor_api {
            explicit cputs() {}
            void do_execute(ELString msg, ELogLevel) override {
                puts(msg.c_str());
            }
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_CPUTS_HPP
