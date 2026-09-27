# /api/v1/client/license/status

**方法**: GET

**认证**: 公开（需 appid + license）

**描述**: 查询卡密状态

## cURL

``bash
curl -s 'http://127.0.0.1:10211/api/v1/client/license/status?appid=1&license=TCV-XXXX-XXXX-XXXX'
``
## 响应

``json
{"code":0,"data":{"status":"active","expired":false,"banned":false},"message":"OK"}
``
