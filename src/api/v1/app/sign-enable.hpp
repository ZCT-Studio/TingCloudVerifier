// Copyright 2026 ZCT-Studio
// Licensed under the Apache License, Version 2.0.

#ifndef TCV_API_V1_APP_SIGN_ENABLE
#define TCV_API_V1_APP_SIGN_ENABLE

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::app::sign_enable {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto user_type = tcv::inside::api::v1::common::currentUserType(req);
        const auto appid_str = tcv::inside::api::v1::common::paramStr(req, "app_id");
        const auto enable_str = tcv::inside::api::v1::common::paramStr(req, "sign_enable", "0");

        if (user_type != "USER") {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::UNAUTHORIZED, "仅主用户可修改")));
            return;
        }

        const int enable = (enable_str == "1") ? 1 : 0;

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

        tcv::repo::AppRepo::updateSignEnable(app->id, enable, std::time(nullptr));
        Json::Value v;
        v["appid"] = app->appid;
        v["sign_enable"] = enable;
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
}

#endif // TCV_API_V1_APP_SIGN_ENABLE
