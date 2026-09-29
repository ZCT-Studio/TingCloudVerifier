// Copyright 2026 ZCT-Studio
// Licensed under the Apache License, Version 2.0.

#ifndef TCV_API_V1_APP_CHANNEL_LIST
#define TCV_API_V1_APP_CHANNEL_LIST

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::app::channel::list_ {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        const auto appid = tcv::inside::api::v1::common::paramStr(req, "appid");

        auto app = tcv::repo::AppRepo::findByAppid(appid);
        if (!app) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::APP_NOT_FOUND, "APPID 不存在")));
            return;
        }

        auto channels = tcv::service::UpdateService::listChannels(appid);
        Json::Value arr(Json::arrayValue);
        for (auto& c : channels) arr.append(c);
        Json::Value v;
        v["channels"] = arr;
        v["count"] = static_cast<int>(channels.size());
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk(v)));
    }
}

#endif // TCV_API_V1_APP_CHANNEL_LIST