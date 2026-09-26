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

#ifndef TCV_API_V1_CLIENT_LICENSE_STATUS
#define TCV_API_V1_CLIENT_LICENSE_STATUS

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

namespace tcv::inside::api::v1::client::license::status {
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
        const auto lhash = tcv::service::hashLicense(license_plain);
        const auto lic = tcv::repo::LicenseRepo::findByAppAndHash(app->id, lhash);
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
                tcv::api::makeOk(tcv::service::LicenseService::statusOf(*lic))
            )
        );
    }
} // namespace

#endif // TCV_API_V1_CLIENT_LICENSE_STATUS
