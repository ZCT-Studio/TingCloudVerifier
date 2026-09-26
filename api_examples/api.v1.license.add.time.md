# /api/v1/license/add-time

**方法**: POST

**认证**: Owner Bearer

**描述**: 给卡密延长时长（秒）

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/license/add-time -H 'Authorization: Bearer ' -d 'license_id=1&seconds=86400'
``
## 请求体

``json
license_id=1&seconds=86400
``
## 响应

``json
{"code":0,"data":{"expires_at":"2026-09-28 15:20:00"},"message":"OK"}
``
