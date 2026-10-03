# 客户端安全层

所有 `/api/v1/client/*` 请求（6 个公开端点）都必须携带**四要素**。这四要素在 `common.hpp::wrap()` 中间件里统一校验，Handler 层完全不感知。

## 四要素

| 参数 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `appid` | string | ✅ | APP 唯一标识 |
| `timestamp` | int64 | ✅ | Unix 秒级时间戳 |
| `nonce` | string | ✅ | 随机字符串（建议 16 字节 hex 或 base64） |
| `signature` | string | 条件必填 | APP `sign_enable=1` 时必填 |

参数可放 **query string**、**form body**、**JSON body** 或 **HTTP header**，`paramStr()` 会按顺序查找。

## timestamp 窗口

默认 ±300 秒（5 分钟）。超过窗口的请求返回 `TIMESTAMP_BAD`。可通过 `config.yaml` 的 `security.replay_window_seconds` 调整。

```yaml
security:
  replay_window_seconds: 300
```

## nonce 防重放

同一个 APP 在同一个 `replay_window` 时间窗口内，`nonce` 不允许重复。服务器把 `(app_id, nonce, timestamp)` 存进 `nonces` 表（CREATE UNIQUE INDEX），请求到的时候先 INSERT 尝试成功才能继续，冲突返回 `NONCE_REPLAY`。

## APP 级粒度

安全层配置全部在 APP 上存：

```sql
ALTER TABLE apps ADD COLUMN sign_enable INTEGER DEFAULT 0;
ALTER TABLE apps ADD COLUMN dec_mode TEXT DEFAULT 'NONE';
ALTER TABLE apps ADD COLUMN dec_key TEXT DEFAULT '';
```

| 字段 | 类型 | 默认 | 说明 |
|---|---|---|---|
| `sign_enable` | int | 0 | 0 = 关闭签名校验，1 = 强制 HMAC-SHA256 |
| `dec_mode` | text | 'NONE' | NONE / AES256-GCM / BASE64 / HEX |
| `dec_key` | text | '' | AES256-GCM 专用，64 字符 hex |

这意味着 **同一个服务器上不同 APP 可以有完全不同的安全策略**。OWNER 通过 `POST /api/v1/app/sign-enable` 和 `POST /api/v1/app/set-decode` 动态调整，无需重启。

## 统一错误码

所有 client API 都使用统一错误码：

| code | 常量 | 说明 |
|---|---|---|
| 0 | OK | 成功 |
| 1000 | BAD_REQUEST | 参数问题 |
| 1001 | APP_NOT_FOUND | APPID 不存在 |
| 1002 | APP_DISABLED | APP 已禁用 |
| 2001 | TIMESTAMP_BAD | timestamp 缺失或超窗口 |
| 2002 | NONCE_REPLAY | nonce 重复 |
| 2003 | SIGNATURE_BAD | 签名缺失或校验失败 |
| 3001 | LICENSE_NOT_FOUND | 卡密不存在 |
| 3002 | LICENSE_BANNED | 卡密已封禁 |
| 3003 | LICENSE_EXPIRED | 卡密已过期 |
| 3004 | LICENSE_BINDING_FAIL | 绑定校验失败 |
| 9999 | INTERNAL | 服务器内部错误 |

## wrap() 中间件流程

```
请求进来
  │
  ├─ timestamp 是否存在？    NO → TIMESTAMP_BAD
  ├─ timestamp 是否在窗口内？  NO → TIMESTAMP_BAD
  ├─ 查 app（by appid）
  │
  ├─ 如果 app.dec_mode != NONE 且有 encode 参数：
  │    └─ 解密 encode → URL k=v → 存 req.attributes["tcv.decoded_kv"]
  │    └─ 解密失败 → BAD_REQUEST
  │
  ├─ 如果有 nonce 且 app 存在：
  │    └─ tryUseNonce() → NONCE_REPLAY
  │
  ├─ 如果 app.sign_enable == 1：
  │    └─ canonical = METHOD + PATH + appid + timestamp + nonce + encode
  │    └─ HMAC-SHA256(canonical, app.secret).hex() == signature?
  │    └─ 不等 → SIGNATURE_BAD
  │
  ├─ 包装 cb：handler 输出后，若 request_encrypted → 加密响应
  │
  └─ 调 handler
```

Handler 里的 `paramStr(req, "license")` 会**优先从 decoded_kv 取**——如果请求用了加密，解密后的 license/device/appid 直接覆盖明文参数。Handler 完全不知道加密发生了。
