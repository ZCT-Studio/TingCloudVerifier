# TingCloudVerifier

一个基于 **Drogon + SQLite + OpenSSL** 的多平台软件授权验证服务端，支持卡密生成、设备/IP 绑定、AES256-GCM 加密传输、HMAC-SHA256 签名校验、可选客户端密钥/公告/版本管理，以及多架构多平台发布流水线。

> 项目早期，持续迭代中。API 变更会在 GitHub Releases 里说明。

## 特性

- **三种授权角色**：ADMIN（超级管理员）、OWNER（应用所有者）、SUBUSER（子用户/团队成员）
- **多应用管理**：每个 OWNER 可创建多个独立 APP，APPID 全局唯一
- **卡密系统**：创建、封禁/解封、重置时间、延长时长、删除、自由卡密类型
- **多种绑定模式**：NONE / IP / DEVICE / IP_AND_DEVICE（卡密可与任意 APP 不同）
- **可选加密传输**：按 APP 粒度配置请求/响应加密（AES256-GCM / BASE64 / HEX）
- **可选 HMAC-SHA256 签名校验**：按 APP 粒度开启，timestamp + nonce 防重放
- **客户端密钥下发**：timestamp / nonce / signature / encode 四要素
- **公告系统**：OWNER 可给 APP 设置公告，客户端一键拉取
- **版本通道**：stable / rc / canary / alpha 四默认通道，OWNER 上传版本后客户端检查更新
- **多架构 CI**：Windows x64/x86/arm64、Linux x64/x86/arm64/armv7、macOS arm64/x64 — 9 份 artifact

## 技术栈

| 分类 | 选型 |
|---|---|
| 语言 | C++23（MSVC 19.40+ / GCC 13+ / Clang 17+） |
| Web 框架 | Drogon 1.9 |
| 数据库 | SQLite 3（vcpkg `unofficial-sqlite3`） |
| 加密 | OpenSSL 3.6（AES256-GCM / SHA256 / HMAC） |
| 密码哈希 | Argon2id（vcpkg `unofficial-argon2`） |
| 构建系统 | CMake 4.4 + vcpkg manifest |
| CI | GitHub Actions（matrix 9 平台） |
| 文档 | VitePress（GitHub Pages） |

## 目录结构

```
TingCloudVerifier/
├── CMakeLists.txt
├── vcpkg.json
├── migrations/            # SQL 迁移脚本
│   ├── initial.sql        # 基础表
│   └── 003_client_security.sql
├── config/                # 运行时配置（复制到 dist/config/）
│   └── config.yaml
├── src/
│   ├── main.cpp
│   ├── common/            # 日志、类型
│   ├── crypto/            # AES / HMAC / SHA / Base64 / Hex
│   ├── database/          # SQLite 封装
│   ├── models/            # 数据结构（App / License / Session ...）
│   ├── repositories/      # 数据访问层
│   ├── services/          # 业务逻辑层
│   └── api/
│       ├── response.hpp   # 统一响应 {code, message, data}
│       └── v1/
│           ├── common.hpp # wrap() 中间件 / paramStr() / 加密辅助
│           ├── auth/      # /api/v1/auth/*  登录/登出/引导
│           ├── admin/     # /api/v1/admin/* 超级管理员
│           ├── app/       # /api/v1/app/*   OWNER 应用管理
│           ├── license/   # /api/v1/license/* OWNER 卡密管理
│           ├── client/    # /api/v1/client/* 客户端 API
│           └── register_all.hpp
├── README.md
├── README/
│   └── README.en-US.md
├── docs/                  # VitePress 文档站点
└── .github/
    └── workflows/
        ├── build.yml      # 9 平台构建 + release
        └── docs.yml       # VitePress 部署到 Pages
```

## 快速开始

### 环境要求

- **Windows**：Visual Studio 2022（MSVC 19.40+）
- **Linux**：GCC 13+
- **macOS**：Xcode 15.4+
- **CMake**：4.0+
- **vcpkg**：最新（build 时会自动 clone）

### 构建

```bash
# 1. 安装构建工具（Windows: 用 PowerShell 管理员安装 OpenSSL）
#    Linux: apt install build-essential cmake git
#    macOS: xcode-select --install

# 2. Clone 项目
git clone --recurse-submodules https://github.com/<owner>/TingCloudVerifier.git
cd TingCloudVerifier

# 3. Configure + Build（vcpkg 会自动拉依赖）
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j

# 4. 运行
cd build
./TingCloudVerifier        # Linux/macOS
TingCloudVerifier.exe      # Windows
```

### 首次启动

```bash
# 初始化数据库（自动执行 migrations/*.sql）
# 引导 Admin 账户（第一次启动会在数据库里留一条 bootstrap 记录）
```

## 认证体系

所有 `/api/v1/admin/*` 和 `/api/v1/{app,license}/*` 请求需要 HTTP Header：

```
Authorization: Bearer <token>
```

token 由 `POST /api/v1/auth/{admin|owner|subuser}/login` 返回，存储在 `sessions` 表中。

## 客户端安全层

### 四要素

所有 `/api/v1/client/*` 请求必须携带：

| 参数 | 说明 |
|---|---|
| `appid` | APP 唯一标识 |
| `timestamp` | Unix 秒级时间戳（默认 ±300s 窗口） |
| `nonce` | 随机字符串，服务器侧防重放（默认同窗口内不可重复） |
| `signature` | 可选（APP `sign_enable=1` 时必填）：HMAC-SHA256 |

### 签名算法

```
canonical = METHOD + PATH + appid + timestamp + nonce + encode(optional)
signature = HMAC_SHA256_HEX(canonical, app.secret)
```

### 可选请求/响应加密

APP 级粒度（`app.dec_mode`），所有 `/api/v1/client/*` 请求/响应同步生效：

| dec_mode | 请求 encode 编码 | 响应 encode 编码 |
|---|---|---|
| `NONE` | 明文参数 | 明文 JSON |
| `AES256-GCM` | base64([12B nonce][ct][16B tag]) 明文是 URL k=v | 同格式，明文是完整 JSON 响应 |
| `BASE64` | base64(URL k=v) | base64(JSON 响应) |
| `HEX` | hex(URL k=v) | hex(JSON 响应) |

`dec_key`：AES256-GCM 专用，64 字符 hex（32 字节）。BASE64/HEX 不需要。

> 加密在 `wrap()` 中间件里统一处理——handler 内**零加密代码**，业务逻辑与传输层完全解耦。

## API 概览

### 认证

| Method | Path | 说明 |
|---|---|---|
| POST | `/api/v1/auth/bootstrap-admin` | 超级管理员引导（生产一次性） |
| POST | `/api/v1/auth/admin/login` | Admin 登录 |
| POST | `/api/v1/auth/owner/login` | OWNER 登录 |
| POST | `/api/v1/auth/subuser/login` | SUBUSER 登录 |
| POST | `/api/v1/auth/logout` | 注销当前 session |

### Admin

| Method | Path | 说明 |
|---|---|---|
| POST | `/api/v1/admin/owner/create` | 创建 OWNER 账户 |
| POST | `/api/v1/admin/owner/balance` | 调整 OWNER 余额 |
| GET  | `/api/v1/admin/audit/list` | 审计日志列表 |
| POST | `/api/v1/admin/license-timer/pause` | 全局暂停卡密计时 |
| POST | `/api/v1/admin/license-timer/resume` | 恢复卡密计时 |

### APP 管理

| Method | Path | 说明 |
|---|---|---|
| POST | `/api/v1/app/create` | 创建 APP |
| GET  | `/api/v1/app/list` | 列出 OWNER 名下 APP |
| POST | `/api/v1/app/delete` | 删除 APP |
| POST | `/api/v1/app/binding` | 设置 APP 默认绑定模式 |
| POST | `/api/v1/app/secret/regenerate` | 重新生成 HMAC secret |
| POST | `/api/v1/app/set-decode` | 设置 dec_mode / dec_key |
| POST | `/api/v1/app/sign-enable` | 开关签名校验 |
| POST | `/api/v1/app/notice` | 设置/更新公告 |
| GET  | `/api/v1/app/channel/list` | 列出 APP 的版本通道 |
| POST | `/api/v1/app/version/create` | 上传新版本 |
| POST | `/api/v1/app/version/latest` | 按频道查询最新版 |

### 卡密

| Method | Path | 说明 |
|---|---|---|
| POST | `/api/v1/license/create` | 创建卡密 |
| GET  | `/api/v1/license/list` | 卡密列表（支持过滤/分页） |
| POST | `/api/v1/license/set-time` | 调整到期时间 |
| POST | `/api/v1/license/add-time` | 延长时长 |
| POST | `/api/v1/license/set-type` | 修改 license_type |
| POST | `/api/v1/license/ban` | 封禁 |
| POST | `/api/v1/license/unban` | 解封 |
| POST | `/api/v1/license/unbind` | 解绑设备/IP |
| POST | `/api/v1/license/unbind-info` | 查看可解绑信息 |
| POST | `/api/v1/license/delete` | 删除 |

### 客户端（公开）

所有 client API 都受四要素 + 可选加密/签名保护，**不要求 Bearer Token**。

| Method | Path | 说明 |
|---|---|---|
| POST | `/api/v1/client/license/verify` | 激活/验证（核心接口） |
| POST | `/api/v1/client/license/status` | 查询卡密状态 |
| POST | `/api/v1/client/license/remaining` | 查询剩余时长 |
| POST | `/api/v1/client/app/channels` | 获取 APP 可用版本通道 |
| POST | `/api/v1/client/app/version` | 按频道检查最新版 |
| POST | `/api/v1/client/app/notice` | 获取 APP 公告 |

## 数据库

项目迁移脚本位于 `migrations/`，首次启动自动顺序执行。当前三张业务主表：

```
apps           — 应用（APPID 全局唯一，有 dec_mode/dec_key/sign_enable/notice）
licenses       — 卡密（含绑定信息：IP hash / device hash / 解绑次数）
sessions       — 用户会话（admin/owner/subuser 统一存储）
update_channels — 版本通道（APP 创建时自动建 stable/rc/canary/alpha 四条）
nonces         — 防重放 nonce（APPID + nonce 唯一）
ip_whitelist   — LOCAL 安全等级白名单
audit_logs     — 审计日志
```

## 构建产物

每个 Release 自动包含 9 份独立归档，目录结构统一：

```
TingCloudVerifier-<arch>-<os>/
├── TingCloudVerifier(.exe)
├── config/
├── migrations/
├── README/
│   ├── README.zh-CN.md      # 根目录 README.md 改名
│   └── README.en-US.md      # 英文 README
└── LICENSE.TXT
```

## 路线图

- [ ] Subuser 团队协作（OWNER 把卡密分配给 SUBUSER）
- [ ] 多节点部署 + Redis 存储 nonce（替代 SQLite）
- [ ] 客户端 SDK（C++ / Go / C# 封装）
- [ ] Web 控制台（Vue + Drogon 后端）
- [ ] Prometheus metrics + OpenTelemetry 接入

## 许可证

Apache License 2.0 — 见 [LICENSE](./LICENSE)。

---

[English version](./README/README.en-US.md) · [完整 API 文档](https://<owner>.github.io/TingCloudVerifier/)
