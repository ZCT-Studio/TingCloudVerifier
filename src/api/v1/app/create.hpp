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

#ifndef TCV_API_V1_APP_CREATE
#define TCV_API_V1_APP_CREATE

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::app::create {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto user_type = tcv::inside::api::v1::common::currentUserType(req);
        if (user_type == "SUBUSER") {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::UNAUTHORIZED, "子用户无权创建应用")
            ));
            return;
        }
        const int64_t owner_id = tcv::inside::api::v1::common::currentUserId(req);
        const auto name = tcv::inside::api::v1::common::paramStr(req, "name");
        const auto desc = tcv::inside::api::v1::common::paramStr(req, "description");
        const auto r = tcv::service::AppService::createApp(owner_id, name, desc);
        if (!r.success) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(r.code, r.message)));
            return;
        }
        const auto app = tcv::repo::AppRepo::findByAppid(r.value->appid);
        Json::Value v;
        v["appid"] = r.value->appid;
        v["secret"] = r.value->secret;
        v["notice"] = app ? app->notice : "";
        v["binding_mode"] = app ? app->binding_mode : "NONE";
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
}

#endif // TCV_API_V1_APP_CREATE