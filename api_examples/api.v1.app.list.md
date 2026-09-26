# /api/v1/app/list

**方法**: GET

**认证**: Owner Bearer

**描述**: 所有者查询自己的应用列表

## cURL

``bash
curl -s http://127.0.0.1:10211/api/v1/app/list -H 'Authorization: Bearer '
``
## 响应

``json
{"code":0,"data":[{"appid":1,"name":"Game","status":"active","created_at":"2026-09-26 15:20:00"}],"message":"OK"}
``
