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

#ifndef TCV_API_V1_AUTH_ADMIN_LOGIN
#define TCV_API_V1_AUTH_ADMIN_LOGIN

#include "api/v1/uih.h"

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
}

#endif // TCV_API_V1_AUTH_ADMIN_LOGIN
