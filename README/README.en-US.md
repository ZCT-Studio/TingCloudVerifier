# TingCloudVerifier

A **Drogon + SQLite + OpenSSL** multi-platform software licensing server featuring license keys, device/IP binding, AES256-GCM encrypted transport, optional HMAC-SHA256 request signing, app-level announcement and version management — all shipped with a 9-platform CI release pipeline.

> Early-stage, actively evolving. Breaking API changes are documented in GitHub Releases.

## Features

- **Three auth roles**: ADMIN (super root), OWNER (app creator), SUBUSER (team member)
- **Multi-app** per OWNER, each with a globally-unique `appid`
- **License keys**: create, ban/unban, time-adjust, time-extend, delete, arbitrary license types
- **Binding modes**: `NONE` / `IP` / `DEVICE` / `IP_AND_DEVICE` (independent from app)
- **Optional encrypted transport**: per-app request/response encryption — `AES256-GCM` / `BASE64` / `HEX`
- **Optional HMAC-SHA256 signing**: per-app, `timestamp` + `nonce` anti-replay
- **Client API security layer**: `timestamp` / `nonce` / `signature` / `encode` four required elements
- **Announcements**: OWNER posts a notice, client fetches with one call
- **Version channels**: `stable` / `rc` / `canary` / `alpha` auto-created per app
- **9-platform CI**: Windows x64/x86/arm64, Linux x64/x86/arm64/armv7, macOS arm64/x64

## Tech Stack

| Area | Choice |
|---|---|
| Language | C++23 (MSVC 19.40+ / GCC 13+ / Clang 17+) |
| Web framework | Drogon 1.9 |
| Database | SQLite 3 (`unofficial-sqlite3` from vcpkg) |
| Crypto | OpenSSL 3.6 (AES256-GCM / SHA256 / HMAC) |
| Password hashing | Argon2id (`unofficial-argon2` from vcpkg) |
| Build | CMake 4.4 + vcpkg manifest |
| CI | GitHub Actions (9-platform matrix) |
| Docs | VitePress (GitHub Pages) |

## Layout

```
TingCloudVerifier/
├── CMakeLists.txt
├── vcpkg.json
├── migrations/            # SQL migration scripts
│   ├── initial.sql
│   └── 003_client_security.sql
├── config/                # runtime config (copied to dist/config/)
│   └── config.yaml
├── src/
│   ├── main.cpp
│   ├── common/            # logger, shared types
│   ├── crypto/            # AES / HMAC / SHA / Base64 / Hex helpers
│   ├── database/          # SQLite wrapper
│   ├── models/            # App / License / Session structs
│   ├── repositories/      # data access layer
│   ├── services/          # business logic
│   └── api/
│       ├── response.hpp   # {code, message, data} envelope
│       └── v1/
│           ├── common.hpp # wrap() middleware, paramStr(), crypto helpers
│           ├── auth/      # /api/v1/auth/*   login/logout/bootstrap
│           ├── admin/     # /api/v1/admin/*  super admin ops
│           ├── app/       # /api/v1/app/*    OWNER app management
│           ├── license/   # /api/v1/license/* OWNER key management
│           ├── client/    # /api/v1/client/* public client API
│           └── register_all.hpp
├── README.md
├── README/
│   └── README.en-US.md
├── docs/                  # VitePress docs site
└── .github/
    └── workflows/
        ├── build.yml      # 9-platform build + release
        └── docs.yml       # VitePress → GitHub Pages
```

## Quick Start

### Requirements

- **Windows**: Visual Studio 2022 (MSVC 19.40+)
- **Linux**: GCC 13+
- **macOS**: Xcode 15.4+
- **CMake**: 4.0+
- **vcpkg**: latest (auto-cloned during build)

### Build

```bash
# 1. Install toolchain
#    Windows:   PowerShell as Administrator → Install OpenSSL
#    Linux:     sudo apt install build-essential cmake git
#    macOS:     xcode-select --install

# 2. Clone
git clone --recurse-submodules https://github.com/<owner>/TingCloudVerifier.git
cd TingCloudVerifier

# 3. Configure + Build (vcpkg pulls all deps automatically)
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j

# 4. Run
cd build
./TingCloudVerifier        # Linux/macOS
TingCloudVerifier.exe      # Windows
```

### First Run

```bash
# Migrations auto-executed in order from migrations/*.sql
# Bootstrap admin account created (see bootstrap-admin endpoint)
```

## Authentication

All `/api/v1/admin/*` and `/api/v1/{app,license}/*` routes require:

```
Authorization: Bearer <token>
```

Token returned by `POST /api/v1/auth/{admin|owner|subuser}/login`. Stored in `sessions` table.

## Client API Security

### Four Required Fields

All `/api/v1/client/*` requests must include:

| Field | Description |
|---|---|
| `appid` | globally unique app ID |
| `timestamp` | Unix seconds (default ±300s window) |
| `nonce` | random string, server enforces window-level uniqueness |
| `signature` | optional (mandatory when app `sign_enable=1`): HMAC-SHA256 |

### Signature Algorithm

```
canonical = METHOD + PATH + appid + timestamp + nonce + encode(optional)
signature = HMAC_SHA256_HEX(canonical, app.secret)
```

### Optional Request/Response Encryption

Per-app granularity (`app.dec_mode`). All `/api/v1/client/*` requests and responses are encrypted **symmetrically** — if the request is encrypted, the response uses the same algorithm.

| dec_mode | Request `encode` encoding | Response `encode` encoding |
|---|---|---|
| `NONE` | plain params | plain JSON |
| `AES256-GCM` | base64([12B nonce][ciphertext][16B tag]), plaintext is URL `k=v` | same format, plaintext is full JSON response |
| `BASE64` | base64(URL `k=v`) | base64(JSON response) |
| `HEX` | hex(URL `k=v`) | hex(JSON response) |

`dec_key`: 64-char hex (32 bytes), **AES256-GCM only**. BASE64/HEX ignore it.

> Encryption is handled in the `wrap()` middleware — handlers contain **zero crypto code**.

## API Summary

### Auth

| Method | Path | Description |
|---|---|---|
| POST | `/api/v1/auth/bootstrap-admin` | One-time admin bootstrap |
| POST | `/api/v1/auth/admin/login` | Admin login |
| POST | `/api/v1/auth/owner/login` | OWNER login |
| POST | `/api/v1/auth/subuser/login` | SUBUSER login |
| POST | `/api/v1/auth/logout` | Invalidate session |

### Admin

| Method | Path | Description |
|---|---|---|
| POST | `/api/v1/admin/owner/create` | Create OWNER account |
| POST | `/api/v1/admin/owner/balance` | Adjust OWNER balance |
| GET  | `/api/v1/admin/audit/list` | Audit log |
| POST | `/api/v1/admin/license-timer/pause` | Global pause |
| POST | `/api/v1/admin/license-timer/resume` | Global resume |

### App Management

| Method | Path | Description |
|---|---|---|
| POST | `/api/v1/app/create` | Create app |
| GET  | `/api/v1/app/list` | List OWNER's apps |
| POST | `/api/v1/app/delete` | Delete app |
| POST | `/api/v1/app/binding` | Set default binding mode |
| POST | `/api/v1/app/secret/regenerate` | Rotate HMAC secret |
| POST | `/api/v1/app/set-decode` | Configure `dec_mode` / `dec_key` |
| POST | `/api/v1/app/sign-enable` | Toggle signing requirement |
| POST | `/api/v1/app/notice` | Set/update announcement |
| GET  | `/api/v1/app/channel/list` | List version channels |
| POST | `/api/v1/app/version/create` | Upload a version |
| POST | `/api/v1/app/version/latest` | Latest version by channel |

### License Keys

| Method | Path | Description |
|---|---|---|
| POST | `/api/v1/license/create` | Create keys |
| GET  | `/api/v1/license/list` | List with filters + pagination |
| POST | `/api/v1/license/set-time` | Adjust expiry |
| POST | `/api/v1/license/add-time` | Extend duration |
| POST | `/api/v1/license/set-type` | Change `license_type` |
| POST | `/api/v1/license/ban` | Ban |
| POST | `/api/v1/license/unban` | Unban |
| POST | `/api/v1/license/unbind` | Unbind device/IP |
| POST | `/api/v1/license/unbind-info` | What can be unbound |
| POST | `/api/v1/license/delete` | Delete |

### Client (Public)

Requires the four security fields above. **No Bearer token**.

| Method | Path | Description |
|---|---|---|
| POST | `/api/v1/client/license/verify` | Verify/activate (core) |
| POST | `/api/v1/client/license/status` | License status |
| POST | `/api/v1/client/license/remaining` | Remaining duration |
| POST | `/api/v1/client/app/channels` | Get available channels |
| POST | `/api/v1/client/app/version` | Check for updates |
| POST | `/api/v1/client/app/notice` | Fetch announcement |

## Database

Migrations live in `migrations/`, applied on first start. Main tables:

```
apps           — globally unique appid, dec_mode/dec_key/sign_enable/notice
licenses       — keys with binding info (IP hash / device hash / unbind count)
sessions       — unified (admin / owner / subuser)
update_channels — stable/rc/canary/alpha auto-seeded per app
nonces         — anti-replay uniqueness (appid + nonce)
ip_whitelist   — LOCAL security level
audit_logs     — audit trail
```

## Build Artifacts

Each GitHub Release contains 9 archives with a unified layout:

```
TingCloudVerifier-<arch>-<os>/
├── TingCloudVerifier(.exe)
├── config/
├── migrations/
├── README/
│   ├── README.zh-CN.md      # root README.md renamed
│   └── README.en-US.md      # English README
└── LICENSE.TXT
```

## Roadmap

- [ ] SUBUSER team collaboration (OWNER assigns keys to SUBUSER)
- [ ] Multi-node deploy + Redis-backed nonce store
- [ ] Client SDKs (C++ / Go / C# wrappers)
- [ ] Web console (Vue + Drogon backend)
- [ ] Prometheus metrics + OpenTelemetry

## License

Apache License 2.0 — see [LICENSE](./LICENSE).

---

[中文版](../README.md) · [Full API Docs](https://<owner>.github.io/TingCloudVerifier/)
