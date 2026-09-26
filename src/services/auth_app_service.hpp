// Copyright 2026 ZCT-Studio
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Created by wanjiangzhi on 2026/9/25.


#ifndef TCV_SERVICES_AUTH_APP_SERVICE
#define TCV_SERVICES_AUTH_APP_SERVICE

#include <string>
#include <ctime>
#include <cstdint>
#include <optional>
#include <vector>
#include "common/types.hpp"
#include "config/app_config.hpp"
#include "crypto/crypto.hpp"
#include "repositories/user_app_repo.hpp"
#include "services/license_service.hpp"

namespace tcv::service {
    struct HashedPassword {
        std::string hash;
        std::string salt;
    };

    inline HashedPassword hashPassword(std::string_view password) {
        std::string salt = tcv::crypto::randomHex(16);
        std::string h;
        #if defined(TCV_HAS_ARGON2)
        h = tcv::crypto::argon2idHash(password, salt);
        #else
        h = tcv::crypto::sha512Hex(salt + std::string(password) + salt);
        #endif
        return {h, salt};
    }

    inline bool verifyPassword(std::string_view password, const std::string& hash, const std::string& salt) {
        #if defined(TCV_HAS_ARGON2)
        return tcv::crypto::argon2idVerify(password, salt, hash);
        #else
        auto actual = tcv::crypto::sha512Hex(salt + std::string(password) + salt);
        volatile unsigned char diff = 0;
        for (size_t i = 0; i < actual.size() && i < hash.size(); ++i)
            diff |= actual[i] ^ hash[i];
        return diff == 0;
        #endif
    }

    namespace AuthService {
        struct LoginResult {
            bool success;
            std::string token;     // Bearer Token（十六进制）
            std::string user_type; // ADMIN / USER / SUBUSER
            int64_t user_id;
            std::string username;
            int status;
            std::string message;
        };

        inline LoginResult loginAdmin(
            const std::string& username,
            const std::string& password,
            const std::string& ip,
            const std::string& ua
        ) {
            LoginResult r{false, {}, {}, 0, {}, 0, ""};
            auto admin = repo::AdminRepo::findByUsername(username);
            if (!admin) {
                r.message = "用户名或密码错误";
                return r;
            }
            if (admin->status != 1) {
                r.message = "账号已被禁用";
                return r;
            }
            if (!verifyPassword(password, admin->password_hash, admin->salt)) {
                r.message = "用户名或密码错误";
                return r;
            }

            int64_t now = std::time(nullptr);
            int64_t ttl = tcv::AppConfig::instance().security().session_ttl_seconds;
            std::string token = tcv::crypto::randomHex(32);
            std::string token_hash = tcv::crypto::sha256Hex(token);

            repo::SessionRepo::insert("ADMIN", admin->id, token_hash, now + ttl, now, ip, ua);
            repo::AdminRepo::updateLastLogin(admin->id, now);

            r.success = true;
            r.token = token;
            r.user_type = "ADMIN";
            r.user_id = admin->id;
            r.username = admin->username;
            r.status = admin->status;
            r.message = "OK";
            return r;
        }

        inline LoginResult loginOwner(
            const std::string& username,
            const std::string& password,
            const std::string& ip,
            const std::string& ua
        ) {
            LoginResult r{false, {}, {}, 0, {}, 0, ""};
            auto user = repo::UserRepo::findByUsername(username);
            if (!user) {
                r.message = "用户名或密码错误";
                return r;
            }
            if (user->status != 1) {
                r.message = "账号已禁用";
                return r;
            }
            if (user->expires_at != -1 && user->expires_at < std::time(nullptr)) {
                r.message = "账号已过期";
                return r;
            }
            if (!verifyPassword(password, user->password_hash, user->salt)) {
                r.message = "用户名或密码错误";
                return r;
            }
            int64_t now = std::time(nullptr);
            int64_t ttl = tcv::AppConfig::instance().security().session_ttl_seconds;
            std::string token = tcv::crypto::randomHex(32);
            std::string token_hash = tcv::crypto::sha256Hex(token);
            repo::SessionRepo::insert("USER", user->id, token_hash, now + ttl, now, ip, ua);

            r.success = true;
            r.token = token;
            r.user_type = "USER";
            r.user_id = user->id;
            r.username = user->username;
            r.status = user->status;
            r.message = "OK";
            return r;
        }

        inline LoginResult loginSubUser(
            const std::string& username,
            const std::string& password,
            const std::string& ip,
            const std::string& ua
        ) {
            LoginResult r{.success = false, .token = {}, .user_type = {}, .user_id = 0, .username = {}, .status = 0, .message = ""};
            auto rows = db::Database::instance().queryParams(
                "SELECT * FROM sub_users WHERE username = ?",
                username
            );
            if (rows.empty()) {
                r.message = "用户名或密码错误";
                return r;
            }
            const auto& su = rows[0];
            std::string pw_hash = repo::getStr(su, "password_hash");
            std::string salt = repo::getStr(su, "salt");
            int status = static_cast<int>(repo::getInt(su, "status"));
            int64_t expires_at = repo::getInt(su, "expires_at");
            int64_t id = repo::getInt(su, "id");

            if (status != 1) {
                r.message = "账号已禁用";
                return r;
            }
            if (expires_at != -1 && expires_at < std::time(nullptr)) {
                r.message = "账号已过期";
                return r;
            }
            if (!verifyPassword(password, pw_hash, salt)) {
                r.message = "用户名或密码错误";
                return r;
            }

            int64_t now = std::time(nullptr);
            int64_t ttl = tcv::AppConfig::instance().security().session_ttl_seconds;
            std::string token = tcv::crypto::randomHex(32);
            repo::SessionRepo::insert("SUBUSER", id, tcv::crypto::sha256Hex(token), now + ttl, now, ip, ua);

            r.success = true;
            r.token = token;
            r.user_type = "SUBUSER";
            r.user_id = id;
            r.username = username;
            r.status = status;
            r.message = "OK";
            return r;
        }

        struct AuthenticatedUser {
            bool success = false;
            std::string user_type;
            int64_t user_id = 0;
            std::string username;
            int status = 0;
        };

        inline AuthenticatedUser authenticateSession(const std::string& bearer) {
            AuthenticatedUser u;
            if (bearer.empty()) return u;
            std::string token = bearer;
            if (token.starts_with("Bearer ")) token = token.substr(7);
            std::string token_hash = tcv::crypto::sha256Hex(token);
            auto sess = repo::SessionRepo::findByTokenHash(token_hash);
            if (!sess) return u;

            u.user_type = sess->user_type;
            u.user_id = sess->user_id;

            if (u.user_type == "ADMIN") {
                auto a = repo::AdminRepo::findById(u.user_id);
                if (!a) return u;
                u.username = a->username;
                u.status = a->status;
            } else if (u.user_type == "USER") {
                auto ou = repo::UserRepo::findById(u.user_id);
                if (!ou) return u;
                u.username = ou->username;
                u.status = ou->status;
            } else if (u.user_type == "SUBUSER") {
                auto rows = db::Database::instance().queryParams(
                    "SELECT username, status FROM sub_users WHERE id = ?",
                    u.user_id
                );
                if (rows.empty()) return u;
                u.username = repo::getStr(rows[0], "username");
                u.status = static_cast<int>(repo::getInt(rows[0], "status"));
            }
            if (u.status != 1) return u;
            u.success = true;
            return u;
        }

        inline bool logout(const std::string& bearer) {
            std::string token = bearer;
            if (token.starts_with("Bearer ")) token = token.substr(7);
            repo::SessionRepo::revokeByHash(tcv::crypto::sha256Hex(token));
            return true;
        }
    } // namespace AuthService

    namespace AppService {
        struct AppSecretPair {
            std::string appid;
            std::string secret;
        };

        inline tcv::common::ResultOf<AppSecretPair> createApp(
            const int64_t owner_id,
            const std::string& name,
            const std::string& description
        ) {
            std::string appid;
            for (int i = 0; i < 5; ++i) {
                appid = tcv::crypto::randomNumeric(12);
                if (const auto existing = repo::AppRepo::findByAppid(appid); !existing) break;
            }
            const std::string secret = tcv::crypto::randomHex(32);
            const std::string secret_hash = tcv::crypto::sha256Hex(secret);
            const int64_t now = std::time(nullptr);
            int64_t id = repo::AppRepo::insert(owner_id, appid, name, description, secret_hash, now);

            return tcv::common::ResultOf<AppSecretPair>::ok({appid, secret});
        }

        inline tcv::common::Result regenerateSecret(int64_t app_db_id) {
            const auto app = repo::AppRepo::findById(app_db_id);
            if (!app) return tcv::common::Result::fail(3001, "APP 不存在");
            const std::string secret = tcv::crypto::randomHex(32);
            repo::AppRepo::updateSecretHash(app_db_id, tcv::crypto::sha256Hex(secret), std::time(nullptr));
            repo::AuditRepo::log(
                "OWNER",
                app->owner_id,
                "RegenerateAppSecret",
                "APP",
                app_db_id,
                app->appid,
                "success",
                "",
                "{}",
                std::time(nullptr)
            );
            return tcv::common::Result::ok();
        }

        inline bool verifyAppSecret(const int64_t app_db_id, const std::string_view secret_try) {
            const auto app = repo::AppRepo::findById(app_db_id);
            if (!app) return false;
            return tcv::crypto::sha256Hex(secret_try) == app->secret_hash;
        }
    } // namespace AppService
} // namespace tcv::service

#endif // TCV_SERVICES_AUTH_APP_SERVICE
