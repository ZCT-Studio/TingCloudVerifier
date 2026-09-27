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

#ifndef TCV_API_V1_LICENSE_CREATE
#define TCV_API_V1_LICENSE_CREATE

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::license::create {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        tcv::service::LicenseService::CreateBatchInput in;
        in.app_id = tcv::inside::api::v1::common::paramStr(req, "app_id");
        in.count = tcv::inside::api::v1::common::paramI64(req, "count", 1);
        in.seconds_per_license = tcv::inside::api::v1::common::paramI64(req, "seconds_per_license", 86400);
        in.owner_id = tcv::inside::api::v1::common::currentUserId(req);
        in.owner_role = tcv::inside::api::v1::common::currentUserRole(req);
        in.remark = tcv::inside::api::v1::common::paramStr(req, "remark");
        in.binding_mode = tcv::inside::api::v1::common::paramStr(req, "binding_mode", "");
        in.unbind_limit = static_cast<int>(tcv::inside::api::v1::common::paramI64(req, "unbind_limit", 0));
        in.unbind_time_cost = static_cast<int>(tcv::inside::api::v1::common::paramI64(req, "unbind_time_cost", 0));
        in.unbind_count_cost = static_cast<int>(tcv::inside::api::v1::common::paramI64(req, "unbind_count_cost", 0));
        in.license_length = static_cast<int>(tcv::inside::api::v1::common::paramI64(req, "license_length", 32));
        in.license_type = tcv::inside::api::v1::common::paramStr(req, "license_type");

        const auto r = tcv::service::LicenseService::createBatch(in);
        if (!r.success) {
            cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeFail(1003, r.error)));
            return;
        }
        Json::Value v;
        v["created"] = r.actual_created;
        v["first_plain_license"] = r.first_license_plain;
        Json::Value arr(Json::arrayValue);
        for (const auto& lic : r.plain_licenses)
            arr.append(lic);
        v["licenses"] = arr;
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
}

#endif // TCV_API_V1_LICENSE_CREATE