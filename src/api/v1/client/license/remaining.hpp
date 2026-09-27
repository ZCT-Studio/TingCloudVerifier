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

#ifndef TCV_API_V1_CLIENT_LICENSE_REMAINING
#define TCV_API_V1_CLIENT_LICENSE_REMAINING

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::client::license::remaining {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto appid = tcv::inside::api::v1::common::paramStr(req, "appid");
        const auto license_plain = tcv::inside::api::v1::common::paramStr(req, "license");
        const auto app = tcv::repo::AppRepo::findByAppid(appid);
        if (!app) {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APPID 不存在")
                )
            );
            return;
        }
        const auto lic = tcv::repo::LicenseRepo::findByAppAndLicense(app->id, license_plain);
        if (!lic) {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(tcv::api::ErrorCode::LICENSE_NOT_FOUND, "卡密不存在")
                )
            );
            return;
        }
        cb(
            drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeOk(tcv::service::LicenseService::remainingOf(*lic))
            )
        );
    }
}

#endif // TCV_API_V1_CLIENT_LICENSE_REMAINING
