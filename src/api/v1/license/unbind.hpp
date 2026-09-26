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

#ifndef TCV_API_V1_LICENSE_UNBIND
#define TCV_API_V1_LICENSE_UNBIND

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

namespace tcv::inside::api::v1::license::unbind {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        tcv::service::LicenseService::UnbindInput in;
        in.license_id = tcv::inside::api::v1::common::paramI64(req, "license_id");
        in.reason = tcv::inside::api::v1::common::paramStr(req, "reason");
        in.operator_id = tcv::inside::api::v1::common::currentUserId(req);
        in.operator_role = tcv::inside::api::v1::common::paramStr(req, "_owner_role", "OWNER");
        in.ip = tcv::inside::api::v1::common::clientIp(req);

        auto r = tcv::service::LicenseService::unbind(in);
        if (!r.success) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(1003, r.message)));
            return;
        }
        Json::Value v;
        v["new_expires_at"] = r.new_expires_at;
        v["new_unbind_count"] = r.new_unbind_count;
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
} // namespace

#endif // TCV_API_V1_LICENSE_UNBIND
