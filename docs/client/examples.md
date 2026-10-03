# 完整示例

## curl（无加密、无签名）

最简单场景，本地测试用。仅 timestamp + nonce 防重放：

```bash
APPID="TINC123456"
TS=$(date +%s)
NONCE=$(uuidgen)

curl -X POST http://localhost:10211/api/v1/client/license/verify \
  -d "appid=$APPID&license=KEY-ABC-123&device=my_laptop&timestamp=$TS&nonce=$NONCE"
```

返回：

```json
{"code":0,"message":"ok","data":{"status":"active","remaining":86400,"expired":false,"expires_at":1767225600,"binding_mode":"DEVICE","remark":"","license_type":"premium"}}
```

## curl（AES256-GCM 加密 + HMAC-SHA256 签名）

生产级配置，四层防护：

```bash
#!/bin/bash
set -euo pipefail

APPID="TINC123456"
DEC_KEY="4b9c0a..."                # 64 hex chars
SECRET="7e2f1a..."                 # 40 hex chars
METHOD="POST"
PATH="/api/v1/client/license/verify"

# 1. 构造 URL k=v 明文
PLAIN="appid=${APPID}&license=KEY-ABC-123&device=my_laptop"
TS=$(date +%s)
NONCE_HEX=$(openssl rand -hex 8)

# 2. AES256-GCM 加密
NONCE_AES=$(openssl rand -hex 12)  # 24 hex = 12 bytes
CT_TAG=$(echo -n "$PLAIN" | openssl enc -aes-256-gcm -K "${DEC_KEY}" -iv "${NONCE_AES}" -nosalt 2>/dev/null | xxd -p | tr -d '\n')
# 手动拼装 blob = nonce + ct + tag（xxd 输出顺序需要处理）
BLOB_HEX="${NONCE_AES}${CT_TAG}"
ENCODE=$(echo -n "$BLOB_HEX" | xxd -r -p | base64 | tr -d '\n')

# 3. HMAC-SHA256 签名
CANONICAL="${METHOD}${PATH}${APPID}${TS}${NONCE_HEX}${ENCODE}"
SIGNATURE=$(echo -n "$CANONICAL" | openssl dgst -sha256 -mac HMAC -macopt hexkey:"${SECRET}" -hex | awk '{print $NF}')

# 4. 发送
curl -s -X POST "http://localhost:10211${PATH}" \
  -d "appid=$APPID&timestamp=$TS&nonce=$NONCE_HEX&signature=$SIGNATURE&encode=$ENCODE"
```

返回（加密响应）：

```json
{"code":0,"message":"ok","encode":"<base64 blob>"}
```

客户端解密：

```bash
RESP_ENCODE="<base64 from response>"
BLOB_HEX=$(echo -n "$RESP_ENCODE" | base64 -d | xxd -p | tr -d '\n')
NONCE_AES="${BLOB_HEX:0:24}"   # 前 24 hex = 12 bytes
CT_AND_TAG="${BLOB_HEX:24}"
# openssl 解密（tag 是最后 32 hex = 16 bytes）
TAG="${CT_AND_TAG: -32}"
CT="${CT_AND_TAG:0:-32}"
echo -n "$CT" | xxd -r -p | openssl enc -d -aes-256-gcm -K "${DEC_KEY}" -iv "${NONCE_AES}" -mac HMAC -macopt hexkey:"${TAG}"
```

## Python SDK 雏形

```python
import hmac, hashlib, base64, os, time, json
from urllib import request, parse
from Crypto.Cipher import AES   # pip install pycryptodome

class TingClient:
    def __init__(self, base_url, appid, dec_mode="NONE", dec_key="", secret="", sign_enable=False):
        self.base_url = base_url.rstrip("/")
        self.appid = appid
        self.dec_mode = dec_mode
        self.dec_key = bytes.fromhex(dec_key) if dec_key else b""
        self.secret = bytes.fromhex(secret) if secret else b""
        self.sign_enable = sign_enable

    def _encrypt(self, plaintext: str) -> str:
        if self.dec_mode == "AES256-GCM":
            nonce = os.urandom(12)
            cipher = AES.new(self.dec_key, AES.MODE_GCM, nonce=nonce)
            ct, tag = cipher.encrypt_and_digest(plaintext.encode())
            return base64.b64encode(nonce + ct + tag).decode()
        elif self.dec_mode == "BASE64":
            return base64.b64encode(plaintext.encode()).decode()
        elif self.dec_mode == "HEX":
            return plaintext.encode().hex()
        return ""

    def _decrypt(self, encoded: str) -> str:
        if self.dec_mode == "AES256-GCM":
            blob = base64.b64decode(encoded)
            nonce, tag = blob[:12], blob[-16:]
            ct = blob[12:-16]
            cipher = AES.new(self.dec_key, AES.MODE_GCM, nonce=nonce)
            return cipher.decrypt_and_verify(ct, tag).decode()
        elif self.dec_mode == "BASE64":
            return base64.b64decode(encoded).decode()
        elif self.dec_mode == "HEX":
            return bytes.fromhex(encoded).decode()
        return encoded

    def _sign(self, method, path, ts, nonce, encode):
        if not self.sign_enable: return ""
        canonical = f"{method}{path}{self.appid}{ts}{nonce}{encode}"
        return hmac.new(self.secret, canonical.encode(), hashlib.sha256).hexdigest()

    def _request(self, path, plain_params):
        ts = str(int(time.time()))
        nonce = os.urandom(8).hex()
        encode = self._encrypt(parse.urlencode(plain_params)) if self.dec_mode != "NONE" else ""
        sig = self._sign("POST", path, ts, nonce, encode)

        params = {
            "appid": self.appid,
            "timestamp": ts,
            "nonce": nonce,
        }
        if encode:   params["encode"]   = encode
        if sig:      params["signature"] = sig
        data = parse.urlencode(params).encode()

        req = request.Request(
            self.base_url + path,
            data=data, method="POST",
            headers={"Content-Type": "application/x-www-form-urlencoded"}
        )
        with request.urlopen(req) as resp:
            body = json.loads(resp.read())

        if "encode" in body:
            body["data"] = json.loads(self._decrypt(body["encode"]))
            body.pop("encode")
        return body

    def verify(self, license, device=""):
        return self._request("/api/v1/client/license/verify", {
            "appid": self.appid,
            "license": license,
            "device": device,
        })

    def fetch_notice(self):
        return self._request("/api/v1/client/app/notice", {"appid": self.appid})

# 使用
client = TingClient(
    base_url="https://api.example.com",
    appid="TINC123456",
    dec_mode="AES256-GCM",
    dec_key="4b9c0a...",
    secret="7e2f1a...",
    sign_enable=True,
)
resp = client.verify("KEY-ABC-123", device_hash)
print(resp["data"])
```

## 错误处理

所有 client API 返回统一 envelope：

```jsonc
{"code": <int>, "message": "<string>", "data": <any>}
```

| code | 含义 | 客户端处理建议 |
|---|---|---|
| 0 | 成功 | 用 `data` |
| 2001 | timestamp 超窗口 | 检查系统时间同步（NTP） |
| 2002 | nonce 重复 | 确认 nonce 是随机新生成的 |
| 2003 | signature 错 | 检查 secret、canonical 串拼接顺序 |
| 1000 | encode 解密失败 | 检查 dec_key 是否与服务端一致 |
| 3001 | 卡密不存在 | 提示用户重新输入 |
| 3003 | 卡密过期 | 引导续费 |
| 3004 | 绑定失败 | 提示用户设备已变、需要解绑 |

生产级客户端建议把错误码集中管理：

```python
ERR_TIMESTAMP = 2001
ERR_NONCE = 2002
ERR_SIGNATURE = 2003
ERR_DECRYPT = 1000
ERR_EXPIRED = 3003

if resp["code"] == ERR_TIMESTAMP:
    print("[!] 系统时间漂移过大")
elif resp["code"] == ERR_SIGNATURE:
    print("[!] secret 或 canonical 串错误")
elif resp["code"] == ERR_EXPIRED:
    print("[!] 卡密已过期")
```
