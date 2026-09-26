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

#ifndef TCV_API_V1_AUTH_ADMIN_LOGIN
#define TCV_API_V1_AUTH_ADMIN_LOGIN

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

namespace tcv::inside::api::v1::auth::admin::login {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const std::string username = tcv::inside::api::v1::common::paramStr(req, "username");
        const std::string password = tcv::inside::api::v1::common::paramStr(req, "password");
        const auto r = tcv::service::AuthService::loginAdmin(
            username,
            password,
            tcv::inside::api::v1::common::clientIp(req),
            tcv::inside::api::v1::common::clientUa(req)
        );
        if (!r.success) {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(tcv::api::ErrorCode::USER_BAD_CREDENTIALS, r.message)
                )
            );
            return;
        }
        Json::Value v;
        v["token"] = r.token;
        v["user_type"] = r.user_type;
        v["user_id"] = r.user_id;
        v["username"] = r.username;
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
} // namespace tcv::inside::api::v1::auth::admin::login

#endif // TCV_API_V1_AUTH_ADMIN_LOGIN
