# /api/v1/client/license/remaining

**方法**: GET

**认证**: 公开（需 appid + license）

**描述**: 查询卡密剩余秒数

## cURL

``bash
curl -s 'http://127.0.0.1:10211/api/v1/client/license/remaining?appid=1&license=TCV-XXXX-XXXX-XXXX'
``
## 响应

``json
{"code":0,"data":{"remaining":86400,"permanent":false},"message":"OK"}
``
