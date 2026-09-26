# /api/v1/app/secret/regenerate

**方法**: POST

**认证**: Owner Bearer

**描述**: 重新生成 App Secret

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/app/secret/regenerate -H 'Authorization: Bearer ' -d 'app_id=1'
``
## 请求体

``json
app_id=1
``
## 响应

``json
{"code":0,"data":{"appid":1,"secret":"newSecret..."}, "message":"OK"}
``
