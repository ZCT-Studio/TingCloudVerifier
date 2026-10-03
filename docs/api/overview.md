# API 概览

所有端点按**前缀**分四组，每组不同认证策略：

| 前缀 | 角色 | 认证 | 加密/签名 |
|---|---|---|---|
| `/api/v1/auth/*` | bootstrap-admin | 公开 | ❌ |
| `/api/v1/admin/*` | ADMIN | Bearer Token | ❌ |
| `/api/v1/{app,license}/*` | OWNER / SUBUSER | Bearer Token | ❌ |
| `/api/v1/client/*` | 客户端 | 四要素（timestamp + nonce + 可选 signature） | ✅ APP 级可选 |

## 统一响应格式

```json
{
  "code": 0,
  "message": "ok",
  "data": {}
}
```

- `code = 0` 表示成功，非 0 为错误码（见下方）。
- `message` 可读描述。
- `data` 成功时有值，失败时可能不存在。

## 错误码

| code | 名称 | 说明 |
|---|---|---|
| 0 | OK | 成功 |
| 1000 | BAD_REQUEST | 参数缺失/错误 |
| 1001 | APP_NOT_FOUND | appid 不存在 |
| 1002 | APP_DISABLED | APP 已禁用 |
| 1003 | LICENSE_NOT_FOUND | 卡密不存在 |
| 1004 | LICENSE_BANNED | 卡密已封禁 |
| 1005 | LICENSE_EXPIRED | 卡密已过期 |
| 1006 | LICENSE_BINDING_FAIL | 设备/IP 绑定校验失败 |
| 1101 | VALIDATION | 参数校验失败 |
| 2001 | TIMESTAMP_BAD | timestamp 缺失或超窗口 |
| 2002 | NONCE_REPLAY | nonce 重复（重放） |
| 2003 | SIGNATURE_BAD | 签名缺失或校验失败 |
| 2004 | UNAUTHORIZED | Bearer token 无效/过期 |
| 2005 | IP_NOT_ALLOWED | IP 不在白名单 |
| 3001 | NOT_FOUND | 资源不存在 |
| 9999 | INTERNAL | 服务器内部错误 |

## Bearer Token 认证

所有 `/api/v1/admin/*` 和 `/api/v1/{app,license}/*` 请求必须带：

```http
Authorization: Bearer eyJhbGciOiJIUzI1NiJ9...
```

Token 由登录接口返回，存 `sessions` 表。

## Content-Type

- `POST / PUT`：`application/json` 或 `application/x-www-form-urlencoded`
- `GET`：query string
- Drogon `paramStr()` 统一处理，参数名大小写不敏感（会把首字母大写的 header 也纳入查找范围）。
