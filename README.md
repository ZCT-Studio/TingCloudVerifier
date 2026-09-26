<!--
  TingCloudVerifier README (zh-CN)
  Copyright 2026 ZCT-Studio
  Licensed under Apache 2.0
-->

# TingCloudVerifier

> 自托管的授权码 / 卡密验证云服务 —— 为你的游戏、软件、SaaS 提供一体化的授权验证、卡密生成、版本更新、审计追踪能力。

<p align="center">
  <img alt="License" src="https://img.shields.io/badge/license-Apache%202.0-blue.svg">
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-blue.svg">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-brightgreen.svg">
  <img alt="CI" src="https://img.shields.io/badge/CI-GitHub%20Actions-success.svg">
  <a href="README.en-US.md"><img alt="English" src="https://img.shields.io/badge/docs-English-orange.svg"></a>
</p>

---

## ✨ 功能特性

- 🔐 **多层级授权体系** — 超级管理员 / 所有者 / 子用户三级 RBAC
- 🔑 **批量卡密生成** — 支持秒级批量创建、封禁、解封、加时、设永久
- 📱 **客户端验证** — HMAC-SHA256 + timestamp + nonce 重放保护，全链路安全
- ⏱️ **全局计时暂停** — 管理员一键暂停所有卡密计时
- 🔄 **版本更新通道** — 稳定版 / 测试版 / 灰度版多通道推送
- 📝 **完整审计日志** — 每一次登录、卡密操作都有迹可循
- 🛡️ **Argon2id 密码哈希** — 业界抗 GPU 暴力破解最强方案
- 🧩 **SQLite / PostgreSQL** — 零依赖启动，也可扩展生产级数据库
- 📦 **跨平台** — Windows x64/x86/ARM64、Linux x64/x86/ARM64/ARM32、macOS x64/ARM64
- 🎨 **可插拔日志** — ZCLibLog 驱动，支持 stdout + 文件 + 异步 + 自定义 formatter

## 🛠️ 技术栈

| 类别 | 组件 |
|------|------|
| 语言 | C++23 |
| HTTP 框架 | [Drogon](https://github.com/drogonframework/drogon) |
| 数据库 | SQLite（PostgreSQL 预预留） |
| 配置 | yaml-cpp |
| 密码哈希 | Argon2id |
| 加密 | OpenSSL |
| 日志 | [ZCLibLog](https://github.com/ZCT-Studio/ZCLibLog) |
| 包管理 | vcpkg |
| 构建 | CMake 3.25+ |

## 📥 下载

前往 [GitHub Releases](https://github.com/yourname/TingCloudVerifier/releases) 下载对应平台版本。

发布包格式：`TingCloudVerifier_<os>_<arch>.zip` / `.tar.gz`

| 平台 | 架构 | 文件名 |
|------|------|--------|
| Windows Server 2022 | x64 | `TingCloudVerifier_windows-2022_x64.zip` |
| Windows Server 2022 | x86 | `TingCloudVerifier_windows-2022_x86.zip` |
| Windows Server 2022 | ARM64 | `TingCloudVerifier_windows-2022_arm64.zip` |
| Ubuntu LTS | x64 | `TingCloudVerifier_ubuntu-latest_x64.tar.gz` |
| Ubuntu LTS | x86 | `TingCloudVerifier_ubuntu-22.04_x86.tar.gz` |
| Ubuntu LTS | ARM64 | `TingCloudVerifier_ubuntu-latest_arm64.tar.gz` |
| Ubuntu LTS | ARM32 | `TingCloudVerifier_ubuntu-22.04_arm.tar.gz` |
| macOS Sonoma | ARM64 (Apple Silicon) | `TingCloudVerifier_macos-latest_arm64.tar.gz` |
| macOS Ventura | x64 (Intel) | `TingCloudVerifier_macos-13_x64.tar.gz` |

### Tag 规则

- **正式发布**：`v0.1.0`、`v1.0.0`
- **预发布**：`v0.2.0-beta.1`、`v1.0.0-rc.1`（tag 包含 `-` 时自动标记为 prerelease）

## 🚀 快速开始

```bash
# 1. 解压
tar -xzf TingCloudVerifier_ubuntu-latest_x64.tar.gz -C /opt/
cd /opt/TingCloudVerifier

# 2. 复制示例配置
cp config/config.example.yaml config/config.yaml

# 3. 启动
./TingCloudVerifier      # Linux/macOS
.\TingCloudVerifier.exe  # Windows

# 4. 初始化超级管理员（首次启动）
curl -X POST http://127.0.0.1:10211/api/v1/auth/bootstrap-admin \
     -d "username=root&password=YourStrongP@ssword"

# 5. 全链路测试
bash tests/AllTest.sh          # Linux/macOS
tests\AllTest.bat              # Windows
```

## 🧱 从源码构建

### 前置依赖

- CMake **≥ 3.25**
- C++23 编译器（MSVC 2022 / GCC 13+ / Clang 17+）
- [vcpkg](https://github.com/microsoft/vcpkg)

### Windows (MSVC)

```powershell
git clone https://github.com/yourname/TingCloudVerifier.git --recursive
cd TingCloudVerifier
git submodule update --init --recursive

# 安装 vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && .\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg install --triplet=x64-windows-static
cd ..

# 构建
cmake -B build -S . `
  -DCMAKE_TOOLCHAIN_FILE="$(Resolve-Path vcpkg/scripts/buildsystems/vcpkg.cmake)" `
  -DVCPKG_TARGET_TRIPLET=x64-windows-static
cmake --build build --config Release
```

### Linux (GCC)

```bash
sudo apt install build-essential cmake git

git clone https://github.com/yourname/TingCloudVerifier.git --recursive
cd TingCloudVerifier
git submodule update --init --recursive

git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh -disableMetrics
./vcpkg install --triplet=x64-linux
cd ..

cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE="$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=x64-linux
cmake --build build --config Release
```

### macOS (Clang / Apple Silicon)

```bash
brew install cmake git
git clone https://github.com/yourname/TingCloudVerifier.git --recursive
cd TingCloudVerifier

git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh -disableMetrics
./vcpkg install --triplet=arm64-osx
cd ..

cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE="$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_TARGET_TRIPLET=arm64-osx
cmake --build build --config Release
```

## ⚙️ 配置文件

复制 `config/config.example.yaml` 为 `config/config.yaml` 后修改：

```yaml
server:
  host: "0.0.0.0"     # 监听地址
  port: 10211          # 监听端口

database:
  type: "sqlite"                              # sqlite | postgresql
  sqlite_path: "./tcv.db"               # SQLite 文件路径

security:
  admin_security_level: "LAN"                 # LOCAL | LAN | PUBLIC
  replay_window_seconds: 60                   # 重放保护时间窗口
  session_ttl_seconds: 1800                   # Session Token 有效期
  password_hasher: "argon2id"                 # 密码哈希算法

rate_limit:
  enabled: true
  ip_per_sec: 100
  license_per_sec: 10
  login_fail_lock_threshold: 5
  login_fail_lock_seconds: 300

logging:
  level: "INFO"                               # TRACE | DEBUG | INFO | WARN | ERROR | FATAL
  async: true
  file_path: "./logs/tcv.log"
```

## 🏗️ 项目结构

```
TingCloudVerifier/
├── .github/workflows/
│   └── build.yml            # CI/CD：9 平台构建 + tag 自动发布
├── api_examples/            # 每个 API 的完整 cURL + 响应示例（33 个 .md）
│   ├── api.v1.system.ping.md
│   ├── api.v1.auth.admin.login.md
│   └── ...
├── cmake/                   # CMake 辅助模块（可选）
├── config/
│   └── config.example.yaml
├── libs/
│   └── ZCLibLog/            # 日志库（Git submodule）
├── migrations/
│   └── 001_initial.sql      # 数据库初始 schema
├── scripts/                 # 辅助脚本（可选）
├── src/
│   ├── main.cpp             # 入口
│   ├── constants.hpp
│   ├── api/
│   │   ├── response.hpp     # 统一响应格式 Result
│   │   └── v1/
│   │       ├── common.hpp           # wrap() 中间件 + 认证工具
│   │       ├── register_all.hpp     # 全部路由注册
│   │       ├── system/              # /api/v1/system/*
│   │       ├── auth/                # /api/v1/auth/*
│   │       ├── admin/               # /api/v1/admin/*
│   │       ├── app/                 # /api/v1/app/*
│   │       ├── function/            # /api/v1/function/*
│   │       ├── license/             # /api/v1/license/*
│   │       ├── client/license/      # /api/v1/client/license/*
│   │       └── update/              # /api/v1/update/*
│   ├── common/
│   │   ├── logger.hpp       # tcv::logger() 同步/异步单例
│   │   ├── formatters.hpp   # stdcxx20 + 时间戳 + 线程 ID
│   │   └── executors.hpp    # file_and_console（同时落盘 + 控制台）
│   ├── config/              # YAML 配置解析
│   ├── crypto/              # Argon2id / HMAC-SHA256 / AES
│   ├── database/            # SQLite + WAL + recursive_mutex
│   ├── middleware/          # 认证 / 签名 / 限流
│   ├── models/              # 数据结构定义
│   ├── repositories/        # DAO 层
│   └── services/            # 业务逻辑层
├── tests/
│   ├── AllTest.sh           # Bash 一键测试
│   └── AllTest.bat          # Windows CMD 一键测试
├── CMakeLists.txt
├── LICENSE
├── README.md
├── README.en-US.md
└── vcpkg.json
```

## 📖 API 参考

### 统一响应格式

```json
// 成功
{ "code": 0, "data": { ... }, "message": "OK" }

// 失败
{ "code": 40100, "data": null, "message": "Invalid token" }
```

### 路由总览

完整示例请查看 [api_examples/](api_examples/) 目录，每个接口一个 `.md` 文件。

#### System（公开）

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/api/v1/system/ping` | 健康检查 |
| GET | `/api/v1/system/configs` | 公开系统配置 |

#### Auth（公开）

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | `/api/v1/auth/bootstrap-admin` | 初始化超级管理员（仅首次） |
| POST | `/api/v1/auth/admin/login` | 管理员登录 |
| POST | `/api/v1/auth/owner/login` | 所有者登录 |
| POST | `/api/v1/auth/subuser/login` | 子用户登录 |
| POST | `/api/v1/auth/logout` | 登出（使 Session 失效） |

#### Admin（Admin Bearer Token）

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | `/api/v1/admin/owner/create` | 创建所有者账户 |
| POST | `/api/v1/admin/owner/balance` | 查看/调整所有者余额 |
| POST | `/api/v1/admin/license-timer/pause` | 全局暂停卡密计时 |
| POST | `/api/v1/admin/license-timer/resume` | 恢复卡密计时 |
| GET  | `/api/v1/admin/audit/list` | 查询审计日志 |

#### App（Owner Bearer Token）

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | `/api/v1/app/create` | 创建应用 |
| GET  | `/api/v1/app/list` | 所有者的应用列表 |
| POST | `/api/v1/app/secret/regenerate` | 重新生成 App Secret |
| POST | `/api/v1/app/delete` | 删除应用 |

#### Function（Owner Bearer Token）

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | `/api/v1/function/create` | 创建功能/模块 |
| GET  | `/api/v1/function/list` | 查询功能列表 |
| POST | `/api/v1/function/delete` | 删除功能 |

#### License（Owner Bearer Token）

| 方法 | 路径 | 说明 |
|------|------|------|
| POST | `/api/v1/license/create` | 批量创建卡密（返回明文） |
| GET  | `/api/v1/license/list` | 卡密列表 |
| POST | `/api/v1/license/delete` | 删除卡密 |
| POST | `/api/v1/license/ban` | 封禁卡密 |
| POST | `/api/v1/license/unban` | 解封卡密 |
| POST | `/api/v1/license/add-time` | 给卡密延长时长 |
| POST | `/api/v1/license/set-time` | 设置卡密到期时间/永久 |
| POST | `/api/v1/license/unbind` | 解除卡密绑定 |

#### Client（公开）

| 方法 | 路径 | 说明 |
|------|------|------|
| GET/POST | `/api/v1/client/license/verify` | **核心接口**：卡密验证（HMAC 签名 + 重放保护） |
| GET | `/api/v1/client/license/remaining` | 查询剩余秒数 |
| GET | `/api/v1/client/license/status` | 查询卡密状态 |

#### Update（公开 / Owner Bearer）

| 方法 | 路径 | 说明 |
|------|------|------|
| GET  | `/api/v1/update/latest` | 客户端查询最新版本（公开） |
| POST | `/api/v1/update/channel/create` | 创建更新通道（Owner） |
| POST | `/api/v1/update/version/create` | 上传新版本（Owner） |

### 客户端签名算法（`/api/v1/client/license/verify`）

```
# 客户端请求
GET /api/v1/client/license/verify
     ?appid=<APPID>
     &license=<LICENSE_PLAIN>
     &timestamp=<UNIX_SECONDS>      # 允许 ±60s 漂移
     &nonce=<RANDOM_16_HEX>        # 每次唯一，服务端记录防重放

# 可选：客户端用 App Secret 对 query 签名
# signature = HMAC_SHA256(APP_SECRET, "appid=<>&license=<>&timestamp=<>&nonce=<>")
```

## 🧪 测试

```bash
# Linux / macOS
bash tests/AllTest.sh

# Windows
tests\AllTest.bat

# 指定目标
bash tests/AllTest.sh 127.0.0.1 10211
AllTest.bat 127.0.0.1 10211
```

测试覆盖：ping → bootstrap-admin → admin login → owner create → owner login → app create → license create → client verify → ban/unban → audit → logout（共 ~10 个阶段，20+ 次 API 调用）。

## 📊 日志格式

```
2026-09-26 15:44:42.413 [T18045786660363790423] [INFO] [TingCloudVerifier] logger initialized
时间戳(ms)               线程 ID hash                  等级   logger 名字         消息
```

日志同时写入：
- `./logs/tcv.log`（文件）
- `stdout` / `stderr`（控制台，ERROR/FATAL 进 stderr）

## 🤝 贡献

欢迎 PR！请先 fork 并创建 feature 分支，提交时使用 Conventional Commits 风格。

## 📄 许可证

Apache License 2.0 — 详见 [LICENSE](LICENSE) 文件。

## ❤️ 致谢

- [Drogon](https://github.com/drogonframework/drogon) — 现代 C++17/20 HTTP 框架
- [ZCLibLog](https://github.com/ZCT-Studio/ZCLibLog) — 轻量灵活的 C++ 日志库
- [vcpkg](https://github.com/microsoft/vcpkg) — 跨平台 C++ 包管理器