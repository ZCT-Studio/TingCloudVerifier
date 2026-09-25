// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/3/30.
//

#ifndef ZCLIBLOG_EXECUTORS_CSTDOUT_HPP
#define ZCLIBLOG_EXECUTORS_CSTDOUT_HPP

#include "basic_executor.hpp"
#include <cstdio>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        /**
         * @struct cstdio
         * @brief C语言"stdout/stderr"标准输出执行器
         * @author wanjiangzhi
         */
        struct cstdio : executor_api {
            explicit cstdio(const bool enable_fwrite = false) : enable_fwrite(enable_fwrite) {}
            void do_execute(ELString msg, ELogLevel lv) override {
                if (enable_fwrite) {
                    if (lv >= LogLevel::ERROR) {
                        fwrite(msg.data(), 1, msg.size(), stderr);
                        fwrite("\n", 1, 1, stderr);
                    } else {
                        fwrite(msg.data(), 1, msg.size(), stdout);
                        fwrite("\n", 1, 1, stdout);
                    }
                } else {
                    if (lv >= LogLevel::ERROR) {
                        fputs(msg.c_str(), stderr);
                        fputs("\n", stderr);
                    } else {
                        fputs(msg.c_str(), stdout);
                        fputs("\n", stdout);
                    }
                }
            }
        private:
            const bool enable_fwrite;
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_CSTDOUT_HPP
