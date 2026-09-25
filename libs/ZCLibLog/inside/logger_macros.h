// Licensed under the Apache License, Version 2.0

//
// Created by wanjiangzhi on 2026/4/4.
//

#ifndef ZCLIBLOG_LOGGER_MACROS_H
#define ZCLIBLOG_LOGGER_MACROS_H

#ifdef _MSVC_LANG // MSVC下__cplusplus可能不准，故用_MSVC_LANG
    #define ZCLibLog_CPP_VER _MSVC_LANG
#else
    #define ZCLibLog_CPP_VER __cplusplus
#endif

#if ZCLibLog_CPP_VER >= 202302L
    #define ZCLibLog_CPP_YEAR 2023
    #define ZCLibLog_CPP 23
#elif ZCLibLog_CPP_VER >= 202002L
    #define ZCLibLog_CPP_YEAR 2020
    #define ZCLibLog_CPP 20
#elif ZCLibLog_CPP_VER >= 201703L
    #define ZCLibLog_CPP_YEAR 2017
    #define ZCLibLog_CPP 17
#elif ZCLibLog_CPP_VER >= 201402L
    #define ZCLibLog_CPP_YEAR 2014
    #define ZCLibLog_CPP 14
#elif ZCLibLog_CPP_VER >= 201103L
    #define ZCLibLog_CPP_YEAR 2011
    #define ZCLibLog_CPP 11
#else
    #define ZCLibLog_CPP_YEAR 2003
    #define ZCLibLog_CPP 03
#endif

#if ZCLibLog_CPP >= 14
    #define ZCLibLog_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(__GNUC__) || defined(__clang__)
    #define ZCLibLog_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(_MSC_VER)
    #define ZCLibLog_DEPRECATED(msg) __declspec(deprecated(msg))
#else
    #define ZCLibLog_DEPRECATED(msg)
#endif

#if ZCLibLog_CPP >= 17 // C++17启用nodiscard特性，避免警告
    #define ZCLibLog_NODISCARD [[nodiscard]]
#else
    #define ZCLibLog_NODISCARD
#endif

#if ZCLibLog_CPP >= 20
    #define ZCLibLog_HAS_STD_FORMAT // C++20支持std::format
    #define ZCLibLog_HAS_CONSTRAINTS // C++20支持契约
#endif

#endif //ZCLIBLOG_LOGGER_MACROS_H
