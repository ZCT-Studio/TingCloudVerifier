# /api/v1/license/list

**方法**: GET

**认证**: Owner Bearer

**描述**: 查询卡密列表

## cURL

``bash
curl -s 'http://127.0.0.1:8080/api/v1/license/list?app_id=1&page=1&page_size=20' -H 'Authorization: Bearer '
``
## 响应

``json
{"code":0,"data":{"total":10,"items":[{"id":1,"license_hash":"...","status":"unused","expires_at":"2026-09-27 15:20:00"}]},"message":"OK"}
``
