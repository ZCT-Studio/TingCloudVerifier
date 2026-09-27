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

#ifndef TCV_API_V1_AUTH_BOOTSTRAP_ADMIN
#define TCV_API_V1_AUTH_BOOTSTRAP_ADMIN

#include "api/v1/uih.h"

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
}

#endif // TCV_API_V1_AUTH_BOOTSTRAP_ADMIN
