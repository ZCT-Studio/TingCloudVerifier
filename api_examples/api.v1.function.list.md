# /api/v1/function/list

**方法**: GET

**认证**: Owner Bearer

**描述**: 查询应用功能列表

## cURL

``bash
curl -s 'http://127.0.0.1:10211/api/v1/function/list?app_id=1' -H 'Authorization: Bearer '
``
## 响应

``json
{"code":0,"data":[{"id":1,"app_id":1,"name":"VIP","code":"vip"}],"message":"OK"}
``
