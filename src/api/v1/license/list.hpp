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

#ifndef TCV_API_V1_LICENSE_LIST
#define TCV_API_V1_LICENSE_LIST

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
#include "repositories/user_app_repo.hpp"
#include "repositories/license_repo.hpp"
#include "services/auth_app_service.hpp"
#include "services/license_service.hpp"
#include "services/security_service.hpp"
#include "services/subuser_update_service.hpp"

namespace tcv::inside::api::v1::license::list {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        auto appid_str = tcv::inside::api::v1::common::paramStr(req, "app_id");
        auto app = tcv::repo::AppRepo::findByAppid(appid_str);
        if (!app) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "app_id 对应的 APP 不存在")
            ));
            return;
        }
        const int64_t app_id = app->id;
        const auto user_type = tcv::inside::api::v1::common::currentUserType(req);
        const int64_t cur_user_id = tcv::inside::api::v1::common::currentUserId(req);

        std::vector<tcv::models::License> lics;
        if (user_type == "SUBUSER") {
            lics = tcv::repo::LicenseRepo::listByAppAndCreator(app_id, cur_user_id, "SUBUSER");
        } else {
            lics = tcv::repo::LicenseRepo::listByApp(app_id);
        }

        Json::Value arr(Json::arrayValue);
        for (auto& l : lics) {
            Json::Value v;
            v["id"] = l.id;
            v["license"] = l.license;
            v["status"] = l.status;
            v["banned"] = l.banned != 0;
            v["binding_mode"] = l.binding_mode;
            v["license_type"] = l.license_type;
            v["expires_at"] = l.expires_at;
            v["created_at"] = l.created_at;
            v["unbind_count"] = l.unbind_count;
            v["unbind_limit"] = l.unbind_limit;

            if (l.created_by_role == "SUBUSER") {
                auto su_rows = tcv::db::Database::instance().queryParams(
                    "SELECT username FROM sub_users WHERE id = ?", l.created_by
                );
                std::string creator = su_rows.empty() ? "subuser" : repo::getStr(su_rows[0], "username");
                if (user_type == "SUBUSER" && l.created_by == cur_user_id) {
                    creator += '*';
                } else if (user_type == "OWNER") {
                }
                v["creator"] = creator;
            } else {
                auto owner = tcv::repo::UserRepo::findById(l.created_by);
                std::string creator = owner ? owner->username : "owner";
                if (user_type == "USER" && l.created_by == cur_user_id) {
                    creator += '*';
                }
                v["creator"] = creator;
            }
            arr.append(v);
        }
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(arr)));
    }
}

#endif // TCV_API_V1_LICENSE_LIST