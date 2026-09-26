# /api/v1/system/configs

**方法**: GET

**认证**: 公开

**描述**: 获取系统公开配置

## cURL

``bash
curl -s http://127.0.0.1:10211/api/v1/system/configs
``
## 响应

``json
{"code":0,"data":{"rate_limit_enabled":true,"admin_security_level":"LAN"},"message":"OK"}
``
