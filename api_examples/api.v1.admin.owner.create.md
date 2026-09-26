# /api/v1/admin/owner/create

**方法**: POST

**认证**: Admin Bearer

**描述**: 管理员创建所有者账户

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/admin/owner/create -H 'Authorization: Bearer ' -d 'username=owner1&password=ownerpass'
``
## 请求体

``json
username=owner1&password=ownerpass
``
## 响应

``json
{"code":0,"data":{"user_id":2},"message":"OK"}
``
