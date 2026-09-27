# /api/v1/auth/bootstrap-admin

**方法**: POST

**认证**: 公开（仅首次）

**描述**: 初始化超级管理员

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/auth/bootstrap-admin -d 'username=root&password=admin123'
``
## 请求体

``json
username=root&password=admin123
``
## 响应

``json
{"code":0,"data":null,"message":"OK"}
``
