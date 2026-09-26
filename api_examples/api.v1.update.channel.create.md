# /api/v1/update/channel/create

**方法**: POST

**认证**: Owner Bearer

**描述**: 创建更新通道

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/update/channel/create -H 'Authorization: Bearer ' -d 'app_id=1&name=stable'
``
## 请求体

``json
app_id=1&name=stable
``
## 响应

``json
{"code":0,"data":{"id":1},"message":"OK"}
``
