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

#include <exception>
#include <filesystem>
#include <string>
#include <thread>
#include <drogon/drogon.h>
#include "constants.hpp"
#include "common/logger.hpp"
#include "config/app_config.hpp"
#include "database/database.hpp"
#include "api/v1/register_all.hpp"
#include "middleware/middlewares.hpp"

namespace {
    void ensureDirectory(const std::filesystem::path& path) {
        try { std::filesystem::create_directories(path); } catch (...) {}
    }

    bool bootstrapDatabase() {
        try {
            const auto& cfg = tcv::AppConfig::instance();
            auto& db = tcv::db::Database::instance();
            db.open(cfg.database());

            std::string migrations = "migrations";
            if (!std::filesystem::exists(migrations)) {
                migrations = (std::filesystem::current_path() / "migrations").string();
            }
            tcv::db::MigrationRunner::runAll(migrations);
            tcv::logger().INFO("Migrations applied from: {}", migrations);
            return true;
        } catch (const std::exception& e) {
            tcv::logger().ERROR("Database bootstrap FAILED: {}", e.what());
            return false;
        }
    }
}

int main(const int argc, char** argv) {
    try {
        tcv::logger().INFO("TCV Server \"{}\" is starting, version {}", tcv::constants::PROJECT_NAME, tcv::constants::PROJECT_VERSION);

        const std::filesystem::path config_path = argc > 1 ? argv[1] : "config/config.yaml";

        tcv::logger().INFO("Config path: \"{}\"", std::filesystem::absolute(config_path).string());

        if (!std::filesystem::exists(config_path)) {
            tcv::logger().FATAL("Config file not found");
            return 1;
        }

        tcv::logger().INFO("Loading config...");
        tcv::AppConfig::instance().load(config_path.string());

        const auto& cfg = tcv::AppConfig::instance();
        tcv::initLogger(cfg.logging().level, cfg.logging().file_path, cfg.logging().async);
        tcv::logger().INFO("Logger initialized from config");

        tcv::logger().INFO("Bootstrapping database...");
        if (!bootstrapDatabase()) {
            tcv::logger().FATAL("Cannot start: database bootstrap failed. Check TCV was compiled with the requested backend.");
            return 1;
        }

        tcv::logger().INFO("Registering routes...");
        tcv::middleware::registerAllMiddlewares(drogon::app());
        tcv::api::registerAllV1Routes(drogon::app());

        auto nthreads = std::thread::hardware_concurrency();
        if (nthreads == 0) nthreads = 4;

        auto& svr = drogon::app();
        const auto& srv_cfg = cfg.server();

        if (!srv_cfg.ipv4_host.empty()) {
            tcv::logger().INFO("Listening IPv4 {}:{}, {} threads", srv_cfg.ipv4_host, srv_cfg.port, nthreads);
            svr.addListener(srv_cfg.ipv4_host, srv_cfg.port);
        }
        if (!srv_cfg.ipv6_host.empty()) {
            tcv::logger().INFO("Listening IPv6 [{}]:{}, {} threads", srv_cfg.ipv6_host, srv_cfg.port, nthreads);
            svr.addListener(srv_cfg.ipv6_host, srv_cfg.port);
        }

        svr.setThreadNum(nthreads)
           .enableServerHeader(false)
           .setDocumentRoot("./tcv_web")
           .run();
    } catch (const std::exception& e) {
        tcv::logger().FATAL("FATAL exception: {}", e.what());
        return 1;
    } catch (...) {
        tcv::logger().FATAL("FATAL unknown exception");
        return 2;
    }
    return 0;
}
