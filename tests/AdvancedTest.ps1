$ErrorActionPreference = "Stop"
$BASE = "http://127.0.0.1:10212"
$pass = 0; $fail = 0; $expected_fail = 0

# T: test passes (code==0)
# X: test expected to fail with specific code (counts as "expected")
function T($n, $b) {
    try {
        $r = & $b
        if ($r.code -ne 0) {
            Write-Host "  FAIL $n (code:$($r.code):$($r.message))" -ForegroundColor Red
            $script:fail++
            return $r.data
        }
        Write-Host "  OK  $n" -ForegroundColor Green
        $script:pass++
        return $r.data
    } catch {
        Write-Host "  FAIL $n ($_)" -ForegroundColor Red
        $script:fail++
    }
}
function X($n, $expected, $b) {
    try {
        $r = & $b
        if ($r.code -eq $expected) {
            Write-Host "  OK  $n (预期拒绝, code=$($r.code))" -ForegroundColor Green
            $script:pass++
        } else {
            Write-Host "  FAIL $n (期望 code=$expected, 实际 code=$($r.code):$($r.message))" -ForegroundColor Red
            $script:fail++
        }
    } catch {
        Write-Host "  FAIL $n ($_)" -ForegroundColor Red
        $script:fail++
    }
}

Write-Host "=== 1. 准备环境 ===" -ForegroundColor Yellow
T "bootstrap-admin" { Invoke-RestMethod "$BASE/api/v1/auth/bootstrap-admin" -Method POST -Body @{username="root";password="admin123"} }
$a = T "admin login" { Invoke-RestMethod "$BASE/api/v1/auth/admin/login" -Method POST -Body @{username="root";password="admin123"} }
$AT = $a.token
T "owner create" { Invoke-RestMethod "$BASE/api/v1/admin/owner/create" -Method POST -Body @{username="owner1";password="ownerpass"} -Headers @{Authorization="Bearer $AT"} }
$o = T "owner login" { Invoke-RestMethod "$BASE/api/v1/auth/owner/login" -Method POST -Body @{username="owner1";password="ownerpass"} }
$OT = $o.token
$ac = T "app create" { Invoke-RestMethod "$BASE/api/v1/app/create" -Method POST -Body @{name="TestGame"} -Headers @{Authorization="Bearer $OT"} }
$AID = $ac.appid; $SECRET = $ac.secret
Write-Host "  App ID: $AID" -ForegroundColor Gray

$lc1 = T "license create (LIC-1, DEVICE)" {
    Invoke-RestMethod "$BASE/api/v1/license/create" -Method POST `
        -Body @{app_id=$AID;count=1;seconds_per_license=86400;binding_mode="DEVICE";unbind_limit=3} `
        -Headers @{Authorization="Bearer $OT"}
}
$LIC = $lc1.first_plain_license
Write-Host "  License-1: $LIC" -ForegroundColor Gray
$lc2 = T "license create (LIC-2, DEVICE)" {
    Invoke-RestMethod "$BASE/api/v1/license/create" -Method POST `
        -Body @{app_id=$AID;count=1;seconds_per_license=86400;binding_mode="DEVICE";unbind_limit=3} `
        -Headers @{Authorization="Bearer $OT"}
}
$LIC2 = $lc2.first_plain_license
Write-Host "  License-2: $LIC2" -ForegroundColor Gray

$ts=[int](Get-Date -Date (Get-Date).ToUniversalTime() -UFormat %s)

Write-Host "`n=== 2. 明文验证（带 device 参数）===" -ForegroundColor Yellow
$n1=[guid]::NewGuid().ToString('N').Substring(0,16)
T "明文+device" { Invoke-RestMethod "$BASE/api/v1/client/license/verify?appid=$AID&license=$LIC&device=DEVICE_X&timestamp=$ts&nonce=$n1" -Method GET }

Write-Host "`n=== 3. Base64 编码（不同 hash，预期失败）===" -ForegroundColor Yellow
$LIC_B64=[Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes($LIC))
Write-Host "  Base64: $LIC_B64" -ForegroundColor Gray
$n2=[guid]::NewGuid().ToString('N').Substring(0,16)
X "Base64 编码验证" 5001 { Invoke-RestMethod "$BASE/api/v1/client/license/verify?appid=$AID&license=$LIC_B64&device=DEVICE_X&timestamp=$ts&nonce=$n2" -Method GET }

Write-Host "`n=== 4. HMAC 签名+Base64（当前不支持，预期失败）===" -ForegroundColor Yellow
Add-Type -AssemblyName System.Security
$hmac=New-Object System.Security.Cryptography.HMACSHA256
$hmac.Key=[Text.Encoding]::UTF8.GetBytes($SECRET)
$sig=$hmac.ComputeHash([Text.Encoding]::UTF8.GetBytes($LIC))
$sig_hex=-join($sig|%{$_.ToString("x2")})
Write-Host "  HMAC Hex: $sig_hex" -ForegroundColor Gray
$signed_b64=[Convert]::ToBase64String([Text.Encoding]::UTF8.GetBytes("$LIC.$sig_hex"))
$n3=[guid]::NewGuid().ToString('N').Substring(0,16)
X "签名+Base64 验证" 5001 { Invoke-RestMethod "$BASE/api/v1/client/license/verify?appid=$AID&license=$signed_b64&device=DEVICE_X&timestamp=$ts&nonce=$n3" -Method GET }

Write-Host "`n=== 5. 设备绑定测试 ===" -ForegroundColor Yellow
$n4=[guid]::NewGuid().ToString('N').Substring(0,16)
T "绑定 DEVICE_A" { Invoke-RestMethod "$BASE/api/v1/client/license/verify?appid=$AID&license=$LIC2&device=DEVICE_A&timestamp=$ts&nonce=$n4" -Method GET }
$n5=[guid]::NewGuid().ToString('N').Substring(0,16)
X "换 DEVICE_B（应拒绝-设备不匹配）" 5006 { Invoke-RestMethod "$BASE/api/v1/client/license/verify?appid=$AID&license=$LIC2&device=DEVICE_B&timestamp=$ts&nonce=$n5" -Method GET }

Write-Host "`n=== 6. 解绑测试 ===" -ForegroundColor Yellow
T "解绑 license_id=2" { Invoke-RestMethod "$BASE/api/v1/license/unbind" -Method POST -Body @{license_id=2} -Headers @{Authorization="Bearer $OT"} }
$n6=[guid]::NewGuid().ToString('N').Substring(0,16)
T "解绑后用 DEVICE_B 验证" { Invoke-RestMethod "$BASE/api/v1/client/license/verify?appid=$AID&license=$LIC2&device=DEVICE_B&timestamp=$ts&nonce=$n6" -Method GET }

Write-Host "`n=======================================" -ForegroundColor Cyan
$total=$pass+$fail
if($fail-eq0){Write-Host "  ALL $total TESTS PASSED" -ForegroundColor Green}else{Write-Host "  $fail/$total FAILED | $pass PASSED" -ForegroundColor Red}
Write-Host "=======================================" -ForegroundColor Cyan