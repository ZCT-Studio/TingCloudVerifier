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

#ifndef TCV_API_V1_AUTH_BOOTSTRAP_ADMIN
#define TCV_API_V1_AUTH_BOOTSTRAP_ADMIN

#include <exception>
#include <functional>
#include <ctime>
#include <drogon/drogon.h>
#include <json/json.h>
#include "api/response.hpp"
#include "api/v1/common.hpp"
#include "common/types.hpp"
#include "crypto/crypto.hpp"
#include "database/database.hpp"
#include "models/models.hpp"
#include "repositories/license_repo.hpp"
#include "services/auth_app_service.hpp"
#include "services/license_service.hpp"
#include "services/security_service.hpp"
#include "services/subuser_update_service.hpp"

namespace tcv::inside::api::v1::auth::bootstrap_admin {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto existing = tcv::db::Database::instance().queryScalar("SELECT COUNT(*) FROM admins");
        if (existing && *existing != "0") {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(1001, "已存在管理员")
                )
            );
            return;
        }
        const auto username = tcv::inside::api::v1::common::paramStr(req, "username");
        const auto password = tcv::inside::api::v1::common::paramStr(req, "password");
        const auto display = tcv::inside::api::v1::common::paramStr(req, "display_name", "");
        if (username.empty() || password.size() < 4) {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(tcv::api::ErrorCode::VALIDATION, "参数不合法")
                )
            );
            return;
        }
        const auto [hash, salt] = tcv::service::hashPassword(password);
        tcv::repo::AdminRepo::insert(username, hash, salt, display, std::time(nullptr));
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk()));
    }
} // namespace tcv::inside::api::v1::auth::bootstrap_admin

#endif // TCV_API_V1_AUTH_BOOTSTRAP_ADMIN
