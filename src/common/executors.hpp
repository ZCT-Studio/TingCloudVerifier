// Copyright 2026 ZCT-Studio
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

#ifndef TCV_COMMON_EXECUTORS_HPP
#define TCV_COMMON_EXECUTORS_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ZCLibLog/inside/logger_types.hpp"

namespace tcv::logging {
    struct file_and_console : ZCLibLog::executor_api {
        explicit file_and_console(std::string log_path, bool console_only = false)
            : log_path_(std::move(log_path)),
              console_only_(console_only) {
            if (console_only_) return;

            auto parent = std::filesystem::path(log_path_).parent_path();
            if (!parent.empty()) {
                std::filesystem::create_directories(parent);
            }

            ofs_.open(log_path_, std::ios::app | std::ios::binary);
            if (!ofs_.is_open()) {
                throw std::runtime_error("cannot open log file: " + log_path_);
            }
        }

        ~file_and_console() override {
            if (ofs_.is_open()) {
                ofs_.flush();
                ofs_.close();
            }
        }

        file_and_console(const file_and_console&) = delete;
        file_and_console& operator=(const file_and_console&) = delete;
        file_and_console(file_and_console&&) noexcept = default;
        file_and_console& operator=(file_and_console&&) noexcept = default;

        void do_execute(ELString msg, ELogLevel lv) override {
            if (ofs_.is_open()) {
                ofs_.write(msg.data(), static_cast<std::streamsize>(msg.size()));
                ofs_.put('\n');
                ofs_.flush();
            }

            std::ostream& console = (lv >= ZCLibLog::LogLevel::ERROR) ? std::cerr : std::cout;
            console.write(msg.data(), static_cast<std::streamsize>(msg.size()));
            console.put('\n');
            console.flush();
        }

    private:
        std::string log_path_;
        bool console_only_;
        std::ofstream ofs_;
    };
} // namespace tcv::logging

#endif // TCV_COMMON_EXECUTORS_HPP
