// Copyright 2026 ZCT-Studio
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

//
// Created by wanjiangzhi on 2026/9/25.
//

#ifndef TCV_MIDDLEWARE_MIDDLEWARES
#define TCV_MIDDLEWARE_MIDDLEWARES

#include <functional>
#include <string>
#include <ctime>
#include <memory>
#include <drogon/drogon.h>
#include "common/logger.hpp"
#include "common/types.hpp"
#include "crypto/crypto.hpp"
#include "repositories/license_repo.hpp"
#include "services/auth_app_service.hpp"
#include "services/security_service.hpp"

namespace tcv::middleware {
    class RateLimitMiddleware : public drogon::HttpMiddleware<RateLimitMiddleware, false> {
    public:
        void invoke(
            const drogon::HttpRequestPtr& req,
            drogon::MiddlewareNextCallback&& nextCb,
            drogon::MiddlewareCallback&& mcb
        ) override {
            auto& rl = tcv::AppConfig::instance().rateLimit();
            if (!rl.enabled) {
                nextCb(nullptr);
                return;
            }

            const auto ip = req->getPeerAddr().toIp();
            const std::string key_ip = "ip:" + ip;
            if (!tcv::service::RateLimiter::instance().tryAcquire(key_ip, rl.ip_per_sec)) {
                const auto resp = drogon::HttpResponse::newHttpJsonResponse(
                    api::makeFail(api::ErrorCode::RATE_LIMITED, "请求过于频繁")
                );
                resp->setStatusCode(drogon::k429TooManyRequests);
                mcb(resp);
                return;
            }
            nextCb(nullptr);
        }
    };

    class BearerAuthMiddleware : public drogon::HttpMiddleware<BearerAuthMiddleware, false> {
    public:
        void invoke(
            const drogon::HttpRequestPtr& req,
            drogon::MiddlewareNextCallback&& nextCb,
            drogon::MiddlewareCallback&& mcb
        ) override {
            auto path = req->path();
            tcv::logger().DEBUG("[MW:Bearer] path={}", path);
            bool need = path.rfind("/api/v1/app/", 0) == 0 ||
                        path.rfind("/api/v1/license/", 0) == 0 ||
                        path.rfind("/api/v1/function/", 0) == 0;
            tcv::logger().DEBUG("[MW:Bearer] need={}", need);
            if (!need) {
                nextCb(nullptr);
                return;
            }

            std::string bearer;
            auto& hdrs = req->headers();
            if (const auto it = hdrs.find("authorization"); it != hdrs.end()) bearer = it->second;
            else if (const auto IT = hdrs.find("Authorization"); IT != hdrs.end()) bearer = IT->second;

            auto u = tcv::service::AuthService::authenticateSession(bearer);
            tcv::logger().DEBUG("[MW:Bearer] auth success={} uid={} type={}", u.success, u.user_id, u.user_type);
            if (!u.success) {
                mcb(
                    drogon::HttpResponse::newHttpJsonResponse(
                        api::makeFail(api::ErrorCode::UNAUTHORIZED, "未登录或登录已过期")
                    )
                );
                return;
            }

            (*req->attributes())["tcv.user_type"] = u.user_type;
            (*req->attributes())["tcv.user_id"] = u.user_id;
            tcv::logger().DEBUG("[MW:Bearer] attributes set!");
            nextCb(nullptr);
        }
    };

    class AdminAuthMiddleware : public drogon::HttpMiddleware<AdminAuthMiddleware, false> {
    public:
        void invoke(
            const drogon::HttpRequestPtr& req,
            drogon::MiddlewareNextCallback&& nextCb,
            drogon::MiddlewareCallback&& mcb
        ) override {
            auto path = req->path();
            bool need = path.rfind("/api/v1/admin/", 0) == 0;
            if (!need) {
                nextCb(nullptr);
                return;
            }

            auto ip = req->getPeerAddr().toIp();
            std::string bearer;
            auto& hdrs = req->headers();
            if (const auto it = hdrs.find("authorization"); it != hdrs.end()) bearer = it->second;
            else if (const auto IT = hdrs.find("Authorization"); IT != hdrs.end()) bearer = IT->second;

            auto u = tcv::service::AuthService::authenticateSession(bearer);
            if (!u.success || u.user_type != "ADMIN") {
                mcb(
                    drogon::HttpResponse::newHttpJsonResponse(
                        api::makeFail(api::ErrorCode::UNAUTHORIZED, "需要管理员登录")
                    )
                );
                return;
            }

            const auto level = tcv::repo::ConfigRepo::get("admin_security_level", "LAN");
            const bool is_local = (ip == "127.0.0.1" || ip == "::1");
            if (level == "LOCAL" && !is_local) {
                mcb(
                    drogon::HttpResponse::newHttpJsonResponse(
                        api::makeFail(api::ErrorCode::IP_NOT_ALLOWED, "LOCAL 级别仅允许本机访问")
                    )
                );
                return;
            }
            if (level == "PUBLIC") {
                const auto wh = tcv::db::Database::instance().queryParams(
                    "SELECT 1 FROM ip_whitelist WHERE ip = ?",
                    ip
                );
                if (wh.empty()) {
                    mcb(
                        drogon::HttpResponse::newHttpJsonResponse(
                            api::makeFail(api::ErrorCode::IP_NOT_ALLOWED, "IP 不在白名单")
                        )
                    );
                    return;
                }
            }

            (*req->attributes())["tcv.user_type"] = u.user_type;
            (*req->attributes())["tcv.user_id"] = u.user_id;
            nextCb(nullptr);
        }
    };

    class AppSignatureMiddleware : public drogon::HttpMiddleware<AppSignatureMiddleware, false> {
    public:
        void invoke(
            const drogon::HttpRequestPtr& req,
            drogon::MiddlewareNextCallback&& nextCb,
            drogon::MiddlewareCallback&& mcb
        ) override {
            const auto path = req->path();
            if (const bool need = path.rfind("/api/v1/client/", 0) == 0; !need) {
                nextCb(nullptr);
                return;
            }

            auto& cfg = tcv::AppConfig::instance().security();
            auto& params = req->getParameters();

            const auto it = params.find("timestamp");
            if (it == params.end() || it->second.empty()) {
                mcb(
                    drogon::HttpResponse::newHttpJsonResponse(
                        api::makeFail(api::ErrorCode::TIMESTAMP_BAD, "缺少 timestamp")
                    )
                );
                return;
            }
            int64_t ts = 0;
            try { ts = std::stoll(it->second); } catch (...) {}
            int64_t now = std::time(nullptr);
            if (std::llabs(now - ts) > cfg.replay_window_seconds) {
                mcb(
                    drogon::HttpResponse::newHttpJsonResponse(
                        api::makeFail(api::ErrorCode::TIMESTAMP_BAD, "timestamp 超出窗口")
                    )
                );
                return;
            }

            const auto nonce_it = params.find("nonce");
            if (const auto appid_it = params.find("appid"); nonce_it != params.end() && appid_it != params.end()) {
                if (const auto app = tcv::repo::AppRepo::findByAppid(appid_it->second)) {
                    const bool ok = tcv::repo::NonceRepo::tryUseNonce(
                        app->id,
                        nonce_it->second,
                        ts,
                        cfg.replay_window_seconds
                    );
                    if (!ok) {
                        mcb(
                            drogon::HttpResponse::newHttpJsonResponse(
                                api::makeFail(api::ErrorCode::NONCE_REPLAY, "nonce 重复")
                            )
                        );
                        return;
                    }
                }
            }

            nextCb(nullptr);
        }
    };

    inline void registerAllMiddlewares(drogon::HttpAppFramework& app) {
        app.registerMiddleware(std::make_shared<RateLimitMiddleware>());
        app.registerMiddleware(std::make_shared<BearerAuthMiddleware>());
        app.registerMiddleware(std::make_shared<AdminAuthMiddleware>());
        app.registerMiddleware(std::make_shared<AppSignatureMiddleware>());
    }
} // namespace tcv::middleware

#endif // TCV_MIDDLEWARE_MIDDLEWARES
