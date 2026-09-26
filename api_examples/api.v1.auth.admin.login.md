# /api/v1/auth/admin/login

**方法**: POST

**认证**: 公开

**描述**: 管理员登录

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/auth/admin/login -d 'username=root&password=admin123'
``
## 请求体

``json
username=root&password=admin123
``
## 响应

``json
{"code":0,"data":{"token":"eyJhbGciOi...","user_id":1,"username":"root"},"message":"OK"}
``
