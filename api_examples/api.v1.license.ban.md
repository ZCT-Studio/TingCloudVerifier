# /api/v1/license/ban

**方法**: POST

**认证**: Owner Bearer

**描述**: 封禁卡密

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/license/ban -H 'Authorization: Bearer ' -d 'license_id=1&reason=fraud'
``
## 请求体

``json
license_id=1&reason=fraud
``
## 响应

``json
{"code":0,"data":null,"message":"OK"}
``
