# /api/v1/admin/license-timer/pause

**方法**: POST

**认证**: Admin Bearer

**描述**: 管理员全局暂停卡密计时

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/admin/license-timer/pause -H 'Authorization: Bearer '
``
## 响应

``json
{"code":0,"data":{"timer_paused":true},"message":"OK"}
``
