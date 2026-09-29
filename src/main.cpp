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

#include "binary-c-array/config.yaml.hpp"

namespace {
    void ensureDirectory(const std::filesystem::path& path) {
        try { std::filesystem::create_directories(path); } catch (...) {}
    }

    void bootstrapDatabase() {
        const auto& cfg = tcv::AppConfig::instance();
        if (cfg.database().type != "sqlite") {
            tcv::logger().FATAL("Non-SQLite backend not implemented yet");
            std::exit(1);
        }
        const auto p = std::filesystem::path(cfg.database().sqlite_path);
        if (p.has_parent_path()) ensureDirectory(p.parent_path());

        auto& db = tcv::db::Database::instance();
        db.open(cfg.database().sqlite_path);

        tcv::logger().INFO("Database opened: \"{}\"", std::filesystem::absolute(cfg.database().sqlite_path).string());

        std::string migrations = "migrations";
        if (!std::filesystem::exists(migrations)) {
            migrations = (std::filesystem::current_path() / "migrations").string();
        }
        try {
            tcv::db::MigrationRunner::runAll(migrations);
            tcv::logger().INFO("Migrations applied from: {}", migrations);
        } catch (const std::exception& e) {
            tcv::logger().ERROR("Migrations failed: {}", e.what());
        }
    }

    void extractDefaultConfig(const std::filesystem::path& path) {
        ensureDirectory(path.parent_path());

        std::ofstream file(path, std::ios::binary);

        if (!file) {
            throw std::runtime_error("Failed to create config.yaml");
        }

        file.write(
            reinterpret_cast<const char*>(config_yaml),
            config_yaml_len
        );

        if (!file) {
            throw std::runtime_error("Failed to write config.yaml");
        }
    }
}

int main(const int argc, char** argv) {
    try {
        const std::string config_path = argc > 1 ? argv[1] : "config.yaml";

        tcv::logger().INFO("Config path: \"{}\"", std::filesystem::absolute(config_path).string());

        if (!std::filesystem::exists(config_path)) {
            tcv::logger().WARN("Config file not found, extract default config");

            extractDefaultConfig(config_path);

            tcv::logger().WARN("Default configuration generated successfully. Please modify it and restart the application");

            return 0;
        }

        tcv::logger().INFO("Server \"{}\" is running, version {}", tcv::constants::PROJECT_NAME, tcv::constants::PROJECT_VERSION);

        tcv::logger().INFO("Loading config...");
        tcv::AppConfig::instance().load(config_path);

        const auto& cfg = tcv::AppConfig::instance();
        tcv::initLogger(cfg.logging().level, cfg.logging().file_path, cfg.logging().async);
        tcv::logger().INFO("Logger initialized from config");

        tcv::logger().INFO("Bootstrapping database...");
        bootstrapDatabase();

        tcv::logger().INFO("Registering routes...");
        tcv::middleware::registerAllMiddlewares(drogon::app());
        tcv::api::registerAllV1Routes(drogon::app());

        auto nthreads = std::thread::hardware_concurrency();
        if (nthreads == 0) nthreads = 4;

        tcv::logger().INFO(
            "Listening {}:{}, {} threads",
            cfg.server().host,
            cfg.server().port,
            nthreads
        );

        drogon::app()
           .addListener(cfg.server().host, cfg.server().port)
           .setThreadNum(nthreads)
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
