# /api/v1/app/delete

**方法**: POST

**认证**: Owner Bearer

**描述**: 所有者删除应用

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/app/delete -H 'Authorization: Bearer ' -d 'app_id=1'
``
## 请求体

``json
app_id=1
``
## 响应

``json
{"code":0,"data":null,"message":"OK"}
``
