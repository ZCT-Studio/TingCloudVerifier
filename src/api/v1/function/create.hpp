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

#ifndef TCV_API_V1_FUNCTION_CREATE
#define TCV_API_V1_FUNCTION_CREATE

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

namespace tcv::inside::api::v1::function::create {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        int64_t app_id = tcv::inside::api::v1::common::paramI64(req, "app_id");
        auto fid = tcv::inside::api::v1::common::paramStr(req, "function_id");
        auto name = tcv::inside::api::v1::common::paramStr(req, "name");
        auto desc = tcv::inside::api::v1::common::paramStr(req, "description");
        if (fid.empty() || name.empty()) {
            cb(
                drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(tcv::api::ErrorCode::VALIDATION, "function_id/name 必填")
                )
            );
            return;
        }
        try {
            int64_t id = tcv::repo::FuncRepo::insert(app_id, fid, name, desc, std::time(nullptr));
            Json::Value v;
            v["id"] = id;
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
        } catch (const std::exception& e) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(1003, e.what())));
        }
    }
} // namespace

#endif // TCV_API_V1_FUNCTION_CREATE
