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

#ifndef TCV_API_V1_SYSTEM_PING
#define TCV_API_V1_SYSTEM_PING

#include <exception>
#include <functional>
#include <ctime>
#include <drogon/drogon.h>
#include <json/json.h>
#include "constants.hpp"
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

namespace tcv::inside::api::v1::system::ping {
    inline void handle(
        const drogon::HttpRequestPtr&,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        Json::Value v;
        v["pong"] = true;
        v["server"] = "TingCloudVerifier";
        v["project"] = tcv::constants::PROJECT_NAME;
        v["version"] = tcv::constants::PROJECT_VERSION;
        v["server_time"] = std::time(nullptr);
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
} // namespace tcv::inside::api::v1::system::ping

#endif // TCV_API_V1_SYSTEM_PING
