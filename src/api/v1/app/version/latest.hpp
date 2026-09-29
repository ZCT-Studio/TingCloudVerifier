// Copyright 2026 ZCT-Studio
// Licensed under the Apache License, Version 2.0.

#ifndef TCV_API_V1_APP_VERSION_LATEST
#define TCV_API_V1_APP_VERSION_LATEST

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::app::version::latest {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto appid = tcv::inside::api::v1::common::paramStr(req, "appid");
        const auto channel = tcv::inside::api::v1::common::paramStr(req, "channel", "stable");

        auto app = tcv::repo::AppRepo::findByAppid(appid);
        if (!app) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APPID 不存在")));
            return;
        }

        auto v = tcv::service::UpdateService::getLatest(appid, channel);
        if (!v["found"].asBool()) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::NOT_FOUND, "该频道暂无版本")));
            return;
        }
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
}

#endif // TCV_API_V1_APP_VERSION_LATEST