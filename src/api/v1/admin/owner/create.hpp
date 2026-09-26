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

#ifndef TCV_API_V1_ADMIN_OWNER_CREATE
#define TCV_API_V1_ADMIN_OWNER_CREATE

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

namespace tcv::inside::api::v1::admin::owner::create {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto username = tcv::inside::api::v1::common::paramStr(req, "username");
        const auto password = tcv::inside::api::v1::common::paramStr(req, "password");
        const auto display = tcv::inside::api::v1::common::paramStr(req, "display_name");
        const int64_t admin_id = tcv::inside::api::v1::common::paramI64(req, "_admin_id", 0);
        if (username.empty() || password.empty()) {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(tcv::api::ErrorCode::VALIDATION, "参数不完整")
                )
            );
            return;
        }
        try {
            auto hp = tcv::service::hashPassword(password);
            int64_t id = tcv::repo::UserRepo::insert(username, hp.hash, hp.salt, display, std::time(nullptr), admin_id);
            tcv::repo::AuditRepo::log(
                "ADMIN",
                admin_id,
                "CreateOwner",
                "USER",
                id,
                "",
                "success",
                tcv::inside::api::v1::common::clientIp(req),
                "{}",
                std::time(nullptr)
            );
            Json::Value v;
            v["user_id"] = id;
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
        } catch (const std::exception& e) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(1003, e.what())));
        }
    }
} // namespace

#endif // TCV_API_V1_ADMIN_OWNER_CREATE
