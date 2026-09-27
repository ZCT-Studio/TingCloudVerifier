# /api/v1/license/set-time

**方法**: POST

**认证**: Owner Bearer

**描述**: 设置卡密到期时间（或永久）

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/license/set-time -H 'Authorization: Bearer ' -d 'license_id=1&permanent=true'
``
## 请求体

``json
license_id=1&permanent=true
``
## 响应

``json
{"code":0,"data":{"permanent":true},"message":"OK"}
``
