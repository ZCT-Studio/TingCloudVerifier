# /api/v1/auth/logout

**方法**: POST

**认证**: 任意 Bearer Token

**描述**: 登出（使当前 session 失效）

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/auth/logout -H 'Authorization: Bearer '
``
## 响应

``json
{"code":0,"data":null,"message":"OK"}
``
