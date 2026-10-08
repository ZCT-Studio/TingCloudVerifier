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
// Created by wanjiangzhi on 2026/10/3.
//

#ifndef TCV_API_V1_APP_SET_DECODE
#define TCV_API_V1_APP_SET_DECODE

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::app::set_decode {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto user_type = tcv::inside::api::v1::common::currentUserType(req);
        const auto appid_str = tcv::inside::api::v1::common::paramStr(req, "app_id");
        const auto mode = tcv::inside::api::v1::common::paramStr(req, "dec_mode", "NONE");
        const auto key = tcv::inside::api::v1::common::paramStr(req, "dec_key", "");

        if (user_type != "USER") {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::UNAUTHORIZED, "仅主用户可修改")));
            return;
        }

        if (mode != "NONE" && mode != "AES256-GCM" && mode != "BASE64" && mode != "HEX") {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::VALIDATION, "dec_mode 必须为 NONE/AES256-GCM/BASE64/HEX")));
            return;
        }
        if ((mode == "AES256-GCM" || mode == "HEX") && key.size() != 64) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::VALIDATION, "dec_key 必须为 64 位 hex")));
            return;
        }

        auto app = tcv::repo::AppRepo::findByAppid(appid_str);
        if (!app) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APP 不存在")));
            return;
        }

        const int64_t cur_user_id = tcv::inside::api::v1::common::currentUserId(req);
        if (app->owner_id != cur_user_id) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::UNAUTHORIZED, "无权修改此 APP")));
            return;
        }

        tcv::repo::AppRepo::updateDecodeMode(app->id, mode, key, std::time(nullptr));
        Json::Value v;
        v["appid"] = app->appid;
        v["dec_mode"] = mode;
        // dec_key 不回显
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
}

#endif // TCV_API_V1_APP_SET_DECODE