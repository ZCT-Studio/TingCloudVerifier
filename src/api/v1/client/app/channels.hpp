// Copyright 2026 ZCT-Studio
// Licensed under the Apache License, Version 2.0.

#ifndef TCV_API_V1_CLIENT_APP_CHANNELS
#define TCV_API_V1_CLIENT_APP_CHANNELS

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::client::app::channels {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto appid = tcv::inside::api::v1::common::paramStr(req, "appid");

        if (appid.empty()) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::VALIDATION, "缺少 appid")));
            return;
        }

        auto app = tcv::repo::AppRepo::findByAppid(appid);
        if (!app) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APPID 不存在")));
            return;
        }

        auto rows = tcv::db::Database::instance().queryParams(
            "SELECT channel FROM update_channels WHERE app_id = ? ORDER BY id ASC",
            app->id
        );
        Json::Value arr(Json::arrayValue);
        for (auto& r : rows) {
            arr.append(tcv::repo::getStr(r, "channel"));
        }
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(arr)));
    }
}

#endif // TCV_API_V1_CLIENT_APP_CHANNELS