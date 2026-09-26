# /api/v1/system/ping

**方法**: GET

**认证**: 公开

**描述**: 健康检查

## cURL

``bash
curl -s http://127.0.0.1:8080/api/v1/system/ping
``
## 响应

``json
{"code":0,"data":{"pong":true,"server":"TingCloudVerifier","server_time":1790409133,"version":"0.1.0"},"message":"OK"}
``
