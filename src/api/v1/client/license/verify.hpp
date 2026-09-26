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

#ifndef TCV_API_V1_CLIENT_LICENSE_VERIFY
#define TCV_API_V1_CLIENT_LICENSE_VERIFY

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

namespace tcv::inside::api::v1::client::license::verify {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        auto appid = tcv::inside::api::v1::common::paramStr(req, "appid");
        auto license_plain = tcv::inside::api::v1::common::paramStr(req, "license");
        auto device = tcv::inside::api::v1::common::paramStr(req, "device");
        auto func_id = tcv::inside::api::v1::common::paramStr(req, "functionid");
        auto ip = tcv::inside::api::v1::common::clientIp(req);

        auto app = tcv::repo::AppRepo::findByAppid(appid);
        if (!app) {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APPID 不存在")
                )
            );
            return;
        }
        if (app->status != 1) {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(tcv::api::ErrorCode::APP_DISABLED, "APP 已禁用")
                )
            );
            return;
        }
        if (!func_id.empty()) {
            auto func = tcv::repo::FuncRepo::find(app->id, func_id);
            if (!func) {
                cb(
                    drogon::HttpResponse::newHttpJsonResponse(
                        tcv::api::makeFail(tcv::api::ErrorCode::FUNC_NOT_FOUND, "FunctionID 不存在")
                    )
                );
                return;
            }
        }

        tcv::service::LicenseVerifyInput in;
        in.license_plain = license_plain;
        in.ip = ip;
        if (!device.empty()) in.device = device;
        auto r = tcv::service::LicenseService::verify(in, app->id);

        Json::Value v;
        v["status"] = r.status;
        v["remaining"] = r.remaining;
        v["expired"] = r.expired;
        v["expires_at"] = r.expires_at;
        v["binding_mode"] = r.binding_mode;
        v["remark"] = r.remark;
        cb(
            drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeResponse(r.success ? 0 : r.code, r.message, v)
            )
        );
    }
} // namespace

#endif // TCV_API_V1_CLIENT_LICENSE_VERIFY
