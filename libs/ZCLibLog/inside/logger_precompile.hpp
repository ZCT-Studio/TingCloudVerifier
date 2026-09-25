// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/5.
//

#ifndef ZCLIBLOG_LOGGER_PRECOMPILE_HPP
#define ZCLIBLOG_LOGGER_PRECOMPILE_HPP

#include "logger_macros.h"

#if ZCLibLog_CPP < 11
    #error "ZCLibLog need CPP Standard is less than 11"
#endif

/**
 * @namespace ZCLibLog
 * @brief ZCLibLog的命名空间
 */
namespace ZCLibLog {}

#endif //ZCLIBLOG_LOGGER_PRECOMPILE_HPP
