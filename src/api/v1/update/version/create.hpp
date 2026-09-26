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


#ifndef TCV_API_V1_UPDATE_VERSION_CREATE
#define TCV_API_V1_UPDATE_VERSION_CREATE

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


namespace tcv::inside::api::v1::update::version::create {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        int64_t channel_id = tcv::inside::api::v1::common::paramI64(req, "channel_id");
        auto version = tcv::inside::api::v1::common::paramStr(req, "version");
        auto url = tcv::inside::api::v1::common::paramStr(req, "download_url");
        auto hash = tcv::inside::api::v1::common::paramStr(req, "file_hash");
        auto algo = tcv::inside::api::v1::common::paramStr(req, "hash_algorithm", "SHA256");
        auto changelog = tcv::inside::api::v1::common::paramStr(req, "changelog");
        auto minv = tcv::inside::api::v1::common::paramStr(req, "minimum_version");
        auto r = tcv::service::UpdateService::createVersion(channel_id, version, url, hash, algo, changelog, minv);
        if (!r.success) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(r.code, r.message)));
            return;
        }
        Json::Value v;
        v["version_id"] = static_cast<Json::Int64>(*r.value);
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
} // namespace

#endif // TCV_API_V1_UPDATE_VERSION_CREATE
