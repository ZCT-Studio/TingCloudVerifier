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

#ifndef TCV_DATABASE_DATABASE
#define TCV_DATABASE_DATABASE

#include <exception>
#include <filesystem>
#include <utility>
#include <fstream>
#include <optional>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <shared_mutex>
#include <algorithm>
#include <string>
#include <ctime>
#include <mutex>
#include <vector>
#include <sqlite3.h>
#include "common/logger.hpp"


namespace tcv::db {
    class SQLException : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    using Row = std::vector<std::pair<std::string, std::string>>;
    using ResultSet = std::vector<Row>;

    class Database {
    public:
        static Database& instance() {
            static Database inst;
            return inst;
        }

        void open(const std::string& path) {
            std::unique_lock lock(mtx_);
            if (db_) sqlite3_close(db_);
            constexpr int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |
                                  SQLITE_OPEN_FULLMUTEX;
            if (sqlite3_open_v2(path.c_str(), &db_, flags, nullptr) != SQLITE_OK) {
                auto msg = db_ ? sqlite3_errmsg(db_) : "unknown";
                if (db_) sqlite3_close(db_);
                db_ = nullptr;
                throw SQLException(std::format("sqlite open [{}] failed: {}", path, msg));
            }
            // execLocked 避死
            execLocked("PRAGMA journal_mode=DELETE;");
            execLocked("PRAGMA foreign_keys=ON;");
            execLocked("PRAGMA synchronous=NORMAL;");
        }

        sqlite3* handle() const { return db_; }

        int64_t lastInsertId() const { return sqlite3_last_insert_rowid(db_); }
        int rowsChanged() const { return sqlite3_changes(db_); }

        void exec(const std::string& sql) const {
            std::unique_lock lock(mtx_);
            char* err = nullptr;
            if (const int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err); rc != SQLITE_OK) {
                std::string msg = err ? err : "unknown";
                sqlite3_free(err);
                throw SQLException(std::format("sqlite exec: {}", msg));
            }
        }

        template <typename... Args>
        void execParams(const std::string& sql, Args&&... args) {
            std::unique_lock lock(mtx_);
            sqlite3_stmt* stmt = compileLocked(sql);
            bindAll(stmt, std::forward<Args>(args)...);
            stepFinalize(stmt);
        }

        ResultSet query(const std::string& sql) const {
            std::unique_lock lock(mtx_);
            sqlite3_stmt* stmt = compileLocked(sql);
            return fetchAll(stmt);
        }

        template <typename... Args>
        ResultSet queryParams(const std::string& sql, Args&&... args) {
            std::unique_lock lock(mtx_);
            sqlite3_stmt* stmt = compileLocked(sql);
            bindAll(stmt, std::forward<Args>(args)...);
            return fetchAll(stmt);
        }

        template <typename... Args>
        std::optional<std::string> queryScalar(const std::string& sql, Args&&... args) {
            std::unique_lock lock(mtx_);
            sqlite3_stmt* stmt = compileLocked(sql);
            bindAll(stmt, std::forward<Args>(args)...);
            if (const int rc = sqlite3_step(stmt); rc == SQLITE_ROW && sqlite3_column_count(stmt) > 0) {
                const auto ptr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                std::string val = ptr ? ptr : "";
                sqlite3_finalize(stmt);
                return val;
            }
            sqlite3_finalize(stmt);
            return std::nullopt;
        }

        template <typename F>
        bool transaction(F&& fn) {
            std::unique_lock lock(mtx_);
            execLocked("BEGIN IMMEDIATE");
            bool committed = false;
            try {
                if (fn()) {
                    execLocked("COMMIT");
                    committed = true;
                } else {
                    execLocked("ROLLBACK");
                }
            } catch (...) {
                execLocked("ROLLBACK");
                throw;
            }
            return committed;
        }

        void execSqlFile(const std::string& path) const {
            std::ifstream f(path);
            if (!f.is_open()) throw SQLException(std::format("cannot open sql file: {}", path));
            std::ostringstream ss;
            ss << f.rdbuf();
            exec(ss.str());
        }

        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;
    private:
        Database() = default;
        ~Database() { if (db_) sqlite3_close(db_); }

        sqlite3* db_ = nullptr;
        mutable std::recursive_mutex mtx_;

        sqlite3_stmt* compileLocked(const std::string& sql) const {
            sqlite3_stmt* stmt = nullptr;
            if (const int rc = sqlite3_prepare_v2(db_, sql.c_str(), static_cast<int>(sql.size()), &stmt, nullptr); rc != SQLITE_OK) {
                throw SQLException(std::format("sqlite prepare: {} | sql: {}", sqlite3_errmsg(db_), sql));
            }
            return stmt;
        }

        void stepFinalize(sqlite3_stmt* stmt) const {
            const int rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            if (rc != SQLITE_DONE) {
                throw SQLException(std::format("sqlite step: {}", sqlite3_errmsg(db_)));
            }
        }

        void execLocked(const std::string& sql) const {
            char* err = nullptr;
            if (const int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err); rc != SQLITE_OK) {
                std::string msg = err ? err : "unknown";
                sqlite3_free(err);
                throw SQLException(std::format("sqlite exec: {}", msg));
            }
        }

        ResultSet fetchAll(sqlite3_stmt* stmt) const {
            ResultSet out;
            const int cols = sqlite3_column_count(stmt);
            std::vector<std::string> col_names(static_cast<size_t>(cols));
            for (int i = 0; i < cols; ++i)
                col_names[i] = sqlite3_column_name(stmt, i);

            int rc;
            while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
                Row row;
                row.reserve(cols);
                for (int i = 0; i < cols; ++i) {
                    auto v = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    row.emplace_back(col_names[i], v ? v : "");
                }
                out.push_back(std::move(row));
            }
            sqlite3_finalize(stmt);
            if (rc != SQLITE_DONE) {
                throw SQLException(std::format("sqlite fetch: {}", sqlite3_errmsg(db_)));
            }
            return out;
        }

        template <typename T, typename... Rest>
        void bindAll(sqlite3_stmt* stmt, T&& v, Rest&&... rest) {
            bindOne(stmt, sqlite3_bind_parameter_count(stmt) - static_cast<int>(sizeof...(rest)), std::forward<T>(v));
            bindAll(stmt, std::forward<Rest>(rest)...);
        }

        static void bindAll(sqlite3_stmt* /*stmt*/) {}

        static void bindOne(sqlite3_stmt* stmt, const int idx, const std::string_view v) {
            const auto sv = std::string(v);
            sqlite3_bind_text(stmt, idx, sv.data(), static_cast<int>(sv.size()), SQLITE_TRANSIENT);
        }

        static void bindOne(sqlite3_stmt* stmt, const int idx, const std::string& v) {
            sqlite3_bind_text(stmt, idx, v.data(), static_cast<int>(v.size()), SQLITE_TRANSIENT);
        }

        static void bindOne(sqlite3_stmt* stmt, const int idx, const char* v) {
            sqlite3_bind_text(stmt, idx, v, -1, SQLITE_TRANSIENT);
        }

        static void bindOne(sqlite3_stmt* stmt, const int idx, const int64_t v) {
            sqlite3_bind_int64(stmt, idx, v);
        }

        static void bindOne(sqlite3_stmt* stmt, const int idx, const int v) {
            sqlite3_bind_int(stmt, idx, v);
        }

        static void bindOne(sqlite3_stmt* stmt, const int idx, std::nullptr_t) {
            sqlite3_bind_null(stmt, idx);
        }

        static void bindOne(sqlite3_stmt* stmt, const int idx, const std::optional<std::string>& v) {
            if (v) bindOne(stmt, idx, *v);
            else sqlite3_bind_null(stmt, idx);
        }

        static void bindOne(sqlite3_stmt* stmt, const int idx, const std::optional<int64_t>& v) {
            if (v) bindOne(stmt, idx, *v);
            else sqlite3_bind_null(stmt, idx);
        }
    };

    class MigrationRunner {
    public:
        static void runAll(const std::string& migrationsDir) {
            auto& db = Database::instance();
            try {
                db.exec(
                    "CREATE TABLE IF NOT EXISTS schema_migrations ("
                    "version INTEGER PRIMARY KEY, "
                    "name TEXT NOT NULL, "
                    "applied_at INTEGER NOT NULL)"
                );
            } catch (...) {}

            std::vector<std::filesystem::path> files;
            if (std::filesystem::exists(migrationsDir)) {
                for (auto& f : std::filesystem::directory_iterator(migrationsDir)) {
                    if (f.path().extension() == ".sql") files.push_back(f.path());
                }
                std::ranges::sort(files);
            }

            for (auto& p : files) {
                std::string f_name = p.filename().string();
                int version = 0;
                if (auto pos = f_name.find('_'); pos != std::string::npos) {
                    try { version = std::stoi(f_name.substr(0, pos)); } catch (...) {}
                }

                auto applied = db.queryScalar(
                    "SELECT version FROM schema_migrations WHERE version = ?",
                    version
                );
                if (applied.has_value()) continue;

                try {
                    db.execSqlFile(p.string());
                    int64_t now = std::time(nullptr);
                    db.execParams(
                        "INSERT INTO schema_migrations(version, name, applied_at) VALUES (?, ?, ?)",
                        version,
                        f_name,
                        now
                    );
                    tcv::logger().INFO("Migration applied: {}", f_name);
                } catch (const std::exception& e) {
                    tcv::logger().ERROR("Migration FAILED {}: {}", f_name, e.what());
                    throw;
                }
            }
        }
    };
} // namespace tcv::db

#endif // TCV_DATABASE_DATABASE
