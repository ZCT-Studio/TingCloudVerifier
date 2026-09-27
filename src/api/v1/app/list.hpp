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

#ifndef TCV_API_V1_APP_LIST
#define TCV_API_V1_APP_LIST

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::app::list {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto user_type = tcv::inside::api::v1::common::currentUserType(req);
        const int64_t cur_user_id = tcv::inside::api::v1::common::currentUserId(req);
        const bool show_secret = tcv::inside::api::v1::common::paramBool(req, "secret", false);

        std::vector<tcv::models::App> apps;
        if (user_type == "ADMIN") {
            apps = tcv::repo::AppRepo::listAll();
            for (auto assigned = tcv::service::SubUserService::listAssignedApps(cur_user_id); auto app_id : assigned) {
                if (auto app = tcv::repo::AppRepo::findById(app_id)) apps.push_back(*app);
            }
        } else {
            apps = tcv::repo::AppRepo::listByOwner(cur_user_id);
        }

        Json::Value arr(Json::arrayValue);
        for (auto& a : apps) {
            Json::Value v;
            v["appid"] = a.appid;
            v["name"] = a.name;
            v["description"] = a.description;
            v["binding_mode"] = a.binding_mode;
            v["status"] = a.status;
            v["created_at"] = a.created_at;
            if (show_secret && user_type != "SUBUSER") {
                v["secret"] = a.secret;
            }

            auto owner = tcv::repo::UserRepo::findById(a.owner_id);
            std::string creator_name = owner ? owner->username : "unknown";
            if (creator_name.empty()) creator_name = "unknown";
            if (user_type == "USER" && a.owner_id == cur_user_id) {
                creator_name += "*";
            }
            if (user_type == "SUBUSER") {
                auto su_rows = tcv::db::Database::instance().queryParams(
                    "SELECT parent_id FROM sub_users WHERE id = ?", cur_user_id
                );
                if (!su_rows.empty()) {
                    int64_t parent_id = repo::getInt(su_rows[0], "parent_id");
                    if (a.owner_id == parent_id) {
                        creator_name = owner ? owner->username : "unknown";
                    }
                }
            }
            v["creator"] = creator_name;
            arr.append(v);
        }
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(arr)));
    }
}

#endif // TCV_API_V1_APP_LIST