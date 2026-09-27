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

#ifndef TCV_API_V1_UPDATE_VERSION_CREATE
#define TCV_API_V1_UPDATE_VERSION_CREATE

#include "api/v1/uih.h"

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
