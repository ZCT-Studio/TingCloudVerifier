# /api/v1/license/create

**方法**: POST

**认证**: Owner Bearer

**描述**: 批量创建卡密

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/license/create -H 'Authorization: Bearer ' -d 'app_id=1&count=5&seconds_per_license=86400'
``
## 请求体

``json
app_id=1&count=5&seconds_per_license=86400
``
## 响应

``json
{"code":0,"data":{"created_count":5,"first_plain_license":"TCV-XXXX-XXXX-XXXX","plain_licenses":["TCV-...","TCV-..."]},"message":"OK"}
``
