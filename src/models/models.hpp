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

#ifndef TCV_MODELS_MODELS
#define TCV_MODELS_MODELS

#include <optional>
#include <string>

namespace tcv::models {
    struct Admin {
        int64_t id = 0;
        std::string username;
        std::string password_hash;
        std::string salt;
        std::string display_name;
        int64_t created_at = 0;
        int64_t updated_at = 0;
        std::optional<int64_t> last_login_at;
        int status = 1;
    };

    struct User {
        int64_t id = 0;
        std::string username;
        std::string password_hash;
        std::string salt;
        std::string display_name;
        int64_t balance = -1;    // -1 = 无限
        int64_t expires_at = -1; // -1 = 永不过期
        int status = 1;
        std::optional<int64_t> created_by;
        int64_t created_at = 0;
        int64_t updated_at = 0;
    };

    struct SubUser {
        int64_t id = 0;
        int64_t parent_id = 0;
        std::string username;
        std::string password_hash;
        std::string salt;
        std::string display_name;
        int64_t quota_limit = 0;
        int64_t quota_used = 0;
        int64_t expires_at = -1;
        int status = 1;
        int64_t created_at = 0;
        int64_t updated_at = 0;
    };

    struct App {
        int64_t id = 0;
        int64_t owner_id = 0;
        std::string appid; // 纯数字随机字符串
        std::string name;
        std::string description;
        std::string secret_hash;
        int status = 1;
        int64_t created_at = 0;
        int64_t updated_at = 0;
    };

    struct Function {
        int64_t id = 0;
        int64_t app_id = 0;
        std::string function_id;
        std::string name;
        std::string description;
        int status = 1;
        int64_t created_at = 0;
    };

    struct Session {
        int64_t id = 0;
        std::string user_type; // ADMIN / USER / SUBUSER
        int64_t user_id = 0;
        std::string token_hash;
        int64_t expires_at = 0;
        int revoked = 0;
        int64_t created_at = 0;
        std::string created_ip;
        std::string user_agent;
    };

    struct TemporaryToken {
        int64_t id = 0;
        std::string token_hash;
        std::string description;
        int max_uses = 1;
        int used_count = 0;
        int64_t expires_at = 0;
        int revoked = 0;
        int64_t created_at = 0;
        int64_t created_by = 0;
        std::string used_by_ip;
    };

    struct License {
        int64_t id = 0;
        int64_t app_id = 0;
        std::string license_hash;
        std::string remark;

        int64_t created_at = 0;
        std::optional<int64_t> activated_at;
        int64_t expires_at = -1; // -1 = 永久

        std::string status = "unused"; // unused/active/expired/banned/deleted
        int banned = 0;

        std::string binding_mode = "NONE"; // NONE/IP/DEVICE/IP_AND_DEVICE
        std::optional<std::string> bound_ip_hash;
        std::optional<std::string> bound_device_hash;

        std::optional<int64_t> last_used_at;
        std::optional<std::string> last_used_ip;
        std::optional<std::string> last_used_device_hash;

        int unbind_count = 0;
        int unbind_limit = 0;

        int unbind_time_cost = 0;
        int unbind_count_cost = 0;

        int max_devices = 1;
        int max_ips = 1;

        int64_t created_by = 0;
        std::string created_by_role = "OWNER";
        int64_t updated_at = 0;

        std::string storage_mode = "HASH";
    };

    struct AuditLog {
        int64_t id = 0;
        std::string actor_type;
        int actor_id = 0;
        std::string action;
        std::string target_type;
        std::optional<int64_t> target_id;
        std::string target_detail;
        std::string result = "success";
        std::string ip;
        std::string extra;
        int64_t created_at = 0;
    };

    struct SystemConfig {
        std::string key;
        std::string value;
        std::string description;
        int64_t updated_at = 0;
    };
} // namespace tcv::models

#endif // TCV_MODELS_MODELS
