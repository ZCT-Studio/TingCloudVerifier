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


#ifndef TCV_API_V1_REGISTER_ALL
#define TCV_API_V1_REGISTER_ALL

#include <drogon/drogon.h>
#include "api/v1/common.hpp"
#include "api/response.hpp"
#include "common/logger.hpp"
#include "middleware/middlewares.hpp"

// --- system ---
#include "api/v1/system/ping.hpp"
#include "api/v1/system/configs.hpp"

// --- auth ---
#include "api/v1/auth/admin/login.hpp"
#include "api/v1/auth/bootstrap-admin.hpp"
#include "api/v1/auth/logout.hpp"
#include "api/v1/auth/owner/login.hpp"
#include "api/v1/auth/subuser/login.hpp"

// --- admin ---
#include "admin/audit/list.hpp"
#include "api/v1/admin/license-timer/pause.hpp"
#include "api/v1/admin/license-timer/resume.hpp"
#include "api/v1/admin/owner/balance.hpp"
#include "api/v1/admin/owner/create.hpp"

// --- app ---
#include "api/v1/app/create.hpp"
#include "api/v1/app/delete.hpp"
#include "api/v1/app/list.hpp"
#include "api/v1/app/secret/regenerate.hpp"

// --- function ---
#include "api/v1/function/create.hpp"
#include "api/v1/function/delete.hpp"
#include "api/v1/function/list.hpp"

// --- license ---
#include "api/v1/license/add-time.hpp"
#include "api/v1/license/ban.hpp"
#include "api/v1/license/create.hpp"
#include "api/v1/license/delete.hpp"
#include "api/v1/license/list.hpp"
#include "api/v1/license/set-time.hpp"
#include "api/v1/license/unban.hpp"
#include "api/v1/license/unbind.hpp"

// --- client ---
#include "api/v1/client/license/remaining.hpp"
#include "api/v1/client/license/status.hpp"
#include "api/v1/client/license/verify.hpp"

// --- update ---
#include "api/v1/update/channel/create.hpp"
#include "api/v1/update/latest.hpp"
#include "api/v1/update/version/create.hpp"


namespace tcv::api {
    inline void registerAllV1Routes(drogon::HttpAppFramework& app) {
        using tcv::inside::api::v1::common::wrap;
        using namespace drogon;

        // ============ /api/v1/system（公开） ============
        app.registerHandler(
            "/api/v1/system/ping",
            wrap(&tcv::inside::api::v1::system::ping::handle),
            {Get}
        );
        app.registerHandler(
            "/api/v1/system/configs",
            wrap(&tcv::inside::api::v1::system::configs::handle),
            {Get}
        );

        app.registerHandler(
            "/api/v1/auth/admin/login",
            wrap(&tcv::inside::api::v1::auth::admin::login::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/auth/owner/login",
            wrap(&tcv::inside::api::v1::auth::owner::login::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/auth/subuser/login",
            wrap(&tcv::inside::api::v1::auth::subuser::login::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/auth/logout",
            wrap(&tcv::inside::api::v1::auth::logout::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/auth/bootstrap-admin",
            wrap(&tcv::inside::api::v1::auth::bootstrap_admin::handle),
            {Post}
        );

        app.registerHandler(
            "/api/v1/admin/owner/create",
            wrap(&tcv::inside::api::v1::admin::owner::create::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/admin/owner/balance",
            wrap(&tcv::inside::api::v1::admin::owner::balance::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/admin/license-timer/pause",
            wrap(&tcv::inside::api::v1::admin::license_timer::pause::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/admin/license-timer/resume",
            wrap(&tcv::inside::api::v1::admin::license_timer::resume::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/admin/audit/list",
            wrap(&tcv::inside::api::v1::admin::audit::list::handle),
            {Get}
        );

        app.registerHandler(
            "/api/v1/app/create",
            wrap(&tcv::inside::api::v1::app::create::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/app/list",
            wrap(&tcv::inside::api::v1::app::list::handle),
            {Get}
        );
        app.registerHandler(
            "/api/v1/app/secret/regenerate",
            wrap(&tcv::inside::api::v1::app::secret::regenerate::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/app/delete",
            wrap(&tcv::inside::api::v1::app::delete_::handle),
            {Post}
        );

        app.registerHandler(
            "/api/v1/function/create",
            wrap(&tcv::inside::api::v1::function::create::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/function/list",
            wrap(&tcv::inside::api::v1::function::list::handle),
            {Get}
        );
        app.registerHandler(
            "/api/v1/function/delete",
            wrap(&tcv::inside::api::v1::function::delete_::handle),
            {Post}
        );

        app.registerHandler(
            "/api/v1/license/create",
            wrap(&tcv::inside::api::v1::license::create::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/license/delete",
            wrap(&tcv::inside::api::v1::license::delete_::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/license/ban",
            wrap(&tcv::inside::api::v1::license::ban::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/license/unban",
            wrap(&tcv::inside::api::v1::license::unban::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/license/add-time",
            wrap(&tcv::inside::api::v1::license::add_time::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/license/set-time",
            wrap(&tcv::inside::api::v1::license::set_time::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/license/unbind",
            wrap(&tcv::inside::api::v1::license::unbind::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/license/list",
            wrap(&tcv::inside::api::v1::license::list::handle),
            {Get}
        );

        app.registerHandler(
            "/api/v1/client/license/verify",
            wrap(&tcv::inside::api::v1::client::license::verify::handle),
            {Post, Get}
        );
        app.registerHandler(
            "/api/v1/client/license/remaining",
            wrap(&tcv::inside::api::v1::client::license::remaining::handle),
            {Get}
        );
        app.registerHandler(
            "/api/v1/client/license/status",
            wrap(&tcv::inside::api::v1::client::license::status::handle),
            {Get}
        );

        app.registerHandler(
            "/api/v1/update/latest",
            wrap(&tcv::inside::api::v1::update::latest::handle),
            {Get}
        );
        app.registerHandler(
            "/api/v1/update/channel/create",
            wrap(&tcv::inside::api::v1::update::channel::create::handle),
            {Post}
        );
        app.registerHandler(
            "/api/v1/update/version/create",
            wrap(&tcv::inside::api::v1::update::version::create::handle),
            {Post}
        );

        tcv::logger().INFO("All /api/v1 routes registered");
    }
} // namespace tcv::api

#endif // TCV_API_V1_REGISTER_ALL
