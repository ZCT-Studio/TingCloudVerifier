# TingCloudVerifier - All-in-one Test Script (PowerShell)
# Usage: .\AllTest.ps1 [[-Host] <string>] [[-Port] <int>]

param(
    [string]$Host = "127.0.0.1",
    [int]$Port = 10212
)

$BASE = "http://$($Host):$($Port)"
$pass = 0
$fail = 0

function Test-Api {
    param($Name, $ScriptBlock)
    try {
        $result = & $ScriptBlock
        if ($result.code -eq 0) {
            Write-Host "  OK  $Name" -ForegroundColor Green
            $script:pass++
            return $result.data
        } else {
            Write-Host "  FAIL $Name (code: $($result.code), msg: $($result.message))" -ForegroundColor Red
            $script:fail++
            return $null
        }
    } catch {
        Write-Host "  FAIL $Name (exception: $_ )" -ForegroundColor Red
        $script:fail++
        return $null
    }
}

Write-Host "==============================================" -ForegroundColor Cyan
Write-Host " TingCloudVerifier - All-in-one Test Script" -ForegroundColor Cyan
Write-Host " Target: $BASE" -ForegroundColor Cyan
Write-Host "==============================================" -ForegroundColor Cyan
Write-Host ""

# --- 1. System / Ping ---
Write-Host "--- 1. System / Ping ---" -ForegroundColor Yellow
$ping = Test-Api "ping" { Invoke-RestMethod "$BASE/api/v1/system/ping" -Method GET }
$cfg  = Test-Api "configs" { Invoke-RestMethod "$BASE/api/v1/system/configs" -Method GET }

# --- 2. Auth / Bootstrap ---
Write-Host "`n--- 2. Auth / Bootstrap ---" -ForegroundColor Yellow
$boot = Test-Api "bootstrap-admin" {
    Invoke-RestMethod "$BASE/api/v1/auth/bootstrap-admin" -Method POST -Body @{username="root";password="admin123"}
}

# --- 3. Auth / Admin Login ---
Write-Host "`n--- 3. Auth / Admin Login ---" -ForegroundColor Yellow
$adminLogin = Test-Api "admin login" {
    Invoke-RestMethod "$BASE/api/v1/auth/admin/login" -Method POST -Body @{username="root";password="admin123"}
}
if ($adminLogin -and $adminLogin.token) {
    $ADMIN_TOKEN = $adminLogin.token
    Write-Host "  Admin token obtained" -ForegroundColor Gray
} else {
    $ADMIN_TOKEN = $null
    Write-Host "  FAIL could not get admin token" -ForegroundColor Red
    $fail++
}

# --- 4. Owner ---
Write-Host "`n--- 4. Owner ---" -ForegroundColor Yellow
$ownerCreate = Test-Api "owner create" {
    Invoke-RestMethod "$BASE/api/v1/admin/owner/create" -Method POST -Body @{username="owner1";password="ownerpass"} -Headers @{Authorization = "Bearer $ADMIN_TOKEN"}
}

$ownerLogin = Test-Api "owner login" {
    Invoke-RestMethod "$BASE/api/v1/auth/owner/login" -Method POST -Body @{username="owner1";password="ownerpass"}
}
if ($ownerLogin -and $ownerLogin.token) {
    $OWNER_TOKEN = $ownerLogin.token
    Write-Host "  Owner token obtained" -ForegroundColor Gray
} else {
    $OWNER_TOKEN = $null
    Write-Host "  FAIL could not get owner token" -ForegroundColor Red
    $fail++
}

# --- 5. App ---
Write-Host "`n--- 5. App ---" -ForegroundColor Yellow
$appCreate = Test-Api "app create" {
    Invoke-RestMethod "$BASE/api/v1/app/create" -Method POST -Body @{name="Game"} -Headers @{Authorization = "Bearer $OWNER_TOKEN"}
}
if ($appCreate -and $appCreate.appid) {
    $APPID = $appCreate.appid
    Write-Host "  App ID: $APPID" -ForegroundColor Gray
} else {
    $APPID = $null
    Write-Host "  FAIL could not get app id" -ForegroundColor Red
    $fail++
}

$appList = Test-Api "app list" {
    Invoke-RestMethod "$BASE/api/v1/app/list" -Method GET -Headers @{Authorization = "Bearer $OWNER_TOKEN"}
}

# --- 6. License / Create ---
Write-Host "`n--- 6. License / Create ---" -ForegroundColor Yellow
$licCreate = Test-Api "license create" {
    Invoke-RestMethod "$BASE/api/v1/license/create" -Method POST -Body @{app_id=$APPID;count=3;seconds_per_license=86400} -Headers @{Authorization = "Bearer $OWNER_TOKEN"}
}
if ($licCreate -and $licCreate.first_plain_license) {
    $LICENSE = $licCreate.first_plain_license
    Write-Host "  First license: $LICENSE" -ForegroundColor Gray
} else {
    $LICENSE = $null
    Write-Host "  First license: (none)" -ForegroundColor DarkYellow
}

$licList = Test-Api "license list" {
    Invoke-RestMethod "$BASE/api/v1/license/list?app_id=$APPID" -Method GET -Headers @{Authorization = "Bearer $OWNER_TOKEN"}
}

# --- 7. Client / Verify ---
Write-Host "`n--- 7. Client / Verify ---" -ForegroundColor Yellow
$timestamp = [int][double]::Parse((Get-Date -UFormat %s))
$nonce = [guid]::NewGuid().ToString('N').Substring(0,16)
$verifyUrl = "$BASE/api/v1/client/license/verify?appid=$APPID&license=$LICENSE&timestamp=$timestamp&nonce=$nonce"
$clientVerify = Test-Api "client verify" {
    Invoke-RestMethod $verifyUrl -Method GET
}
$remainingUrl = "$BASE/api/v1/client/license/remaining?appid=$APPID&license=$LICENSE"
$clientRemain = Test-Api "client remaining" {
    Invoke-RestMethod $remainingUrl -Method GET
}

# --- 8. License / Ban & Unban ---
Write-Host "`n--- 8. License / Ban & Unban ---" -ForegroundColor Yellow
$ban = Test-Api "license ban" {
    Invoke-RestMethod "$BASE/api/v1/license/ban" -Method POST -Body @{license_id=1} -Headers @{Authorization = "Bearer $OWNER_TOKEN"}
}
$unban = Test-Api "license unban" {
    Invoke-RestMethod "$BASE/api/v1/license/unban" -Method POST -Body @{license_id=1} -Headers @{Authorization = "Bearer $OWNER_TOKEN"}
}

# --- 9. Admin / Audit ---
Write-Host "`n--- 9. Admin / Audit ---" -ForegroundColor Yellow
$audit = Test-Api "audit list" {
    Invoke-RestMethod "$BASE/api/v1/admin/audit/list" -Method GET -Headers @{Authorization = "Bearer $ADMIN_TOKEN"}
}

# --- 10. Auth / Logout ---
Write-Host "`n--- 10. Auth / Logout ---" -ForegroundColor Yellow
$logout = Test-Api "admin logout" {
    Invoke-RestMethod "$BASE/api/v1/auth/logout" -Method POST -Headers @{Authorization = "Bearer $ADMIN_TOKEN"}
}

# --- Summary ---
Write-Host ""
Write-Host "==============================================" -ForegroundColor Cyan
$total = $pass + $fail
if ($fail -eq 0) {
    Write-Host "  ALL $total TESTS PASSED" -ForegroundColor Green
} else {
    Write-Host "  $fail/$total TESTS FAILED" -ForegroundColor Red
}
Write-Host "==============================================" -ForegroundColor Cyan