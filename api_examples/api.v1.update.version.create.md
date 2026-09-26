# /api/v1/update/version/create

**方法**: POST

**认证**: Owner Bearer

**描述**: 上传新版本

## cURL

``bash
curl -s -X POST http://127.0.0.1:8080/api/v1/update/version/create -H 'Authorization: Bearer ' -d 'channel_id=1&version=1.2.3&download_url=https://...'
``
## 请求体

``json
channel_id=1&version=1.2.3&download_url=https://example.com/game-1.2.3.zip
``
## 响应

``json
{"code":0,"data":{"id":1},"message":"OK"}
``
