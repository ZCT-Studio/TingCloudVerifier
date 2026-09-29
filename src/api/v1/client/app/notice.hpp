// Copyright 2026 ZCT-Studio
// Licensed under the Apache License, Version 2.0.

#ifndef TCV_API_V1_CLIENT_APP_NOTICE
#define TCV_API_V1_CLIENT_APP_NOTICE

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::client::app::notice {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto appid = tcv::inside::api::v1::common::paramStr(req, "appid");
        const auto app = tcv::repo::AppRepo::findByAppid(appid);
        if (!app) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APPID 不存在")));
            return;
        }

        Json::Value v;
        v["appid"] = app->appid;
        v["notice"] = app->notice;
        v["has_notice"] = !app->notice.empty();
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
}

#endif // TCV_API_V1_CLIENT_APP_NOTICE
