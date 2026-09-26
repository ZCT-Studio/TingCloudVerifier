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

#ifndef TCV_SERVICES_LICENSE_SERVICE
#define TCV_SERVICES_LICENSE_SERVICE

#include <optional>
#include <string>
#include <ctime>
#include <json/json.h>
#include <format>
#include "common/types.hpp"

namespace tcv::service {
    struct LicenseVerifyInput {
        std::string license_plain;
        std::string ip;
        std::optional<std::string> device;
    };

    struct LicenseVerifyResult {
        bool success = false;
        int code = 0;
        std::string message;

        std::string status;
        int64_t remaining = 0; // 秒
        bool expired = false;
        int64_t expires_at = 0;
        int64_t activated_at = 0;
        std::string binding_mode;
        std::string remark;
    };

    inline std::string hashLicense(const std::string_view plain) {
        return tcv::crypto::sha256Hex(plain);
    }

    namespace LicenseService {
        inline int64_t virtualNow() {
            return repo::GlobalTimer::effectiveNow(std::time(nullptr));
        }

        inline int64_t calcRemaining(const int64_t expires_at) {
            if (expires_at == -1) return -1;
            int64_t now = virtualNow();
            int64_t r = expires_at - now;
            return r < 0 ? 0 : r;
        }

        struct CreateBatchInput {
            int64_t app_id;
            int64_t count;
            int64_t seconds_per_license; // -1 = 永久
            int64_t owner_id;
            std::string owner_role; // OWNER / SUBUSER
            std::string remark;
            std::string binding_mode; // NONE / IP / DEVICE / IP_AND_DEVICE
            int unbind_limit = 0;
            int unbind_time_cost = 0;
            int unbind_count_cost = 0;
        };

        struct CreateBatchResult {
            bool success{};
            int64_t actual_created = 0;
            std::string first_license_plain; // 返回第一条明文卡密用于通知
            std::string error;
        };

        inline CreateBatchResult createBatch(const CreateBatchInput& in) {
            CreateBatchResult r;
            auto app = repo::AppRepo::findById(in.app_id);
            if (!app) {
                r.error = "APP 不存在";
                return r;
            }

            int64_t cost_per = 1; // 每张卡消耗 1 额度
            int64_t total_cost = cost_per * in.count;
            bool is_sub = (in.owner_role == "SUBUSER");

            if (!is_sub) {
                int64_t owner_balance = -1;
                auto u = repo::UserRepo::findById(in.owner_id);
                if (!u) {
                    r.error = "主用户不存在";
                    return r;
                }
                owner_balance = u->balance;
                if (owner_balance != -1 && owner_balance < total_cost) {
                    r.error = "主用户余额不足";
                    return r;
                }
            } else {
                int64_t sub_quota_used = 0;
                int64_t sub_quota_limit = -1;
                auto rows = db::Database::instance().queryParams(
                    "SELECT quota_limit, quota_used FROM sub_users WHERE id = ?",
                    in.owner_id
                );
                if (rows.empty()) {
                    r.error = "子用户不存在";
                    return r;
                }
                sub_quota_limit = repo::getInt(rows[0], "quota_limit");
                sub_quota_used = repo::getInt(rows[0], "quota_used");
                if (sub_quota_limit != -1 && sub_quota_used + total_cost > sub_quota_limit) {
                    r.error = "子用户额度不足";
                    return r;
                }
            }

            int64_t now = std::time(nullptr);
            int64_t effective_expires;
            if (in.seconds_per_license == -1) {
                effective_expires = -1;
            } else {
                effective_expires = virtualNow() + in.seconds_per_license;
            }

            bool ok = db::Database::instance().transaction(
                [&] {
                    int64_t main_user_id = in.owner_id;
                    if (is_sub) {
                        auto rows = db::Database::instance().queryParams(
                            "SELECT parent_id FROM sub_users WHERE id = ?",
                            in.owner_id
                        );
                        if (rows.empty()) return false;
                        main_user_id = repo::getInt(rows[0], "parent_id");
                    }

                    if (int64_t new_balance = repo::UserRepo::atomicConsumeBalance(main_user_id, total_cost); new_balance == -1 && total_cost > 0) {
                        if (auto u = repo::UserRepo::findById(main_user_id); !u || u->balance != -1) return false;
                    }

                    if (is_sub) {
                        db::Database::instance().execParams(
                            "UPDATE sub_users SET quota_used = quota_used + ?, updated_at = ? WHERE id = ?",
                            total_cost,
                            now,
                            in.owner_id
                        );
                    }

                    int64_t created = 0;
                    std::string first_plain;
                    for (int64_t i = 0; i < in.count; ++i) {
                        std::string plain = tcv::crypto::randomAlphanumeric(24);
                        std::string lhash = hashLicense(plain);

                        models::License l;
                        l.app_id = in.app_id;
                        l.license_hash = lhash;
                        l.remark = in.remark;
                        l.created_at = now;
                        l.expires_at = effective_expires;
                        l.status = "unused";
                        l.banned = 0;
                        l.binding_mode = in.binding_mode.empty() ? "NONE" : in.binding_mode;
                        l.unbind_limit = in.unbind_limit;
                        l.unbind_time_cost = in.unbind_time_cost;
                        l.unbind_count_cost = in.unbind_count_cost;
                        l.created_by = in.owner_id;
                        l.created_by_role = in.owner_role;
                        l.updated_at = now;
                        l.storage_mode = "HASH";

                        repo::LicenseRepo::insert(l);
                        ++created;
                        if (first_plain.empty()) first_plain = plain;
                    }

                    r.actual_created = created;
                    r.first_license_plain = first_plain;
                    return true;
                }
            );

            if (!ok) {
                r.error = "创建失败，余额/配额不足或数据库错误";
                return r;
            }

            r.success = true;

            repo::AuditRepo::log(
                in.owner_role,
                in.owner_id,
                "CreateLicense",
                "APP",
                in.app_id,
                std::format("count={},sec={}", in.count, in.seconds_per_license),
                "success",
                "",
                "{}",
                now
            );
            return r;
        }

        inline LicenseVerifyResult verify(const LicenseVerifyInput& in, int64_t app_id) {
            LicenseVerifyResult out;

            std::string lhash = hashLicense(in.license_plain);
            auto lic = repo::LicenseRepo::findByAppAndHash(app_id, lhash);
            if (!lic) {
                out.code = api::ErrorCode::LICENSE_NOT_FOUND;
                out.message = "卡密不存在";
                return out;
            }

            if (lic->banned) {
                out.code = api::ErrorCode::LICENSE_BANNED;
                out.message = "卡密已封禁";
                return out;
            }

            int64_t remain = calcRemaining(lic->expires_at);

            if (bool expired = (remain == 0 && lic->expires_at != -1); lic->status == "expired" || expired) {
                out.code = api::ErrorCode::LICENSE_EXPIRED;
                out.message = "卡密已过期";
                out.expired = true;
                return out;
            }

            std::string b_mode = lic->binding_mode;
            if (b_mode == "IP" || b_mode == "IP_AND_DEVICE") {
                std::string ip_hash = tcv::crypto::sha256Hex(in.ip);
                if (lic->bound_ip_hash && *lic->bound_ip_hash != ip_hash) {
                    out.code = api::ErrorCode::LICENSE_BOUND_IP;
                    out.message = "IP 绑定不匹配";
                    return out;
                }

                if (!lic->bound_ip_hash) {
                    int64_t now = std::time(nullptr);
                    if (b_mode == "IP") repo::LicenseRepo::bindIp(lic->id, ip_hash, now);
                    else {
                        std::string dev_hash = tcv::crypto::sha256Hex(in.device.value_or(""));
                        repo::LicenseRepo::bindIpAndDevice(lic->id, ip_hash, dev_hash, now);
                    }
                    lic->bound_ip_hash = ip_hash;
                    lic->status = "active";
                }
            }

            if (b_mode == "DEVICE" || b_mode == "IP_AND_DEVICE") {
                if (!in.device) {
                    out.code = api::ErrorCode::LICENSE_BOUND_DEV;
                    out.message = "需要 device 参数";
                    return out;
                }
                std::string dev_hash = tcv::crypto::sha256Hex(*in.device);
                if (lic->bound_device_hash && *lic->bound_device_hash != dev_hash) {
                    out.code = api::ErrorCode::LICENSE_BOUND_DEV;
                    out.message = "设备绑定不匹配";
                    return out;
                }
                if (!lic->bound_device_hash) {
                    int64_t now = std::time(nullptr);
                    if (b_mode == "DEVICE") repo::LicenseRepo::bindDevice(lic->id, dev_hash, now);
                    lic->bound_device_hash = dev_hash;
                    lic->status = "active";
                }
            }

            int64_t now = std::time(nullptr);
            repo::LicenseRepo::touchLastUsed(
                lic->id,
                in.ip,
                in.device ? std::optional(tcv::crypto::sha256Hex(*in.device)) : std::nullopt,
                now
            );

            out.success = true;
            out.code = 0;
            out.message = "OK";
            out.status = lic->status;
            out.remaining = remain;
            out.expired = (remain == 0 && lic->expires_at != -1);
            out.expires_at = lic->expires_at;
            out.activated_at = lic->activated_at.value_or(0);
            out.binding_mode = b_mode;
            out.remark = lic->remark;
            return out;
        }

        struct UnbindInput {
            int64_t license_id;
            std::string reason;
            int64_t operator_id;
            std::string operator_role;
            std::string ip;
        };

        struct UnbindResult {
            bool success{};
            std::string message;
            int64_t new_expires_at = 0;
            int new_unbind_count = 0;
        };

        inline UnbindResult unbind(const UnbindInput& in) {
            UnbindResult r;
            const auto lic = repo::LicenseRepo::findById(in.license_id);
            if (!lic) {
                r.message = "卡密不存在";
                return r;
            }

            if (lic->unbind_limit > 0 && lic->unbind_count >= lic->unbind_limit) {
                r.message = "解绑次数已达上限";
                return r;
            }

            int64_t now = std::time(nullptr);
            int64_t before_expires = lic->expires_at;
            int64_t after_expires = before_expires;

            int64_t cost_count = 0, cost_time = 0;

            if (lic->unbind_count_cost > 0) {
                cost_count = lic->unbind_count_cost;
            }
            if (lic->unbind_time_cost > 0 && lic->expires_at != -1) {
                cost_time = lic->unbind_time_cost;
                after_expires = lic->expires_at - lic->unbind_time_cost;
                if (after_expires < now) after_expires = now; // 不能负债
            }

            repo::GlobalTimer::effectiveNow(now); // 刷新一下
            repo::LicenseRepo::clearBinding(in.license_id, now);
            repo::LicenseRepo::incrUnbindCount(in.license_id, now);

            if (after_expires != before_expires) {
                repo::LicenseRepo::updateFieldInt(in.license_id, "expires_at", after_expires);
            }

            db::Database::instance().execParams(
                "INSERT INTO license_unbind_records"
                "(license_id, reason, before_expires, after_expires, cost_count, cost_time, unbind_count_after, ip, operator_id, created_at)"
                " VALUES (?, ?, ?, ?, ?, ?, (SELECT unbind_count FROM licenses WHERE id = ?), ?, ?, ?)",
                in.license_id,
                in.reason,
                before_expires,
                after_expires,
                cost_count,
                cost_time,
                in.license_id,
                in.ip,
                in.operator_id,
                now
            );

            repo::AuditRepo::log(
                in.operator_role,
                in.operator_id,
                "LicenseUnbind",
                "LICENSE",
                in.license_id,
                "",
                "success",
                in.ip,
                "{}",
                now
            );

            const auto refreshed = repo::LicenseRepo::findById(in.license_id);
            r.success = true;
            r.message = "解绑成功";
            r.new_expires_at = refreshed ? refreshed->expires_at : after_expires;
            r.new_unbind_count = refreshed ? refreshed->unbind_count : 0;
            return r;
        }

        inline tcv::common::Result ban(int64_t license_id) {
            const auto lic = repo::LicenseRepo::findById(license_id);
            if (!lic) return tcv::common::Result::fail(5001, "卡密不存在");
            const int64_t now = std::time(nullptr);
            repo::LicenseRepo::updateFieldInt(license_id, "banned", 1);
            repo::AuditRepo::log(
                "OWNER",
                lic->created_by,
                "LicenseBan",
                "LICENSE",
                license_id,
                "",
                "success",
                "",
                "{}",
                now
            );
            return tcv::common::Result::ok();
        }

        inline tcv::common::Result unban(int64_t license_id) {
            const auto lic = repo::LicenseRepo::findById(license_id);
            if (!lic) return tcv::common::Result::fail(5001, "卡密不存在");
            repo::LicenseRepo::updateFieldInt(license_id, "banned", 0);
            repo::AuditRepo::log(
                "OWNER",
                lic->created_by,
                "LicenseUnban",
                "LICENSE",
                license_id,
                "",
                "success",
                "",
                "{}",
                std::time(nullptr)
            );
            return tcv::common::Result::ok();
        }

        inline tcv::common::Result addTime(int64_t license_id, int64_t add_seconds) {
            const auto lic = repo::LicenseRepo::findById(license_id);
            if (!lic) return tcv::common::Result::fail(5001, "卡密不存在");
            const int64_t now = std::time(nullptr);
            const int64_t base = lic->expires_at == -1 ? now : lic->expires_at;
            const int64_t new_exp = base + add_seconds;
            repo::LicenseRepo::updateFieldInt(license_id, "expires_at", new_exp);
            repo::AuditRepo::log(
                "OWNER",
                lic->created_by,
                "LicenseAddTime",
                "LICENSE",
                license_id,
                std::format("add={}", add_seconds),
                "success",
                "",
                "{}",
                now
            );
            return tcv::common::Result::ok();
        }

        inline tcv::common::Result setTime(int64_t license_id, int64_t new_expires_at) {
            const auto lic = repo::LicenseRepo::findById(license_id);
            if (!lic) return tcv::common::Result::fail(5001, "卡密不存在");
            repo::LicenseRepo::updateFieldInt(license_id, "expires_at", new_expires_at);
            repo::AuditRepo::log(
                "OWNER",
                lic->created_by,
                "LicenseSetTime",
                "LICENSE",
                license_id,
                std::format("set={}", new_expires_at),
                "success",
                "",
                "{}",
                std::time(nullptr)
            );
            return tcv::common::Result::ok();
        }

        inline tcv::common::Result removeLicense(int64_t license_id, bool hard = false) {
            const auto lic = repo::LicenseRepo::findById(license_id);
            if (!lic) return tcv::common::Result::fail(5001, "卡密不存在");
            if (hard) repo::LicenseRepo::hardDelete(license_id);
            else repo::LicenseRepo::softDelete(license_id);
            repo::AuditRepo::log(
                "OWNER",
                lic->created_by,
                hard ? "LicenseHardDelete" : "LicenseSoftDelete",
                "LICENSE",
                license_id,
                "",
                "success",
                "",
                "{}",
                std::time(nullptr)
            );
            return tcv::common::Result::ok();
        }

        inline Json::Value remainingOf(const models::License& lic) {
            Json::Value v;
            const int64_t r = calcRemaining(lic.expires_at);
            v["remaining"] = static_cast<Json::Int64>(r);
            v["unit"] = "seconds";
            v["expired"] = (r == 0 && lic.expires_at != -1);
            return v;
        }

        inline Json::Value statusOf(const models::License& lic) {
            Json::Value v;
            v["status"] = lic.status;
            v["banned"] = (lic.banned != 0);
            const int64_t r = calcRemaining(lic.expires_at);
            v["expired"] = (r == 0 && lic.expires_at != -1);
            v["remaining"] = static_cast<Json::Int64>(r);
            v["binding_mode"] = lic.binding_mode;
            v["activated_at"] = lic.activated_at.value_or(0);
            v["expires_at"] = lic.expires_at;
            return v;
        }
    } // namespace LicenseService
} // namespace tcv::service

#endif // TCV_SERVICES_LICENSE_SERVICE
