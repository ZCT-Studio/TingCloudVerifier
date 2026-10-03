# POST /api/v1/app/set-decode

OWNER 配置 APP 的请求/响应加密模式。

## 请求

```json
{
  "appid": "TINC123456",
  "dec_mode": "AES256-GCM",
  "dec_key": "4b9c0a1f..."
}
```

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `appid` | string | ✅ | 目标 APP |
| `dec_mode` | string | ✅ | `NONE` / `AES256-GCM` / `BASE64` / `HEX` |
| `dec_key` | string | 条件 | `AES256-GCM` 时必填，64 字符 hex（32 字节） |

## 注意事项

- `dec_mode = NONE` 时清空加密：`dec_key` 忽略。
- `BASE64` / `HEX` 是纯混淆，不提供认证——建议配合 `sign_enable=1` 使用。
- `AES256-GCM` **同时提供加密 + 完整性**（GCM tag），推荐生产级场景。
- 修改立即生效，无需重启。
- 修改前后客户端需要用新密钥重新构建请求/解析响应。

## 响应

```json
{"code":0,"message":"ok"}
```

## 生成 dec_key

```bash
openssl rand -hex 32
# 4b9c0a1f2e3d4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4e5f6a7b8c9d
```
