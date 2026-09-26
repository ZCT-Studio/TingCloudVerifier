# /api/v1/auth/subuser/login

**方法**: POST

**认证**: 公开

**描述**: 子用户登录

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/auth/subuser/login -d 'username=sub1&password=subpass'
``
## 请求体

``json
username=sub1&password=subpass
``
## 响应

``json
{"code":0,"data":{"token":"eyJhbGciOi...","user_id":3,"username":"sub1","owner_id":2},"message":"OK"}
``
