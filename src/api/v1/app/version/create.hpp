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
// Created by wanjiangzhi on 2026/10/2.
//

#ifndef TCV_API_V1_APP_VERSION_CREATE
#define TCV_API_V1_APP_VERSION_CREATE

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::app::version::create_ {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto appid_str = tcv::inside::api::v1::common::paramStr(req, "app_id");
        const auto channel = tcv::inside::api::v1::common::paramStr(req, "channel", "stable");
        const auto version = tcv::inside::api::v1::common::paramStr(req, "version");
        const auto url = tcv::inside::api::v1::common::paramStr(req, "download_url");
        const auto hash = tcv::inside::api::v1::common::paramStr(req, "file_hash");
        const auto algo = tcv::inside::api::v1::common::paramStr(req, "hash_algorithm", "SHA256");
        const auto changelog = tcv::inside::api::v1::common::paramStr(req, "changelog");
        const auto minv = tcv::inside::api::v1::common::paramStr(req, "minimum_version");
        const int64_t release_time = tcv::inside::api::v1::common::paramI64(req, "release_time", std::time(nullptr));

        if (version.empty() || url.empty()) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::VALIDATION, "version 和 download_url 为必填")));
            return;
        }

        auto app = tcv::repo::AppRepo::findByAppid(appid_str);
        if (!app) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APP 不存在")));
            return;
        }

        // 找或自动创建 channel
        auto ch_rows = tcv::db::Database::instance().queryParams(
            "SELECT id FROM update_channels WHERE app_id = ? AND channel = ?",
            app->id,
            channel
        );
        int64_t channel_id;
        if (ch_rows.empty()) {
            auto r = tcv::service::UpdateService::createChannel(app->id, channel, "");
            if (!r.success) {
                cb(drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(r.code, r.message)));
                return;
            }
            channel_id = *r.value;
        } else {
            channel_id = tcv::repo::getInt(ch_rows[0], "id");
        }

        auto r = tcv::service::UpdateService::createVersion(
            channel_id, version, url, hash, algo, changelog, minv
        );
        if (!r.success) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(r.code, r.message)));
            return;
        }
        Json::Value v;
        v["version_id"] = static_cast<Json::Int64>(*r.value);
        v["appid"] = app->appid;
        v["channel"] = channel;
        v["version"] = version;
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
}

#endif // TCV_API_V1_APP_VERSION_CREATE