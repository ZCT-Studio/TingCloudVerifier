# /api/v1/function/delete

**方法**: POST

**认证**: Owner Bearer

**描述**: 删除功能/模块

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/function/delete -H 'Authorization: Bearer ' -d 'function_id=1'
``
## 请求体

``json
function_id=1
``
## 响应

``json
{"code":0,"data":null,"message":"OK"}
``
