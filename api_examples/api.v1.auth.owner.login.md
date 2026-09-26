# /api/v1/auth/owner/login

**方法**: POST

**认证**: 公开

**描述**: 所有者登录

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/auth/owner/login -d 'username=owner1&password=ownerpass'
``
## 请求体

``json
username=owner1&password=ownerpass
``
## 响应

``json
{"code":0,"data":{"token":"eyJhbGciOi...","user_id":2,"username":"owner1"},"message":"OK"}
``
