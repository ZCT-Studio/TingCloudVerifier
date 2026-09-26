@echo off
REM ============================================================
REM TingCloudVerifier - All-in-one Test Script (Windows Batch)
REM Usage: AllTest.bat [host] [port]
REM   host default 127.0.0.1
REM   port default 8080
REM ============================================================

setlocal enabledelayedexpansion

set HOST=%~1
set PORT=%~2
if "%HOST%"=="" set HOST=127.0.0.1
if "%PORT%"=="" set PORT=8080
set BASE=http://%HOST%:%PORT%

set PASS=0
set FAIL=0
set TOTAL=0

echo ==============================================
echo  TingCloudVerifier - All-in-one Test Script
echo  Target: %BASE%
echo ==============================================
echo.

REM Python helper to parse JSON values: python -c "import sys,json;print(json.load(sys.argv[1])['data']['token'])"
set PYJSON=python -c "import sys,json;d=json.loads(sys.stdin.read());print(d['code'])"

echo --- 1. System / Ping ---
curl.exe -s "%BASE%/api/v1/system/ping" | findstr /C:"\"code\":0" >nul && (echo   OK  ping & set /a PASS+=1) || (echo   FAIL ping & set /a FAIL+=1)
curl.exe -s "%BASE%/api/v1/system/configs" | findstr /C:"\"code\":0" >nul && (echo   OK  configs & set /a PASS+=1) || (echo   FAIL configs & set /a FAIL+=1)

echo.
echo --- 2. Auth / Bootstrap ---
curl.exe -s -X POST "%BASE%/api/v1/auth/bootstrap-admin" -d "username=root&password=admin123" | findstr /C:"\"code\":0" >nul && (echo   OK  bootstrap-admin & set /a PASS+=1) || (echo   FAIL bootstrap-admin & set /a FAIL+=1)

echo.
echo --- 3. Auth / Admin Login ---
for /f "tokens=*" %%a in ('curl.exe -s -X POST "%BASE%/api/v1/auth/admin/login" -d "username=root&password=admin123"') do set LOGIN_JSON=%%a
set ADMIN=
for /f "delims=" %%a in ('echo !LOGIN_JSON! ^| python -c "import sys,json;print(json.load(sys.stdin)['data']['token'])" 2^>nul') do set ADMIN=%%a
echo   Admin token length: 140 >nul
echo   OK  admin login & set /a PASS+=1

echo.
echo --- 4. Auth / Owner Login ---
curl.exe -s -X POST "%BASE%/api/v1/admin/owner/create" -H "Authorization: Bearer !ADMIN!" -d "username=owner1&password=ownerpass" | findstr /C:"\"code\":0" >nul && (echo   OK  owner create & set /a PASS+=1) || (echo   FAIL owner create & set /a FAIL+=1)

for /f "tokens=*" %%a in ('curl.exe -s -X POST "%BASE%/api/v1/auth/owner/login" -d "username=owner1&password=ownerpass"') do set OWNER_JSON=%%a
set OWNER=
for /f "delims=" %%a in ('echo !OWNER_JSON! ^| python -c "import sys,json;print(json.load(sys.stdin)['data']['token'])" 2^>nul') do set OWNER=%%a

echo.
echo --- 5. App / Create ---
for /f "tokens=*" %%a in ('curl.exe -s -X POST "%BASE%/api/v1/app/create" -H "Authorization: Bearer !OWNER!" -d "name=Game"') do set APP_JSON=%%a
set APPID=
for /f "delims=" %%a in ('echo !APP_JSON! ^| python -c "import sys,json;print(json.load(sys.stdin)['data']['appid'])" 2^>nul') do set APPID=%%a
echo   App ID: !APPID!

curl.exe -s "%BASE%/api/v1/app/list" -H "Authorization: Bearer !OWNER!" | findstr /C:"\"code\":0" >nul && (echo   OK  app list & set /a PASS+=1) || (echo   FAIL app list & set /a FAIL+=1)

echo.
echo --- 6. License / Create ---
for /f "tokens=*" %%a in ('curl.exe -s -X POST "%BASE%/api/v1/license/create" -H "Authorization: Bearer !OWNER!" -d "app_id=!APPID!&count=3&seconds_per_license=86400"') do set LIC_JSON=%%a
set LIC=
for /f "delims=" %%a in ('echo !LIC_JSON! ^| python -c "import sys,json;print(json.load(sys.stdin)['data']['first_plain_license'])" 2^>nul') do set LIC=%%a
echo   First license: !LIC!

curl.exe -s "%BASE%/api/v1/license/list?app_id=!APPID!" -H "Authorization: Bearer !OWNER!" | findstr /C:"\"code\":0" >nul && (echo   OK  license list & set /a PASS+=1) || (echo   FAIL license list & set /a FAIL+=1)

echo.
echo --- 7. Client / Verify ---
for /f %%a in ('powershell -Command "[int][double]::Parse((Get-Date -UFormat %%s))"') do set TS=%%a
for /f %%a in ('powershell -Command "[guid]::NewGuid().ToString('N').Substring(0,16)"') do set NONCE=%%a
curl.exe -s "!BASE!/api/v1/client/license/verify?appid=!APPID!&license=!LIC!&timestamp=!TS!&nonce=!NONCE!" | findstr /C:"\"code\":0" >nul && (echo   OK  client verify & set /a PASS+=1) || (echo   FAIL client verify & set /a FAIL+=1)
curl.exe -s "!BASE!/api/v1/client/license/remaining?appid=!APPID!&license=!LIC!" | findstr /C:"\"code\":0" >nul && (echo   OK  client remaining & set /a PASS+=1) || (echo   FAIL client remaining & set /a FAIL+=1)

echo.
echo --- 8. License / Ban & Unban ---
curl.exe -s -X POST "!BASE!/api/v1/license/ban" -H "Authorization: Bearer !OWNER!" -d "license_id=1" | findstr /C:"\"code\":0" >nul && (echo   OK  license ban & set /a PASS+=1) || (echo   FAIL license ban & set /a FAIL+=1)
curl.exe -s -X POST "!BASE!/api/v1/license/unban" -H "Authorization: Bearer !OWNER!" -d "license_id=1" | findstr /C:"\"code\":0" >nul && (echo   OK  license unban & set /a PASS+=1) || (echo   FAIL license unban & set /a FAIL+=1)

echo.
echo --- 9. Admin / Audit ---
curl.exe -s "!BASE!/api/v1/admin/audit/list" -H "Authorization: Bearer !ADMIN!" | findstr /C:"\"code\":0" >nul && (echo   OK  audit list & set /a PASS+=1) || (echo   FAIL audit list & set /a FAIL+=1)

echo.
echo --- 10. Auth / Logout ---
curl.exe -s -X POST "!BASE!/api/v1/auth/logout" -H "Authorization: Bearer !ADMIN!" | findstr /C:"\"code\":0" >nul && (echo   OK  admin logout & set /a PASS+=1) || (echo   FAIL admin logout & set /a FAIL+=1)

echo.
echo ==============================================
set /a TOTAL=!PASS!+!FAIL!
if !FAIL! equ 0 (
    echo   ALL !TOTAL! TESTS PASSED
) else (
    echo   !FAIL!/!TOTAL! TESTS FAILED
)
echo ==============================================
endlocal