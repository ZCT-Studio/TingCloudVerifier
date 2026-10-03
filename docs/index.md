# TingCloudVerifier

> 多平台软件授权验证服务端 · API 文档

<div class="vp-home-hero-name-color">
  **TingCloudVerifier**
</div>

一款基于 **Drogon + SQLite + OpenSSL** 的 C++23 软件授权服务端，支持卡密系统、设备/IP 绑定、可选 AES256-GCM 加密传输、可选 HMAC-SHA256 签名校验，以及稳定/rc/canary/alpha 四版本通道。

## 快速导航

- **[客户端安全层](./client/security.md)** — 四要素、签名、加密详解
- **[加密传输](./client/encryption.md)** — AES256-GCM / BASE64 / HEX 请求响应对称加密
- **[签名校验](./client/signing.md)** — HMAC-SHA256 canonical 串算法
- **[完整示例](./client/examples.md)** — Python / Go / curl 客户端示例
- **[API 概览](./api/overview.md)** — 全部端点速查表
- **[认证 API](./api/auth/bootstrap-admin.md)** — bootstrap-admin / login / logout
- **[APP 管理](./api/app/create.md)** — create / list / delete / binding / secret / decode / sign / notice / channel / version
- **[卡密管理](./api/license/create.md)** — create / list / set-time / ban / unbind / delete
- **[客户端 API](./api/client/license-verify.md)** — verify / status / remaining / channels / version / notice

## 特性

- ✅ 三种角色：ADMIN / OWNER / SUBUSER
- ✅ 多应用管理 + 全局唯一 APPID
- ✅ 卡密生命周期（创建/封禁/解绑/重置）
- ✅ NONE / IP / DEVICE / IP_AND_DEVICE 绑定模式
- ✅ AES256-GCM / BASE64 / HEX 请求响应对称加密（APP 级粒度）
- ✅ HMAC-SHA256 + timestamp + nonce 防重放（APP 级粒度）
- ✅ 客户端公告 + 四版本通道
- ✅ 9 平台 CI 构建（Windows x64/x86/arm64、Linux x64/x86/arm64/armv7、macOS arm64/x64）

## 架构

```
客户端 ── POST /api/v1/client/* ──► wrap() 中间件（四要素校验 + 解密）
                                        │
                                        ▼
                                   Handler（业务逻辑）
                                        │
                                        ▼
                                   wrap() callback 包装（加密响应）
                                        │
                                        ▼
                                   HTTP JSON / {encode: "..."}
```

加密/签名完全在中间件层处理，Handler 代码**零加密依赖**。

## 技术栈

| 组件 | 版本 |
|---|---|
| C++ | 23 |
| Drogon | 1.9 |
| SQLite | 3 |
| OpenSSL | 3.6 |
| Argon2id | vcpkg |
| CMake | 4.4 |
| vcpkg | manifest 模式 |
