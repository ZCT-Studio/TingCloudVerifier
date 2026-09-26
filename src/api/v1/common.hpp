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

#ifndef TCV_API_V1_COMMON
#define TCV_API_V1_COMMON

#include <chrono>
#include <exception>
#include <functional>
#include <utility>
#include <string>
#include <drogon/drogon.h>
#include "api/response.hpp"
#include "common/logger.hpp"
#include "database/database.hpp"
#include "repositories/license_repo.hpp"
#include "repositories/user_app_repo.hpp"
#include "services/auth_app_service.hpp"
#include "services/security_service.hpp"

namespace tcv::inside::api::v1::common {
    inline std::string paramStr(
        const drogon::HttpRequestPtr& req,
        const std::string& key,
        const std::string& def = ""
    ) {
        auto& p = req->getParameters();
        if (const auto it = p.find(key); it != p.end() && !it->second.empty()) return it->second;

        auto& fp = req->getParameters();

        const auto& hdrs = req->headers();
        for (const auto& k : {
                 key,
                 std::string(1, static_cast<char>(toupper(key[0]))) + key.substr(1)
             }) {
            if (auto h = hdrs.find(k); h != hdrs.end()) return h->second;
        }

        const auto body = req->getJsonObject();
        if (body && body->isMember(key)) {
            const auto& v = (*body)[key];
            if (v.isString()) return v.asString();
            if (v.isInt() || v.isInt64()) return std::to_string(v.asInt64());
            if (v.isBool()) return v.asBool() ? "1" : "0";
        }
        return def;
    }

    inline int64_t paramI64(const drogon::HttpRequestPtr& req, const std::string& key, int64_t def = 0) {
        const auto s = paramStr(req, key);
        if (s.empty()) return def;
        try { return std::stoll(s); } catch (...) { return def; }
    }

    inline bool paramBool(const drogon::HttpRequestPtr& req, const std::string& key, bool def = false) {
        const auto s = paramStr(req, key);
        if (s.empty()) return def;
        return s == "1" || s == "true" || s == "TRUE";
    }

    inline std::string clientIp(const drogon::HttpRequestPtr& req) {
        return req->getPeerAddr().toIp();
    }

    inline std::string clientUa(const drogon::HttpRequestPtr& req) {
        const auto& h = req->headers();
        for (const auto k : {"user-agent", "User-Agent"}) {
            if (auto it = h.find(k); it != h.end()) return it->second;
        }
        return "";
    }

    inline std::string authBearer(const drogon::HttpRequestPtr& req) {
        const auto& h = req->headers();
        for (const auto k : {"authorization", "Authorization"}) {
            if (auto it = h.find(k); it != h.end()) return it->second;
        }
        return "";
    }

    inline int64_t currentUserId(const drogon::HttpRequestPtr& req) {
        const auto attrs = req->attributes();
        if (!attrs || !attrs->find("tcv.user_id")) return 0;
        try {
            const auto v = attrs->get<int64_t>("tcv.user_id");
            return v;
        } catch (...) {}
        try {
            const auto v = attrs->get<int>("tcv.user_id");
            return v;
        } catch (...) { return 0; }
    }

    inline std::string currentUserType(const drogon::HttpRequestPtr& req) {
        const auto attrs = req->attributes();
        if (!attrs || !attrs->find("tcv.user_type")) return "";
        try { return attrs->get<std::string>("tcv.user_type"); } catch (...) { return ""; }
    }

    template <typename Fn>
    auto wrap(Fn fn) {
        return [fn = std::move(fn)](
            const drogon::HttpRequestPtr& req,
            std::function<void(const drogon::HttpResponsePtr &)>&& cb
        ) {
            auto t0 = std::chrono::steady_clock::now();
            auto method = std::string(req->methodString());
            std::string path = req->path();
            std::string ip = req->getPeerAddr().toIp();
            tcv::logger().INFO("-> {} {} from {}", method, path, ip);
            try {
                auto req_path = req->path();

                if (req_path.rfind("/api/v1/admin/", 0) == 0) {
                    std::string bearer = authBearer(req);
                    auto u = tcv::service::AuthService::authenticateSession(bearer);
                    if (!u.success || u.user_type != "ADMIN") {
                        cb(
                            drogon::HttpResponse::newHttpJsonResponse(
                                tcv::api::makeFail(tcv::api::ErrorCode::UNAUTHORIZED, "需要管理员登录")
                            )
                        );
                        return;
                    }

                    auto req_ip = req->getPeerAddr().toIp();
                    auto level = tcv::repo::ConfigRepo::get("admin_security_level", "LAN");
                    if (bool is_local = (req_ip == "127.0.0.1" || req_ip == "::1"); level == "LOCAL" && !is_local) {
                        cb(
                            drogon::HttpResponse::newHttpJsonResponse(
                                tcv::api::makeFail(tcv::api::ErrorCode::IP_NOT_ALLOWED, "LOCAL 级别仅允许本机访问")
                            )
                        );
                        return;
                    }
                    if (level == "PUBLIC") {
                        auto wh = tcv::db::Database::instance().queryParams(
                            "SELECT 1 FROM ip_whitelist WHERE ip = ?",
                            req_ip
                        );
                        if (wh.empty()) {
                            cb(
                                drogon::HttpResponse::newHttpJsonResponse(
                                    tcv::api::makeFail(tcv::api::ErrorCode::IP_NOT_ALLOWED, "IP 不在白名单")
                                )
                            );
                            return;
                        }
                    }
                    (*req->attributes())["tcv.user_type"] = u.user_type;
                    (*req->attributes())["tcv.user_id"] = u.user_id;
                }

                else if (req_path.rfind("/api/v1/app/", 0) == 0 ||
                         req_path.rfind("/api/v1/license/", 0) == 0 ||
                         req_path.rfind("/api/v1/function/", 0) == 0) {
                    std::string bearer = authBearer(req);
                    auto u = tcv::service::AuthService::authenticateSession(bearer);
                    if (!u.success) {
                        cb(
                            drogon::HttpResponse::newHttpJsonResponse(
                                tcv::api::makeFail(tcv::api::ErrorCode::UNAUTHORIZED, "未登录或登录已过期")
                            )
                        );
                        return;
                    }
                    (*req->attributes())["tcv.user_type"] = u.user_type;
                    (*req->attributes())["tcv.user_id"] = u.user_id;
                }

                else if (req_path.rfind("/api/v1/client/", 0) == 0) {
                    auto& cfg = tcv::AppConfig::instance().security();
                    auto& params = req->getParameters();
                    auto it = params.find("timestamp");
                    if (it == params.end() || it->second.empty()) {
                        cb(
                            drogon::HttpResponse::newHttpJsonResponse(
                                tcv::api::makeFail(tcv::api::ErrorCode::TIMESTAMP_BAD, "缺少 timestamp")
                            )
                        );
                        return;
                    }
                    int64_t ts = 0;
                    try { ts = std::stoll(it->second); } catch (...) {}
                    int64_t now = std::time(nullptr);
                    if (std::llabs(now - ts) > cfg.replay_window_seconds) {
                        cb(
                            drogon::HttpResponse::newHttpJsonResponse(
                                tcv::api::makeFail(tcv::api::ErrorCode::TIMESTAMP_BAD, "timestamp 超出窗口")
                            )
                        );
                        return;
                    }
                    auto nonce_it = params.find("nonce");
                    auto appid_it = params.find("appid");
                    if (nonce_it != params.end() && appid_it != params.end()) {
                        auto app = tcv::repo::AppRepo::findByAppid(appid_it->second);
                        if (app) {
                            if (!tcv::repo::NonceRepo::tryUseNonce(
                                app->id,
                                nonce_it->second,
                                ts,
                                cfg.replay_window_seconds
                            )) {
                                cb(
                                    drogon::HttpResponse::newHttpJsonResponse(
                                        tcv::api::makeFail(tcv::api::ErrorCode::NONCE_REPLAY, "nonce 重复")
                                    )
                                );
                                return;
                            }
                        }
                    }
                }
                fn(req, std::move(cb));
                auto t1 = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
                tcv::logger().INFO("<- {} {} done in {}ms", method, req_path, ms);
            } catch (const std::exception& e) {
                tcv::logger().FATAL("Handler exception on {} {}: {}", method, path, e.what());
                auto resp = drogon::HttpResponse::newHttpJsonResponse(
                    tcv::api::makeFail(
                        tcv::api::ErrorCode::INTERNAL,
                        std::string("internal: ") + e.what()
                    )
                );
                cb(resp);
            }
        };
    }
} // namespace tcv::inside::api::v1::common

#endif // TCV_API_V1_COMMON

