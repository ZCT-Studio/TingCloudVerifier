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

#ifndef TCV_API_V1_AUTH_LOGOUT
#define TCV_API_V1_AUTH_LOGOUT

#include "api/v1/uih.h"

namespace tcv::inside::api::v1::auth::logout {
    inline void handle(
        const drogon::HttpRequestPtr& req,
        std::function<void(const drogon::HttpResponsePtr &)>&& cb
    ) {
        tcv::service::AuthService::logout(tcv::inside::api::v1::common::authBearer(req));
        cb(drogon::HttpResponse::newHttpJsonResponse(tcv::api::makeOk()));
    }
}

#endif // TCV_API_V1_AUTH_LOGOUT
