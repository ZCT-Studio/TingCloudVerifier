// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/10.
//

#ifndef ZCLIBLOG_BASIC_FORMATTER_HPP
#define ZCLIBLOG_BASIC_FORMATTER_HPP

// file_wrapper
#include "../inside/logger_types.hpp"

namespace ZCLibLog {
    /**
     * @namespace ZCLibLog::formatters
     * @brief 预定的一些formatter
     */
    namespace formatters {}
}

/**
 * @brief 在类/结构体内public作用域声明格式化API
 * @param id API的ID，和类/结构体标识符应相同
 */
#define ZCLibLog_Format_API(id) typedef id Format_API;

#endif //ZCLIBLOG_BASIC_FORMATTER_HPP
