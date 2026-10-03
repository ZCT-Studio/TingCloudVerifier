# POST /api/v1/client/license/verify

卡密激活/验证。客户端软件启动时调用，返回卡密状态 + 绑定信息 + 剩余时长。

## 请求

### 四要素（必填）

| 参数 | 类型 | 说明 |
|---|---|---|
| `appid` | string | APP 唯一标识 |
| `timestamp` | int64 | Unix 秒级时间戳 |
| `nonce` | string | 随机字符串 |

### 业务参数

| 参数 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `license` | string | ✅ | 卡密明文（KEY-XXX-XXX） |
| `device` | string | ⭕ | 设备标识（MAC 地址 hash / CPU ID hash / 自定义 hash） |

> 当 APP `dec_mode != NONE` 时，`appid/license/device/timestamp/nonce` 全部塞进 `encode` 参数加密传输。见 [加密传输](../client/encryption.md)。

## 成功响应

```json
{
  "code": 0,
  "message": "ok",
  "data": {
    "status": "active",
    "remaining": 86399,
    "expired": false,
    "expires_at": 1767225600,
    "binding_mode": "DEVICE",
    "remark": "VIP 年卡",
    "license_type": "premium"
  }
}
```

| 字段 | 类型 | 说明 |
|---|---|---|
| `status` | string | `active` / `expired` / `banned` |
| `remaining` | int64 | 剩余秒数（未过期时） |
| `expired` | bool | 是否已过期 |
| `expires_at` | int64 | Unix 秒级过期时间 |
| `binding_mode` | string | 当前卡密使用的绑定模式（NONE / IP / DEVICE / IP_AND_DEVICE） |
| `remark` | string | OWNER 创建卡密时的备注 |
| `license_type` | string | 自由字符串，OWNER 自定义分类 |

## 失败响应示例

```json
// APP 不存在
{"code": 1001, "message": "APPID 不存在"}

// 卡密不存在
{"code": 3001, "message": "卡密不存在"}

// 卡密已过期
{"code": 1005, "message": "卡密已过期", "data": {"expires_at": 1735660800}}

// 设备绑定失败
{"code": 1006, "message": "设备绑定失败", "data": {"bound_device_hash": "..."}}
```

## 绑定校验细节

`verify` 内部按 APP 的 binding_mode 决定校验策略：

| APP binding_mode | 校验逻辑 |
|---|---|
| `NONE` | 不校验，仅验证状态和时间 |
| `IP` | `ip` 必须与卡密首次使用时记录的一致 |
| `DEVICE` | `device` 必须与卡密首次使用时记录的一致 |
| `IP_AND_DEVICE` | 两者都必须一致 |

卡密首次被 verify 成功时自动记录绑定信息（`bound_ip_hash` / `bound_device_hash`），之后每次都校验。OWNER 可通过 `POST /api/v1/license/unbind` 手动解除。

## 加密响应

若 APP `dec_mode != NONE`，响应被同算法加密：

```json
{"code":0,"message":"ok","encode":"<base64 blob>"}
```

客户端需要用同一密钥解密 `encode` 拿到原始 JSON（含 `status` / `remaining` / `expires_at` 等字段）。
