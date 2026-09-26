# /api/v1/license/delete

**方法**: POST

**认证**: Owner Bearer

**描述**: 删除卡密

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/license/delete -H 'Authorization: Bearer ' -d 'license_id=1'
``
## 请求体

``json
license_id=1
``
## 响应

``json
{"code":0,"data":null,"message":"OK"}
``
