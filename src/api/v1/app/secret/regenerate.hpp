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

#ifndef TCV_API_V1_APP_SECRET_REGENERATE
#define TCV_API_V1_APP_SECRET_REGENERATE

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

namespace tcv::inside::api::v1::app::secret::regenerate {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const int64_t id = tcv::inside::api::v1::common::paramI64(req, "id");
        const auto app = tcv::repo::AppRepo::findById(id);
        if (!app) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(3001, "APP 不存在")));
            return;
        }
        const auto new_secret = tcv::crypto::randomHex(32);
        tcv::repo::AppRepo::updateSecretHash(id, tcv::crypto::sha256Hex(new_secret), std::time(nullptr));
        Json::Value v;
        v["secret"] = new_secret;
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
} // namespace

#endif // TCV_API_V1_APP_SECRET_REGENERATE
