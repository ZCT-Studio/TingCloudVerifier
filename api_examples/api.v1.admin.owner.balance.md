# /api/v1/admin/owner/balance

**方法**: POST

**认证**: Admin Bearer

**描述**: 管理员查看/调整所有者余额

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/admin/owner/balance -H 'Authorization: Bearer ' -d 'owner_id=2'
``
## 请求体

``json
owner_id=2
``
## 响应

``json
{"code":0,"data":{"owner_id":2,"balance":10000},"message":"OK"}
``
