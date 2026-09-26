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

#ifndef TCV_API_V1_LICENSE_CREATE
#define TCV_API_V1_LICENSE_CREATE

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

namespace tcv::inside::api::v1::license::create {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        tcv::service::LicenseService::CreateBatchInput in;
        in.app_id = tcv::inside::api::v1::common::paramI64(req, "app_id");
        in.count = tcv::inside::api::v1::common::paramI64(req, "count", 1);
        in.seconds_per_license = tcv::inside::api::v1::common::paramI64(req, "seconds_per_license", 86400);
        in.owner_id = tcv::inside::api::v1::common::currentUserId(req);
        in.owner_role = tcv::inside::api::v1::common::paramStr(req, "_owner_role", "OWNER");
        in.remark = tcv::inside::api::v1::common::paramStr(req, "remark");
        in.binding_mode = tcv::inside::api::v1::common::paramStr(req, "binding_mode", "NONE");
        in.unbind_limit = static_cast<int>(tcv::inside::api::v1::common::paramI64(req, "unbind_limit", 0));
        in.unbind_time_cost = static_cast<int>(tcv::inside::api::v1::common::paramI64(req, "unbind_time_cost", 0));
        in.unbind_count_cost = static_cast<int>(tcv::inside::api::v1::common::paramI64(req, "unbind_count_cost", 0));

        const auto r = tcv::service::LicenseService::createBatch(in);
        if (!r.success) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(1003, r.error)));
            return;
        }
        Json::Value v;
        v["created"] = r.actual_created;
        v["first_plain_license"] = r.first_license_plain;
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
} // namespace

#endif // TCV_API_V1_LICENSE_CREATE
