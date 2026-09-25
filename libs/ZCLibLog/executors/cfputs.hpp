// Licensed under the Apache License, Version 2.0

//
// Created by TingIA on 2026/4/4.
//

#ifndef ZCLIBLOG_EXECUTORS_CFPUTS_HPP
#define ZCLIBLOG_EXECUTORS_CFPUTS_HPP

#include "basic_executor.hpp"
#include <cstdio>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        /**
         * @struct cfputs
         * @brief C语言"fputs"通用执行器
         * @warning 请确保执行器存在时传入的FILE*没有被销毁
         * @author TingIAAI
         */
        struct cfputs : executor_api {
            explicit cfputs(FILE*& f, const bool flush = true) : f(f), flush(flush) {}
            void do_execute(ELString msg, ELogLevel) override {
                if (f) {
                    fputs(msg.c_str(), f);
                    fputs("\n", f);
                    if (flush) fflush(f);
                }
            }
        private:
            FILE*& f;
            const bool flush;
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_CFPUTS_HPP
