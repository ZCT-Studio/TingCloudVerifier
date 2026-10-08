---
layout: home

hero:
  name: TingCloudVerifier
  text: 多平台软件授权验证服务端
  tagline: C++23 · Drogon · SQLite/PostgreSQL/MySQL · 卡密系统 · 签名防重放 · AES256-GCM 加密
  actions:
    - theme: brand
      text: 快速上手
      link: /api/overview
    - theme: alt
      text: 客户端安全
      link: /client/security
    - theme: alt
      text: 数据库配置
      link: /database/overview

features:
  - icon: 🔐
    title: 多层安全
    details: 四要素签名 + timestamp/nonce 防重放，AES256-GCM / BASE64 / HEX 请求响应对称加密，Argon2id 密码哈希

  - icon: 🗝️
    title: 卡密生命周期
    details: 创建 / 封禁 / 解绑 / 重置，支持 NONE / IP / DEVICE / IP_AND_DEVICE 四种绑定模式，首次使用自动绑定

  - icon: 🗄️
    title: 三后端
    details: SQLite（默认，零配置）/ PostgreSQL / MariaDB，同一套 migration 支持三方言，vcpkg feature 切换

  - icon: 📡
    title: 四版本通道
    details: stable / rc / canary / alpha，客户端自动拉取对应通道最新版本号 + 更新公告

  - icon: 🧩
    title: 中间件加密
    details: wrap() 中间件统一处理四要素校验、解密、签名，Handler 代码零加密依赖，业务逻辑与安全层解耦

  - icon: 🏗️
    title: 9 平台 CI
    details: Windows x64/x86/arm64 · Linux x64/x86/arm64/armv7 · macOS arm64/x64，全平台 Release 构建 + pg/mysql 回归
---

## 快速导航

<div class="vp-doc">

- **[API 概览](/api/overview)** — 全部端点速查表
- **[客户端安全层](/client/security)** — 四要素、签名、加密详解
- **[加密传输](/client/encryption)** — AES256-GCM / BASE64 / HEX 请求响应对称加密
- **[签名校验](/client/signing)** — HMAC-SHA256 canonical 串算法
- **[完整示例](/client/examples)** — Python / Go / curl 客户端示例
- **[数据库概览](/database/overview)** — SQLite / PG / MySQL 配置、Migration、方言对照

</div>
