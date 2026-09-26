# /api/v1/update/latest

**方法**: GET

**认证**: 公开

**描述**: 客户端查询最新版本

## cURL

``bash
curl -s 'http://127.0.0.1:10211/api/v1/update/latest?appid=1&channel=stable'
``
## 响应

``json
{"code":0,"data":{"version":"1.2.3","download_url":"https://example.com/game-1.2.3.zip","force_update":false},"message":"OK"}
``
