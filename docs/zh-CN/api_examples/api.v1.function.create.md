# /api/v1/function/create

**方法**: POST

**认证**: Owner Bearer

**描述**: 为应用创建功能/模块

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/function/create -H 'Authorization: Bearer ' -d 'app_id=1&name=VIP&code=vip'
``
## 请求体

``json
app_id=1&name=VIP&code=vip
``
## 响应

``json
{"code":0,"data":{"id":1},"message":"OK"}
``
