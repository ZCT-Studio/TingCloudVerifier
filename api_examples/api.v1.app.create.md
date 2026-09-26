# /api/v1/app/create

**方法**: POST

**认证**: Owner Bearer

**描述**: 所有者创建应用

## cURL

``bash
curl -s -X POST http://127.0.0.1:10211/api/v1/app/create -H 'Authorization: Bearer ' -d 'name=Game&remark=mygame'
``
## 请求体

``json
name=Game&remark=mygame
``
## 响应

``json
{"code":0,"data":{"appid":1,"secret":"a1b2c3d4e5f6..."}, "message":"OK"}
``
