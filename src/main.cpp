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

namespace {
    void ensureDirectory(const std::string& path) {
        try { std::filesystem::create_directories(path); } catch (...) {}
    }

    void bootstrapDatabase() {
        const auto& cfg = tcv::AppConfig::instance();
        if (cfg.database().type != "sqlite") {
            tcv::logger().FATAL("Non-SQLite backend not implemented yet");
            std::exit(1);
        }
        auto p = std::filesystem::path(cfg.database().sqlite_path);
        if (p.has_parent_path()) ensureDirectory(p.parent_path().string());

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
} // namespace

int main(const int argc, char** argv) {
    try {
        std::string config_path = "config/config.yaml";
        if (argc > 1) config_path = argv[1];

        tcv::logger().INFO("Server \"{}\" is running, version {}", tcv::constants::PROJECT_NAME, tcv::constants::PROJECT_VERSION);
        tcv::logger().INFO("Config path: \"{}\"", std::filesystem::absolute(config_path).string());

        if (!std::filesystem::exists(config_path)) {
            tcv::logger().WARN("Config file not found, falling back to defaults");
        } else {
            tcv::logger().INFO("Loading config...");
            tcv::AppConfig::instance().load(config_path);
        }

        auto& cfg = tcv::AppConfig::instance();
        tcv::initLogger(cfg.logging().level, cfg.logging().file_path, cfg.logging().async);
        tcv::logger().INFO("Logger initialized from config");

        tcv::logger().INFO("Bootstrapping database...");
        bootstrapDatabase();

        tcv::logger().INFO("Registering routes...");
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
           .setDocumentRoot("./web")
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
