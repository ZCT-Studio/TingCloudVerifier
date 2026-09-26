# /api/v1/license/unbind

**方法**: POST

**认证**: Owner Bearer

**描述**: 解除卡密绑定

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/license/unbind -H 'Authorization: Bearer ' -d 'license_id=1'
``
## 请求体

``json
license_id=1
``
## 响应

``json
{"code":0,"data":null,"message":"OK"}
``
