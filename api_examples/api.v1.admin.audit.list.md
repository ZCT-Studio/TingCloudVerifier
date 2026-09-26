# /api/v1/admin/audit/list

**方法**: GET

**认证**: Admin Bearer

**描述**: 管理员查询审计日志

## cURL

``bash
curl -s 'http://127.0.0.1:10211/api/v1/admin/audit/list?page=1&page_size=20' -H 'Authorization: Bearer '
``
## 响应

``json
{"code":0,"data":{"total":12,"items":[{"id":1,"action":"LOGIN","actor":"root","target":"","created_at":"2026-09-26 15:20:00"}]},"message":"OK"}
``
