// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/7.
//

#ifndef ZCLIBLOG_EXECUTORS_LAMBDA_HPP
#define ZCLIBLOG_EXECUTORS_LAMBDA_HPP

#include "basic_executor.hpp"
#include <functional>

namespace ZCLibLog {
    namespace executors {
        /**
         * @struct lambda
         * @brief C++ lambda函数thunk执行器
         * @author wanjiangzhi
         */
        struct lambda : executor_api {
            using constructible = std::function<void(ELString, ELogLevel)>;
            explicit lambda(constructible constructed) : constructed(std::move(constructed)) {}
            void do_execute(ELString msg, ELogLevel lv) override {
                constructed(msg, lv);
            }
        private:
            constructible constructed;
        };
    }

    /**
     * @brief 辅助创建lambda thunk执行器
     * @param constructed 可构造的函数
     * @return 包装执行器
     */
    inline executor lambda_wrapper(executors::lambda::constructible constructed) {
        return executor::make<executors::lambda>(std::move(constructed));
    }
}

#endif //ZCLIBLOG_EXECUTORS_LAMBDA_HPP
