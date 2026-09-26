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

#ifndef TCV_API_V1_ADMIN_AUDIT_LIST_THIS
#define TCV_API_V1_ADMIN_AUDIT_LIST_THIS

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

namespace tcv::inside::api::v1::admin::audit::list {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const int limit = static_cast<int>(tcv::inside::api::v1::common::paramI64(req, "limit", 200));
        const auto logs = tcv::repo::AuditRepo::listRecent(limit);
        Json::Value arr(Json::arrayValue);
        for (auto& a : logs) {
            Json::Value v;
            v["actor_type"] = a.actor_type;
            v["actor_id"] = a.actor_id;
            v["action"] = a.action;
            v["target_type"] = a.target_type;
            v["target_id"] = a.target_id.has_value() ? static_cast<Json::Int64>(*a.target_id) : 0;
            v["result"] = a.result;
            v["ip"] = a.ip;
            v["created_at"] = a.created_at;
            arr.append(v);
        }
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(arr)));
    }
} // namespace

#endif // TCV_API_V1_ADMIN_AUDIT_LIST_THIS
