<!--
  TingCloudVerifier README (en-US)
  Copyright 2026 ZCT-Studio
  Licensed under Apache 2.0
-->

# TingCloudVerifier

> A self-hosted license / key verification cloud service — offering unified authorization, key generation, version update push, and audit logging for games, software, and SaaS products.

<p align="center">
  <img alt="License" src="https://img.shields.io/badge/license-Apache%202.0-blue.svg">
  <img alt="C++23" src="https://img.shields.io/badge/C%2B%2B-23-blue.svg">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-brightgreen.svg">
  <img alt="CI" src="https://img.shields.io/badge/CI-GitHub%20Actions-success.svg">
  <a href="README.md"><img alt="中文" src="https://img.shields.io/badge/docs-中文-orange.svg"></a>
</p>

---

## ✨ Features

- 🔐 **Multi-tier Authorization** — Three-level RBAC: Super Admin / Owner / Sub-user
- 🔑 **Batch Key Generation** — Sub-second bulk creation, ban/unban, time extension, permanent flag
- 📱 **Client-side Verification** — HMAC-SHA256 + timestamp + nonce anti-replay, end-to-end secure
- ⏱️ **Global Timer Pause** — Admin can freeze all license timers instantly
- 🔄 **Update Channels** — Stable / Beta / Canary multi-channel version push
- 📝 **Full Audit Logs** — Every login and key operation is traceable
- 🛡️ **Argon2id Password Hashing** — Industry strongest against GPU brute-force
- 🧩 **SQLite / PostgreSQL** — Zero-disk startup today, production-ready tomorrow
- 📦 **Cross-platform** — Windows x64/x86/ARM64, Linux x64/x86/ARM64/ARM32, macOS x64/ARM64
- 🎨 **Pluggable Logging** — ZCLibLog backend, stdout + file + async + custom formatters

## 🛠️ Tech Stack

| Category | Component |
|----------|-----------|
| Language | C++23 |
| HTTP Framework | [Drogon](https://github.com/drogonframework/drogon) |
| Database | SQLite (PostgreSQL slot reserved) |
| Config | yaml-cpp |
| Password Hashing | Argon2id |
| Crypto | OpenSSL |
| Logging | [ZCLibLog](https://github.com/ZCT-Studio/ZCLibLog) |
| Package Manager | vcpkg |
| Build | CMake 3.25+ |

## 📥 Downloads

Get your binary at [GitHub Releases](https://github.com/yourname/TingCloudVerifier/releases).

Archive format: `TingCloudVerifier_<os>_<arch>.zip` / `.tar.gz`

| Platform | Arch | Filename |
|----------|------|----------|
| Windows Server 2022 | x64 | `TingCloudVerifier_windows-2022_x64.zip` |
| Windows Server 2022 | x86 | `TingCloudVerifier_windows-2022_x86.zip` |
| Windows Server 2022 | ARM64 | `TingCloudVerifier_windows-2022_arm64.zip` |
| Ubuntu LTS | x64 | `TingCloudVerifier_ubuntu-latest_x64.tar.gz` |
| Ubuntu LTS | x86 | `TingCloudVerifier_ubuntu-22.04_x86.tar.gz` |
| Ubuntu LTS | ARM64 | `TingCloudVerifier_ubuntu-latest_arm64.tar.gz` |
| Ubuntu LTS | ARM32 | `TingCloudVerifier_ubuntu-22.04_arm.tar.gz` |
| macOS Sonoma | ARM64 (Apple Silicon) | `TingCloudVerifier_macos-latest_arm64.tar.gz` |
| macOS Ventura | x64 (Intel) | `TingCloudVerifier_macos-13_x64.tar.gz` |

### Tag Convention

- **Stable releases**: `v0.1.0`, `v1.0.0`
- **Prereleases** (tag contains `-`): `v0.2.0-beta.1`, `v1.0.0-rc.1` — auto-marked as prerelease by the CI workflow.

## 🚀 Quick Start

```bash
# 1. Extract
tar -xzf TingCloudVerifier_ubuntu-latest_x64.tar.gz -C /opt/
cd /opt/TingCloudVerifier

# 2. Copy example config
cp config/config.example.yaml config/config.yaml

# 3. Run
./TingCloudVerifier      # Linux/macOS
.\TingCloudVerifier.exe  # Windows

# 4. Bootstrap super admin (first run only)
curl -X POST http://127.0.0.1:10211/api/v1/auth/bootstrap-admin \
     -d "username=root&password=YourStrongP@ssword"

# 5. Full end-to-end test
bash tests/AllTest.sh          # Linux/macOS
tests\AllTest.bat              # Windows
```

## 🧱 Building from Source

### Prerequisites

- CMake **≥ 3.25**
- C++23 compiler (MSVC 2022 / GCC 13+ / Clang 17+)
- [vcpkg](https://github.com/microsoft/vcpkg)

### Windows (MSVC)

```powershell
git clone https://github.com/yourname/TingCloudVerifier.git --recursive
cd TingCloudVerifier
git submodule update --init --recursive

# Install vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && .\bootstrap-vcpkg.bat -disableMetrics
.\vcpkg install --triplet=x64-windows-static
cd ..

# Build
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

## ⚙️ Configuration

Copy `config/config.example.yaml` → `config/config.yaml` and tweak:

```yaml
server:
  host: "0.0.0.0"
  port: 10211

database:
  type: "sqlite"
  sqlite_path: "./tcv.db"

security:
  admin_security_level: "LAN"          # LOCAL | LAN | PUBLIC
  replay_window_seconds: 60            # Anti-replay window
  session_ttl_seconds: 1800            # Session token TTL
  password_hasher: "argon2id"

rate_limit:
  enabled: true
  ip_per_sec: 100
  license_per_sec: 10
  login_fail_lock_threshold: 5
  login_fail_lock_seconds: 300

logging:
  level: "INFO"                        # TRACE | DEBUG | INFO | WARN | ERROR | FATAL
  async: true
  file_path: "./logs/tcv.log"
```

## 🏗️ Project Structure

```
TingCloudVerifier/
├── .github/workflows/
│   └── build.yml            # CI/CD: 9-platform build + auto-release on tag
├── api_examples/            # One markdown file per endpoint (cURL + response)
│   ├── api.v1.system.ping.md
│   ├── api.v1.auth.admin.login.md
│   └── ...
├── cmake/                   # Optional CMake modules
├── config/
│   └── config.example.yaml
├── libs/
│   └── ZCLibLog/            # Logging library (Git submodule)
├── migrations/
│   └── 001_initial.sql
├── scripts/                 # Helper scripts (optional)
├── src/
│   ├── main.cpp
│   ├── constants.hpp
│   ├── api/
│   │   ├── response.hpp     # Unified response Result<T>
│   │   └── v1/
│   │       ├── common.hpp           # wrap() middleware + auth helpers
│   │       ├── register_all.hpp     # Route registration
│   │       ├── system/
│   │       ├── auth/
│   │       ├── admin/
│   │       ├── app/
│   │       ├── function/
│   │       ├── license/
│   │       ├── client/license/
│   │       └── update/
│   ├── common/
│   │   ├── logger.hpp       # tcv::logger() sync/async singleton
│   │   ├── formatters.hpp   # stdcxx20 + timestamp + thread ID
│   │   └── executors.hpp    # file_and_console executor
│   ├── config/
│   ├── crypto/
│   ├── database/
│   ├── middleware/
│   ├── models/
│   ├── repositories/
│   └── services/
├── tests/
│   ├── AllTest.sh           # Bash end-to-end test
│   └── AllTest.bat          # Windows CMD end-to-end test
├── CMakeLists.txt
├── LICENSE
├── README.md
├── README.en-US.md
└── vcpkg.json
```

## 📖 API Reference

### Unified Response

```json
// Success
{ "code": 0, "data": { ... }, "message": "OK" }

// Error
{ "code": 40100, "data": null, "message": "Invalid token" }
```

### Endpoints

Every endpoint has a full cURL + response example in the [api_examples/](api_examples/) directory.

#### System (Public)

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/v1/system/ping` | Health check |
| GET | `/api/v1/system/configs` | Public system configuration |

#### Auth (Public)

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/auth/bootstrap-admin` | Initialize super admin (first run only) |
| POST | `/api/v1/auth/admin/login` | Admin login |
| POST | `/api/v1/auth/owner/login` | Owner login |
| POST | `/api/v1/auth/subuser/login` | Sub-user login |
| POST | `/api/v1/auth/logout` | Logout (invalidate current session) |

#### Admin (Admin Bearer Token)

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/admin/owner/create` | Create owner account |
| POST | `/api/v1/admin/owner/balance` | View/adjust owner balance |
| POST | `/api/v1/admin/license-timer/pause` | Globally pause all license timers |
| POST | `/api/v1/admin/license-timer/resume` | Resume license timers |
| GET  | `/api/v1/admin/audit/list` | Query audit logs |

#### App (Owner Bearer Token)

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/app/create` | Create an app |
| GET  | `/api/v1/app/list` | Owner's app list |
| POST | `/api/v1/app/secret/regenerate` | Rotate App Secret |
| POST | `/api/v1/app/delete` | Delete an app |

#### Function (Owner Bearer Token)

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/function/create` | Create a function/module |
| GET  | `/api/v1/function/list` | List functions |
| POST | `/api/v1/function/delete` | Delete a function |

#### License (Owner Bearer Token)

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/license/create` | Batch create licenses (returns plaintext) |
| GET  | `/api/v1/license/list` | List licenses |
| POST | `/api/v1/license/delete` | Delete a license |
| POST | `/api/v1/license/ban` | Ban a license |
| POST | `/api/v1/license/unban` | Unban a license |
| POST | `/api/v1/license/add-time` | Extend license duration (seconds) |
| POST | `/api/v1/license/set-time` | Set expiration or permanent |
| POST | `/api/v1/license/unbind` | Unbind a license from a device |

#### Client (Public)

| Method | Path | Description |
|--------|------|-------------|
| GET/POST | `/api/v1/client/license/verify` | **Core endpoint**: license verification (HMAC + anti-replay) |
| GET | `/api/v1/client/license/remaining` | Query remaining seconds |
| GET | `/api/v1/client/license/status` | Query license status |

#### Update (Public / Owner Bearer)

| Method | Path | Description |
|--------|------|-------------|
| GET  | `/api/v1/update/latest` | Client queries latest version (public) |
| POST | `/api/v1/update/channel/create` | Create update channel (Owner) |
| POST | `/api/v1/update/version/create` | Upload new version (Owner) |

### Client Signature Scheme (`/api/v1/client/license/verify`)

```
# Client request
GET /api/v1/client/license/verify
     ?appid=<APPID>
     &license=<LICENSE_PLAIN>
     &timestamp=<UNIX_SECONDS>      # ±60s drift tolerated
     &nonce=<RANDOM_16_HEX>         # Unique per request, server records to prevent replay

# Optional: client signs query with App Secret
# signature = HMAC_SHA256(APP_SECRET, "appid=<>&license=<>&timestamp=<>&nonce=<>")
```

## 🧪 Testing

```bash
# Linux / macOS
bash tests/AllTest.sh

# Windows
tests\AllTest.bat

# Target override
bash tests/AllTest.sh 127.0.0.1 10211
AllTest.bat 127.0.0.1 10211
```

Coverage: ping → bootstrap-admin → admin login → owner create → owner login → app create → license create → client verify → ban/unban → audit → logout (~10 phases, 20+ API calls).

## 📊 Log Format

```
2026-09-26 15:44:42.413 [T18045786660363790423] [INFO] [TingCloudVerifier] logger initialized
Timestamp (ms)           Thread ID hash              Level  Logger Name         Message
```

Log goes to both:
- `./logs/tcv.log` (file)
- `stdout` / `stderr` (console, ERROR/FATAL → stderr)

## 🤝 Contributing

PRs welcome! Please fork, create a feature branch, and use Conventional Commits style in your commits.

## 📄 License

Apache License 2.0 — see [LICENSE](LICENSE).

## ❤️ Acknowledgements

- [Drogon](https://github.com/drogonframework/drogon) — Modern C++17/20 HTTP framework
- [ZCLibLog](https://github.com/ZCT-Studio/ZCLibLog) — Lightweight & flexible C++ logger
- [vcpkg](https://github.com/microsoft/vcpkg) — Cross-platform C++ package manager