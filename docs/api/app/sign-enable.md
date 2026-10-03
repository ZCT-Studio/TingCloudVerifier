# POST /api/v1/app/sign-enable

OWNER 开关 APP 的 HMAC-SHA256 签名强制校验。

## 请求

```json
{
  "appid": "TINC123456",
  "sign_enable": 1
}
```

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `appid` | string | ✅ | 目标 APP |
| `sign_enable` | int | ✅ | `0` = 关闭，`1` = 强制 |

## 效果

| sign_enable | 客户端 `signature` 参数 | 服务端校验 |
|---|---|---|
| 0 | 可选 | 不校验 |
| 1 | **必填** | 强制 HMAC-SHA256(canonical, app.secret).hex() |

`sign_enable=1` 时，缺失或错的 `signature` → `code: 2003 SIGNATURE_BAD`。

## 签名算法

```
canonical = METHOD + PATH + appid + timestamp + nonce + encode(optional)
signature = HMAC_SHA256_HEX(canonical, app.secret)
```

详见 [签名校验](../../client/signing.md)。

## 搭配建议

| 加密模式 | sign_enable | 效果 |
|---|---|---|
| NONE | 0 | 最低安全：仅 timestamp + nonce 防重放 |
| NONE | **1** | 中档：防篡改 + 防重放 |
| AES256-GCM | 0 | 中高档：加密 + GCM tag 隐式完整性 |
| **AES256-GCM** | **1** | **推荐最高：加密 + 显式签名 + 完整 canonical 覆盖** |
