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

#ifndef TCV_API_V1_CLIENT_LICENSE_VERIFY
#define TCV_API_V1_CLIENT_LICENSE_VERIFY

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::client::license::verify {

inline void handle(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr &)>&& cb
) {
    // wrap() 已经在 paramStr 之前处理了 encode 解密 → attributes.tcv.decoded_kv
    // 如果请求没加密，paramStr 正常从 query/form/header/json 取值
    auto appid = tcv::inside::api::v1::common::paramStr(req, "appid");
    auto license_plain = tcv::inside::api::v1::common::paramStr(req, "license");
    auto device = tcv::inside::api::v1::common::paramStr(req, "device");
    auto ip = tcv::inside::api::v1::common::clientIp(req);

    auto app = tcv::repo::AppRepo::findByAppid(appid);
    if (!app) {
        cb(drogon::HttpResponse::newHttpJsonResponse(
            tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APPID 不存在")));
        return;
    }
    if (app->status != 1) {
        cb(drogon::HttpResponse::newHttpJsonResponse(
            tcv::api::makeFail(tcv::api::ErrorCode::APP_DISABLED, "APP 已禁用")));
        return;
    }

    if (license_plain.empty()) {
        cb(drogon::HttpResponse::newHttpJsonResponse(
            tcv::api::makeFail(tcv::api::ErrorCode::BAD_REQUEST, "missing license")));
        return;
    }

    tcv::service::LicenseVerifyInput in;
    in.license_plain = license_plain;
    in.ip = ip;
    if (!device.empty()) in.device = device;
    auto r = tcv::service::LicenseService::verify(in, app->id);

    Json::Value v;
    v["status"] = r.status;
    v["remaining"] = r.remaining;
    v["expired"] = r.expired;
    v["expires_at"] = r.expires_at;
    v["binding_mode"] = r.binding_mode;
    v["remark"] = r.remark;
    v["license_type"] = r.license_type;
    cb(drogon::HttpResponse::newHttpJsonResponse(
        tcv::api::makeResponse(r.success ? 0 : r.code, r.message, v)));
}

} // namespace tcv::inside::api::v1::client::license::verify

#endif // TCV_API_V1_CLIENT_LICENSE_VERIFY
