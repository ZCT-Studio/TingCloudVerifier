// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_EXECUTORS_IOSTREAM_HPP
#define ZCLIBLOG_EXECUTORS_IOSTREAM_HPP

#include "basic_executor.hpp"
#include <iostream>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        /**
         * @struct iostream
         * @brief C++"iostream"标准输出执行器
         * @author wanjiangzhi
         */
        struct iostream : executor_api {
            void do_execute(ELString msg, ELogLevel lv) override {
                if (lv >= LogLevel::ERROR)
                    std::cerr << msg << std::endl;
                else
                    std::cout << msg << std::endl;
            }
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_IOSTREAM_HPP
