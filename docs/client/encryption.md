# 加密传输

APP 级粒度（`app.dec_mode`），所有 `/api/v1/client/*` 的**请求和响应对称加密**：请求用什么算法加密，响应就用什么算法加密。

## 四种模式

| dec_mode | 请求明文 | 请求 `encode` | 响应明文 | 响应 `encode` |
|---|---|---|---|---|
| `NONE` | URL params | 不使用 | 完整 JSON | 不使用（直接返回） |
| `AES256-GCM` | URL k=v（license=xxx&device=yyy） | base64([12B nonce][ct][16B tag]) | 完整 JSON | 同请求格式 |
| `BASE64` | URL k=v | base64(原文) | 完整 JSON | base64(原文) |
| `HEX` | URL k=v | hex(原文) | 完整 JSON | hex(原文) |

## AES256-GCM 细节

**Key**：`dec_key` 字段，64 字符 hex（32 字节）。OWNER 生成方式：

```bash
openssl rand -hex 32
# → 例如: 4b9c0a...共64个字符
```

**Nonce**：每次加密随机 12 字节，附在密文最前面。

**Tag**：GCM 模式的 16 字节认证标签，附在密文最后。

**Blob 结构**：`[12B nonce][ciphertext][16B tag]`

**编码**：整个 blob 做 base64 → `encode` 参数。

```
解密流程：
  base64_decode(encode) → blob
  nonce = blob[0:12]
  ciphertext = blob[12:-16]
  tag = blob[-16:]
  aes256_gcm_decrypt(ciphertext, key_raw, nonce, tag) → URL k=v
  parseKvs → decoded_kv map
```

响应加密是反过来：handler 输出完整 JSON → 序列化成字符串 → AES256-GCM 加密（随机 nonce + tag）→ base64 → `encode` 字段。响应结构：

```jsonc
// 无加密
{"code":0,"message":"ok","data":{"status":"active","remaining":86400}}

// AES256-GCM 加密
{"code":0,"message":"ok","encode":"<base64 of [nonce][ct][tag]>"}
```

## BASE64 / HEX

最简单的两层编码/解码：

```
BASE64 请求:
  URL k=v 字符串 → base64 → encode 参数
  paramStr() 解密时: base64_decode(encode) → parseKvs

HEX 请求:
  URL k=v 字符串 → hex → encode 参数
  paramStr() 解密时: fromHex(encode) → parseKvs
```

响应同理，JSON 字符串直接编码。

## 示例：AES256-GCM 请求/响应

### 1. OWNER 配置 APP

```bash
# 设置 AES256-GCM 模式
curl -X POST http://api.example.com/api/v1/app/set-decode \
  -H "Authorization: Bearer $OWNER_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "appid": "TINC123456",
    "dec_mode": "AES256-GCM",
    "dec_key": "4b9c0a1f..."
  }'
```

### 2. 客户端准备加密请求

```python
import base64, os, time, json
from Crypto.Cipher import AES

APPID = "TINC123456"
DEC_KEY = bytes.fromhex("4b9c0a1f...")

# 构造 URL k=v 明文
plaintext = f"appid={APPID}&license=KEY-ABC-123&device=hash_xyz" + \
            f"&timestamp={int(time.time())}&nonce={os.urandom(8).hex()}"

# AES256-GCM 加密
nonce = os.urandom(12)
cipher = AES.new(DEC_KEY, AES.MODE_GCM, nonce=nonce)
ct, tag = cipher.encrypt_and_digest(plaintext.encode())
blob = nonce + ct + tag
encode = base64.b64encode(blob).decode()

# 发送请求（此时明文参数已塞进 encode）
resp = requests.post(
    "http://api.example.com/api/v1/client/license/verify",
    data={"appid": APPID, "timestamp": ts, "nonce": nonce_str, "encode": encode}
)
```

### 3. 客户端解密响应

```python
resp_json = resp.json()
resp_code = resp_json["code"]

if "encode" in resp_json:
    # 服务端响应也加密了
    blob = base64.b64decode(resp_json["encode"])
    nonce, tag = blob[:12], blob[-16:]
    ct = blob[12:-16]
    cipher = AES.new(DEC_KEY, AES.MODE_GCM, nonce=nonce)
    plain_json = cipher.decrypt_and_verify(ct, tag)
    data = json.loads(plain_json)
else:
    # 服务端明文返回（app dec_mode = NONE）
    data = resp_json
```

## BASE64 / HEX 请求示例

```bash
# BASE64 (无加密 key)
plain="appid=TINC123456&license=KEY-ABC&timestamp=$(date +%s)&nonce=$(uuidgen)"
encoded=$(echo -n "$plain" | base64)
curl -X POST http://api.example.com/api/v1/client/license/verify \
  -d "appid=TINC123456&timestamp=$(date +%s)&nonce=$(uuidgen)&encode=$encoded"

# HEX
encoded=$(echo -n "$plain" | xxd -p | tr -d '\n')
```

## 安全建议

- **AES256-GCM** 是推荐模式——提供加密 + 完整性（tag），服务端可以验证 encode 没被篡改。
- **BASE64** / **HEX** 只做混淆不做认证——配合 `sign_enable=1` 签名校验才能抵御篡改。
- `dec_key` 不要硬编码到客户端二进制，建议加密存储或首次启动从 OWNER 端下载。
- `AES256-GCM` 每次加密随机 nonce——**不要缓存**，每次发请求都要新生成。
