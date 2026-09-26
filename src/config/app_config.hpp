// Copyright 2026 ZCT-Studio
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

//
// Created by wanjiangzhi on 2026/9/25.
//

#ifndef TCV_CONFIG_APP_CONFIG
#define TCV_CONFIG_APP_CONFIG

#include <exception>
#include <string>
#include <yaml-cpp/yaml.h>
#include "common/logger.hpp"

namespace tcv {
    struct DatabaseConfig {
        std::string type = "sqlite";
        std::string sqlite_path = "./tcv.db";
    };

    struct SecurityConfig {
        std::string admin_security_level = "LAN"; // LOCAL/LAN/PUBLIC
        int replay_window_seconds = 60;
        int session_ttl_seconds = 1800;
        int temp_token_ttl_seconds = 3600;
        std::string password_hasher = "argon2id";
    };

    struct RateLimitConfig {
        bool enabled = true;
        int ip_per_sec = 100;
        int app_per_sec = 500;
        int license_per_sec = 10;
        int login_fail_lock_threshold = 5;
        int login_fail_lock_seconds = 300;
    };

    struct LoggingConfig {
        std::string level = "INFO";
        bool async = true;
        std::string file_path = "./logs/tcv.log";
    };

    struct ServerConfig {
        std::string host = "0.0.0.0";
        int port = 10211;
    };

    struct LicenseConfig {
        bool timer_paused = false;
    };

    class AppConfig {
    public:
        static AppConfig& instance() {
            static AppConfig inst;
            return inst;
        }

        bool load(const std::string& path) {
            try {
                YAML::Node root = YAML::LoadFile(path);

                if (auto n = root["server"]) {
                    server_.host = n["host"].as<std::string>(server_.host);
                    server_.port = n["port"].as<int>(server_.port);
                }

                if (auto n = root["database"]) {
                    db_.type = n["type"].as<std::string>(db_.type);
                    if (db_.type == "sqlite" && n["sqlite_path"])
                        db_.sqlite_path = n["sqlite_path"].as<std::string>();
                }

                if (auto n = root["security"]) {
                    sec_.admin_security_level = n["admin_security_level"].as<std::string>(sec_.admin_security_level);
                    sec_.replay_window_seconds = n["replay_window_seconds"].as<int>(sec_.replay_window_seconds);
                    sec_.session_ttl_seconds = n["session_ttl_seconds"].as<int>(sec_.session_ttl_seconds);
                    sec_.temp_token_ttl_seconds = n["temp_token_ttl_seconds"].as<int>(sec_.temp_token_ttl_seconds);
                    sec_.password_hasher = n["password_hasher"].as<std::string>(sec_.password_hasher);
                }

                if (auto n = root["rate_limit"]) {
                    rl_.enabled = n["enabled"].as<bool>(rl_.enabled);
                    rl_.ip_per_sec = n["ip_per_sec"].as<int>(rl_.ip_per_sec);
                    rl_.app_per_sec = n["app_per_sec"].as<int>(rl_.app_per_sec);
                    rl_.license_per_sec = n["license_per_sec"].as<int>(rl_.license_per_sec);
                    rl_.login_fail_lock_threshold = n["login_fail_lock_threshold"].as<int>(rl_.login_fail_lock_threshold);
                    rl_.login_fail_lock_seconds = n["login_fail_lock_seconds"].as<int>(rl_.login_fail_lock_seconds);
                }

                if (auto n = root["logging"]) {
                    log_.level = n["level"].as<std::string>(log_.level);
                    log_.async = n["async"].as<bool>(log_.async);
                    log_.file_path = n["file_path"].as<std::string>(log_.file_path);
                }

                if (auto n = root["license"]) {
                    lic_.timer_paused = n["timer_paused"].as<bool>(lic_.timer_paused);
                }

                return true;
            } catch (const std::exception& e) {
                tcv::logger().ERROR("Failed to load config from {}: {}", path, e.what());
                return false;
            }
        }

        AppConfig() = default;

        [[nodiscard]] const ServerConfig& server() const { return server_; }
        [[nodiscard]] const DatabaseConfig& database() const { return db_; }
        [[nodiscard]] const SecurityConfig& security() const { return sec_; }
        [[nodiscard]] const RateLimitConfig& rateLimit() const { return rl_; }
        [[nodiscard]] const LoggingConfig& logging() const { return log_; }
        [[nodiscard]] const LicenseConfig& license() const { return lic_; }

    private:
        ServerConfig server_;
        DatabaseConfig db_;
        SecurityConfig sec_;
        RateLimitConfig rl_;
        LoggingConfig log_;
        LicenseConfig lic_;
    };
} // namespace tcv

#endif // TCV_CONFIG_APP_CONFIG
