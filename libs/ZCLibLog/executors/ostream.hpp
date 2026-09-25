// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/4.
//

#ifndef ZCLIBLOG_EXECUTORS_OSTREAM_HPP
#define ZCLIBLOG_EXECUTORS_OSTREAM_HPP

#include "basic_executor.hpp"
#include <ostream>

// NOLINTNEXTLINE
namespace ZCLibLog {
    namespace executors {
        /**
         * @struct ostream
         * @brief C++"ostream"通用执行器
         * @warning 请确保执行器存在时传入的ostream没有被销毁
         * @author wanjiangzhi
         */
        struct ostream : executor_api {
            explicit ostream(std::ostream& os) : os(os) {}
            void do_execute(ELString msg, ELogLevel) override {
                os << msg << std::endl;
            }
        private:
            std::ostream& os;
        };
    }
}

#endif //ZCLIBLOG_EXECUTORS_OSTREAM_HPP
