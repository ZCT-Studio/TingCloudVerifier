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

#ifndef TCV_REPOSITORIES_USER_APP_REPO
#define TCV_REPOSITORIES_USER_APP_REPO

#include <utility>
#include <optional>
#include <string>
#include <ctime>
#include <vector>
#include <cstdint>
#include <algorithm>
#include "common/types.hpp"
#include "database/database.hpp"
#include "models/models.hpp"
#include "repositories/license_repo.hpp"  // 提供 getInt/getStr/getOptInt/getOptStr

namespace tcv::repo {
    namespace AdminRepo {
        inline std::optional<tcv::models::Admin> findByUsername(const std::string& u) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM admins WHERE username = ?",
                u
            );
            if (rows.empty()) return std::nullopt;
            auto r = rows[0];
            tcv::models::Admin a;
            a.id = getInt(r, "id");
            a.username = getStr(r, "username");
            a.password_hash = getStr(r, "password_hash");
            a.salt = getStr(r, "salt");
            a.display_name = getStr(r, "display_name");
            a.created_at = getInt(r, "created_at");
            a.updated_at = getInt(r, "updated_at");
            a.last_login_at = getOptInt(r, "last_login_at");
            a.status = static_cast<int>(getInt(r, "status"));
            return a;
        }

        inline std::optional<tcv::models::Admin> findById(int64_t id) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM admins WHERE id = ?",
                id
            );
            if (rows.empty()) return std::nullopt;
            auto r = rows[0];
            tcv::models::Admin a;
            a.id = getInt(r, "id");
            a.username = getStr(r, "username");
            a.password_hash = getStr(r, "password_hash");
            a.salt = getStr(r, "salt");
            a.display_name = getStr(r, "display_name");
            a.created_at = getInt(r, "created_at");
            a.updated_at = getInt(r, "updated_at");
            a.last_login_at = getOptInt(r, "last_login_at");
            a.status = static_cast<int>(getInt(r, "status"));
            return a;
        }

        inline int64_t insert(
            const std::string& username,
            const std::string& pw_hash,
            const std::string& salt,
            const std::string& display,
            int64_t now
        ) {
            tcv::db::Database::instance().execParams(
                "INSERT INTO admins(username, password_hash, salt, display_name, created_at, updated_at, status)"
                " VALUES (?, ?, ?, ?, ?, ?, 1)",
                username,
                pw_hash,
                salt,
                display,
                now,
                now
            );
            return tcv::db::Database::instance().lastInsertId();
        }

        inline void updateLastLogin(int64_t id, int64_t now) {
            tcv::db::Database::instance().execParams(
                "UPDATE admins SET last_login_at = ? WHERE id = ?",
                now,
                id
            );
        }
    } // namespace AdminRepo

    // ============================================================
    // UserRepository
    // ============================================================
    namespace UserRepo {
        inline std::optional<tcv::models::User> findByUsername(const std::string& u) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM users WHERE username = ?",
                u
            );
            if (rows.empty()) return std::nullopt;
            auto r = rows[0];
            tcv::models::User user;
            user.id = getInt(r, "id");
            user.username = getStr(r, "username");
            user.password_hash = getStr(r, "password_hash");
            user.salt = getStr(r, "salt");
            user.display_name = getStr(r, "display_name");
            user.balance = getInt(r, "balance");
            user.expires_at = getInt(r, "expires_at");
            user.status = static_cast<int>(getInt(r, "status"));
            user.created_by = getOptInt(r, "created_by");
            user.created_at = getInt(r, "created_at");
            user.updated_at = getInt(r, "updated_at");
            return user;
        }

        inline std::optional<tcv::models::User> findById(int64_t id) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM users WHERE id = ?",
                id
            );
            if (rows.empty()) return std::nullopt;
            auto r = rows[0];
            tcv::models::User user;
            user.id = getInt(r, "id");
            user.username = getStr(r, "username");
            user.password_hash = getStr(r, "password_hash");
            user.salt = getStr(r, "salt");
            user.display_name = getStr(r, "display_name");
            user.balance = getInt(r, "balance");
            user.expires_at = getInt(r, "expires_at");
            user.status = static_cast<int>(getInt(r, "status"));
            user.created_by = getOptInt(r, "created_by");
            user.created_at = getInt(r, "created_at");
            user.updated_at = getInt(r, "updated_at");
            return user;
        }

        inline int64_t insert(
            const std::string& username,
            const std::string& pw_hash,
            const std::string& salt,
            const std::string& display,
            int64_t now,
            int64_t created_by = 0
        ) {
            tcv::db::Database::instance().execParams(
                "INSERT INTO users(username, password_hash, salt, display_name, balance, expires_at, status, created_by, created_at, updated_at)"
                " VALUES (?, ?, ?, ?, -1, -1, 1, NULLIF(?,0), ?, ?)",
                username,
                pw_hash,
                salt,
                display,
                created_by == 0 ? std::nullopt : std::optional<int64_t>(created_by),
                now,
                now
            );
            return tcv::db::Database::instance().lastInsertId();
        }

        inline void addBalance(int64_t id, int64_t delta, int64_t now) {
            tcv::db::Database::instance().execParams(
                "UPDATE users SET balance = balance + ?, updated_at = ? WHERE id = ?",
                delta,
                now,
                id
            );
        }

        inline void setBalance(int64_t id, int64_t value, int64_t now) {
            tcv::db::Database::instance().execParams(
                "UPDATE users SET balance = ?, updated_at = ? WHERE id = ?",
                value,
                now,
                id
            );
        }

        inline void setExpires(int64_t id, int64_t expires_at, int64_t now) {
            tcv::db::Database::instance().execParams(
                "UPDATE users SET expires_at = ?, updated_at = ? WHERE id = ?",
                expires_at,
                now,
                id
            );
        }

        inline int64_t atomicConsumeBalance(int64_t id, int64_t amount) {
            // 注意：调用者必须已在 transaction 中（避免 SQLite 嵌套 BEGIN）
            auto cur = tcv::db::Database::instance().queryScalar(
                "SELECT balance FROM users WHERE id = ?",
                id
            );
            if (!cur) return -1;
            int64_t b = 0;
            try { b = std::stoll(*cur); } catch (...) { return -1; }
            if (b != -1 && b < amount) return -1;
            int64_t new_balance = (b == -1) ? -1 : (b - amount);
            int64_t now = std::time(nullptr);
            tcv::db::Database::instance().execParams(
                "UPDATE users SET balance = ?, updated_at = ? WHERE id = ?",
                new_balance,
                now,
                id
            );
            return new_balance;
        }
    } // namespace UserRepo

    // ============================================================
    // AppRepository
    // ============================================================
    namespace AppRepo {
        inline std::optional<tcv::models::App> findByAppid(const std::string& appid) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM apps WHERE appid = ?",
                appid
            );
            if (rows.empty()) return std::nullopt;
            auto r = rows[0];
            tcv::models::App a;
            a.id = getInt(r, "id");
            a.owner_id = getInt(r, "owner_id");
            a.appid = getStr(r, "appid");
            a.name = getStr(r, "name");
            a.description = getStr(r, "description");
            a.secret_hash = getStr(r, "secret_hash");
            a.status = static_cast<int>(getInt(r, "status"));
            a.created_at = getInt(r, "created_at");
            a.updated_at = getInt(r, "updated_at");
            return a;
        }

        inline std::optional<tcv::models::App> findById(int64_t id) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM apps WHERE id = ?",
                id
            );
            if (rows.empty()) return std::nullopt;
            auto r = rows[0];
            tcv::models::App a;
            a.id = getInt(r, "id");
            a.owner_id = getInt(r, "owner_id");
            a.appid = getStr(r, "appid");
            a.name = getStr(r, "name");
            a.description = getStr(r, "description");
            a.secret_hash = getStr(r, "secret_hash");
            a.status = static_cast<int>(getInt(r, "status"));
            a.created_at = getInt(r, "created_at");
            a.updated_at = getInt(r, "updated_at");
            return a;
        }

        inline int64_t insert(
            int64_t owner_id,
            const std::string& appid,
            const std::string& name,
            const std::string& desc,
            const std::string& secret_hash,
            int64_t now
        ) {
            tcv::db::Database::instance().execParams(
                "INSERT INTO apps(owner_id, appid, name, description, secret_hash, status, created_at, updated_at)"
                " VALUES (?, ?, ?, ?, ?, 1, ?, ?)",
                owner_id,
                appid,
                name,
                desc,
                secret_hash,
                now,
                now
            );
            return tcv::db::Database::instance().lastInsertId();
        }

        inline void updateSecretHash(int64_t id, const std::string& new_hash, int64_t now) {
            tcv::db::Database::instance().execParams(
                "UPDATE apps SET secret_hash = ?, updated_at = ? WHERE id = ?",
                new_hash,
                now,
                id
            );
        }

        inline void setStatus(int64_t id, int status, int64_t now) {
            tcv::db::Database::instance().execParams(
                "UPDATE apps SET status = ?, updated_at = ? WHERE id = ?",
                status,
                now,
                id
            );
        }

        inline std::vector<tcv::models::App> listByOwner(int64_t owner_id) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM apps WHERE owner_id = ? ORDER BY id DESC",
                owner_id
            );
            std::vector<tcv::models::App> out;
            for (auto& r : rows) {
                tcv::models::App a;
                a.id = getInt(r, "id");
                a.owner_id = getInt(r, "owner_id");
                a.appid = getStr(r, "appid");
                a.name = getStr(r, "name");
                a.description = getStr(r, "description");
                a.secret_hash = getStr(r, "secret_hash");
                a.status = static_cast<int>(getInt(r, "status"));
                a.created_at = getInt(r, "created_at");
                a.updated_at = getInt(r, "updated_at");
                out.push_back(std::move(a));
            }
            return out;
        }

        inline void removeById(int64_t id) {
            tcv::db::Database::instance().execParams("DELETE FROM apps WHERE id = ?", id);
        }
    } // namespace AppRepo

    // ============================================================
    // FunctionRepository
    // ============================================================
    namespace FuncRepo {
        inline std::optional<tcv::models::Function> find(int64_t app_id, const std::string& function_id) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM functions WHERE app_id = ? AND function_id = ?",
                app_id,
                function_id
            );
            if (rows.empty()) return std::nullopt;
            auto r = rows[0];
            tcv::models::Function f;
            f.id = getInt(r, "id");
            f.app_id = getInt(r, "app_id");
            f.function_id = getStr(r, "function_id");
            f.name = getStr(r, "name");
            f.description = getStr(r, "description");
            f.status = static_cast<int>(getInt(r, "status"));
            f.created_at = getInt(r, "created_at");
            return f;
        }

        inline int64_t insert(
            int64_t app_id,
            const std::string& fid,
            const std::string& name,
            const std::string& desc,
            int64_t now
        ) {
            tcv::db::Database::instance().execParams(
                "INSERT INTO functions(app_id, function_id, name, description, status, created_at)"
                " VALUES (?, ?, ?, ?, 1, ?)",
                app_id,
                fid,
                name,
                desc,
                now
            );
            return tcv::db::Database::instance().lastInsertId();
        }

        inline void remove(int64_t app_id, const std::string& fid) {
            tcv::db::Database::instance().execParams(
                "DELETE FROM functions WHERE app_id = ? AND function_id = ?",
                app_id,
                fid
            );
        }

        inline std::vector<tcv::models::Function> listByApp(int64_t app_id) {
            auto rows = tcv::db::Database::instance().queryParams(
                "SELECT * FROM functions WHERE app_id = ? ORDER BY id",
                app_id
            );
            std::vector<tcv::models::Function> out;
            for (auto& r : rows) {
                tcv::models::Function f;
                f.id = getInt(r, "id");
                f.app_id = getInt(r, "app_id");
                f.function_id = getStr(r, "function_id");
                f.name = getStr(r, "name");
                f.description = getStr(r, "description");
                f.status = static_cast<int>(getInt(r, "status"));
                f.created_at = getInt(r, "created_at");
                out.push_back(std::move(f));
            }
            return out;
        }
    } // namespace FuncRepo
} // namespace tcv::repo

#endif // TCV_REPOSITORIES_USER_APP_REPO
