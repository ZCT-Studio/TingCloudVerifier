# /api/v1/client/license/verify

**方法**: GET/POST

**认证**: 公开（需 appid + license + timestamp + nonce）

**描述**: 客户端验证卡密（含 App 签名 + 重放保护）

## cURL

``bash
curl -s 'http://127.0.0.1:10211/api/v1/client/license/verify?appid=1&license=TCV-XXXX-XXXX-XXXX&timestamp=1790409133&nonce=a1b2c3d4e5f6'
``
## 响应

``json
{"code":0,"data":{"status":"unused","expired":false,"remaining":86400,"expires_at":1790495533,"binding_mode":"NONE"},"message":"OK"}
``
