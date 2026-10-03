# 签名校验

当 APP 开启 `sign_enable=1` 时，所有 `/api/v1/client/*` 请求必须携带 `signature` 参数，服务器用 HMAC-SHA256 校验请求完整性。

## canonical 串

```
canonical = METHOD + PATH + appid + timestamp + nonce + encode
```

| 字段 | 来源 | 说明 |
|---|---|---|
| METHOD | `req->methodString()` | POST / GET |
| PATH | `req->path()` | 例如 `/api/v1/client/license/verify` |
| appid | 请求参数 | - |
| timestamp | 请求参数 | - |
| nonce | 请求参数 | - |
| encode | 请求参数（可选） | 无加密时为空字符串 |

**注意**：
- 字段之间**无分隔符**直接拼接。
- `encode` 在 **AES256-GCM** / **BASE64** / **HEX** 模式下存在，`NONE` 模式下为空字符串。
- 任何一个字段顺序不对、多加少少、空格都会导致校验失败。

## HMAC-SHA256

```
signature = HMAC_SHA256_HEX(canonical, app.secret)
```

- `app.secret` 是 40 字符 hex 的随机串，由系统在创建 APP 时自动生成。OWNER 可通过 `POST /api/v1/app/secret/regenerate` 重新生成。
- `hmacSha256Hex` 是 crypto.hpp 里的工具函数。

## 代码示例

### Python

```python
import hmac, hashlib

def sign(method, path, appid, timestamp, nonce, encode, secret):
    canonical = f"{method}{path}{appid}{timestamp}{nonce}{encode or ''}"
    return hmac.new(
        bytes.fromhex(secret),
        canonical.encode(),
        hashlib.sha256
    ).hexdigest()

signature = sign("POST", "/api/v1/client/license/verify",
                 "TINC123456", str(int(time.time())), nonce, encode, SECRET)
```

### Go

```go
import (
    "crypto/hmac"
    "crypto/sha256"
    "encoding/hex"
)

func sign(method, path, appid, timestamp, nonce, encode, secretHex string) string {
    canonical := method + path + appid + timestamp + nonce + encode
    key, _ := hex.DecodeString(secretHex)
    h := hmac.New(sha256.New, key)
    h.Write([]byte(canonical))
    return hex.EncodeToString(h.Sum(nil))
}
```

### curl

```bash
METHOD="POST"
PATH="/api/v1/client/license/verify"
APPID="TINC123456"
TS=$(date +%s)
NONCE=$(uuidgen)
ENCODE="$encoded_or_empty"
SECRET="$(cat secret.hex)"

CANONICAL="${METHOD}${PATH}${APPID}${TS}${NONCE}${ENCODE}"
SIGNATURE=$(echo -n "$CANONICAL" | openssl dgst -sha256 -mac HMAC -macopt hexkey:"$SECRET" -hex | awk '{print $NF}')

curl -X POST "http://api.example.com${PATH}" \
  -d "appid=$APPID&timestamp=$TS&nonce=$NONCE&signature=$SIGNATURE&encode=$ENCODE"
```

## 服务端校验（伪代码）

```cpp
// common.hpp wrap() client 分支
if (app.sign_enable == 1) {
    auto sig_it = params.find("signature");
    if (sig_it == params.end()) return SIGNATURE_BAD;

    const auto canonical = SecurityService::canonicalRequest(
        method, req_path, appid, timestamp, nonce, encode
    );
    auto expected = SecurityService::computeSignature(canonical, app.secret);
    if (expected != sig_it->second) return SIGNATURE_BAD;
}
```

`canonicalRequest` 和 `computeSignature` 在 `security_service.hpp` 里实现。

## 签名与加密的关系

签名和加密是**两层独立防护**：

| 组合 | 加密 | 签名 | 能抵御 |
|---|---|---|---|
| NONE / sign=0 | ❌ | ❌ | 仅靠 nonce 防重放 |
| NONE / sign=1 | ❌ | ✅ | 篡改 / 重放 |
| AES256-GCM / sign=0 | ✅ (隐式认证 tag) | ❌ | 篡改 / 重放（GCM tag 覆盖 encode 内容，但 **canonical 串里只放 encode，明文参数缺失完整校验**） |
| AES256-GCM / sign=1 | ✅ | ✅ | 完整——签名覆盖所有四要素包括 encode，加密隐藏内容 |

**推荐**：`AES256-GCM + sign_enable=1`。签名覆盖了 canonical 串里的所有明文要素（appid / timestamp / nonce）和 encode 密文，加密层再进一步隐藏明文。两层叠加达到 **认证 + 保密 + 完整性**。
