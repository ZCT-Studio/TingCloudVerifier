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

#ifndef TCV_API_V1_FUNCTION_LIST
#define TCV_API_V1_FUNCTION_LIST

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

namespace tcv::inside::api::v1::function::list {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const int64_t app_id = tcv::inside::api::v1::common::paramI64(req, "app_id");
        const auto fs = tcv::repo::FuncRepo::listByApp(app_id);
        Json::Value arr(Json::arrayValue);
        for (auto& f : fs) {
            Json::Value v;
            v["function_id"] = f.function_id;
            v["name"] = f.name;
            v["description"] = f.description;
            v["status"] = f.status;
            arr.append(v);
        }
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(arr)));
    }
} // namespace

#endif // TCV_API_V1_FUNCTION_LIST
