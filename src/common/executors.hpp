// Copyright 2026 ZCT-Studio
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

//
// Created by wanjiangzhi on 2026/9/25.
//

#ifndef TCV_COMMON_EXECUTORS_HPP
#define TCV_COMMON_EXECUTORS_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "ZCLibLog/inside/logger_types.hpp"

namespace tcv::logging {
    struct tcv_executor : ZCLibLog::executor_api {
        explicit tcv_executor(std::string log_path, bool console_only = false)
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

        ~tcv_executor() override {
            if (ofs_.is_open()) {
                ofs_.flush();
                ofs_.close();
            }
        }

        tcv_executor(const tcv_executor&) = delete;
        tcv_executor& operator=(const tcv_executor&) = delete;
        tcv_executor(tcv_executor&&) noexcept = default;
        tcv_executor& operator=(tcv_executor&&) noexcept = default;

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
}

#endif // TCV_COMMON_EXECUTORS_HPP
