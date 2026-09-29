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
#include "crypto/crypto.hpp"

namespace tcv::inside::api::v1::client::license::verify {

namespace {
// Parse "k=v&k2=v2..." urlencoded form -> key/value map
inline void parseKvs(const std::string& body, std::function<void(const std::string&, const std::string&)> cb) {
    size_t pos = 0;
    while (pos < body.size()) {
        size_t amp = body.find('&', pos);
        std::string pair = (amp == std::string::npos) ? body.substr(pos) : body.substr(pos, amp - pos);
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            cb(pair.substr(0, eq), pair.substr(eq + 1));
        }
        if (amp == std::string::npos) break;
        pos = amp + 1;
    }
}
}

inline void handle(
    const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr &)>&& cb
) {
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

    // ── Decode layer (optional, per app dec_mode) ──
    auto encode = tcv::inside::api::v1::common::paramStr(req, "encode");
    if (app->dec_mode != "NONE") {
        if (encode.empty()) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::BAD_REQUEST, "missing encode")));
            return;
        }

        std::string decoded;
        bool ok = true;

        if (app->dec_mode == "AES256-GCM") {
            const auto blob = tcv::crypto::base64Decode(encode);
            const auto key_raw = tcv::crypto::fromHex(app->dec_key);
            decoded = tcv::crypto::aes256GcmDecrypt(blob, key_raw);
            ok = !decoded.empty();
        } else if (app->dec_mode == "BASE64") {
            decoded = tcv::crypto::base64Decode(encode);
        } else if (app->dec_mode == "HEX") {
            decoded = tcv::crypto::fromHex(encode);
        } else {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::BAD_REQUEST, "bad dec_mode")));
            return;
        }

        if (!ok) {
            cb(drogon::HttpResponse::newHttpJsonResponse(
                tcv::api::makeFail(tcv::api::ErrorCode::BAD_REQUEST, "decode failed")));
            return;
        }

        // Apply decoded body fields (override license/device if present)
        parseKvs(decoded, [&](const std::string& k, const std::string& v) {
            if (k == "license") license_plain = v;
            else if (k == "device") device = v;
            else if (k == "appid" && appid.empty()) appid = v;
        });
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
