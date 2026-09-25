// Licensed under the Apache License, Version 2.0

//
// Created by TingIA on 2026/4/4.
//

#ifndef ZCLIBLOG_EXECUTORS_CFWRITE_HPP
#define ZCLIBLOG_EXECUTORS_CFWRITE_HPP

#include "basic_executor.hpp"
#include <cstdio>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        /**
         * @struct cfwrite
         * @brief C语言"cfwrite"通用执行器
         * @warning 请确保执行器存在时传入的FILE*没有被销毁
         * @author TingIAAI
         */
        struct cfwrite : executor_api {
            explicit cfwrite(FILE*& f, const bool flush = false) : f(f), flush(flush) {}
            void do_execute(ELString msg, ELogLevel) override {
                if (f) {
                    fwrite(msg.data(), 1, msg.size(), f);
                    fwrite("\n", 1, 1, f);
                    if (flush) fflush(f);
                }
            }
        private:
            FILE*& f;
            const bool flush;
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_CFWRITE_HPP
