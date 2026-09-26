#!/usr/bin/env bash
# TingCloudVerifier — 一键全链路测试脚本 (Bash)
# 用法: bash AllTest.sh [host] [port]
#   host 默认 127.0.0.1
#   port 默认 8080

set -euo pipefail

HOST="${1:-127.0.0.1}"
PORT="${2:-8080}"
BASE="http://${HOST}:${PORT}"
PASS=0; FAIL=0
TOTAL=0

CURL="curl -s -w\n%{http_code}"

green()  { printf "\033[32m%s\033[0m\n" "$*"; }
red()    { printf "\033[31m%s\033[0m\n" "$*"; }
yellow() { printf "\033[33m%s\033[0m\n" "$*"; }

pass() { green "  ✅ $*"; PASS=$((PASS+1)); }
fail() { red   "  ❌ $*"; FAIL=$((FAIL+1)); }

api() {
    local method="$1" path="$2" data="${3:-}" header="${4:-}"
    local code body
    local args=(-X "$method" -s)
    [[ -n "$data"   ]] && args+=( -d "$data" )
    [[ -n "$header" ]] && args+=( -H "$header" )
    body=$(curl "${args[@]}" "${BASE}${path}")
    code=$(echo "$body" | python3 -c "import sys,sys;print(sys.stdin.read().rsplit('\n',1)[-1])" 2>/dev/null || echo "000")
    echo "$body" | head -n -1
    return 0
}

assert_ok() {
    TOTAL=$((TOTAL+1))
    if echo "$1" | grep -q '"code":0'; then
        pass "$2"
    else
        fail "$2 (response: $(echo "$1" | tr -d '\n'))"
    fi
}

assert_contains() {
    TOTAL=$((TOTAL+1))
    if echo "$1" | grep -q "$3"; then
        pass "$2"
    else
        fail "$2 (missing: $3)"
    fi
}

echo "=============================================="
echo " TingCloudVerifier — All-in-one Test Script"
echo " Target: ${BASE}"
echo "=============================================="
echo

# ---- 1. PING ----
yellow "── 1. System / Ping ──"
R=$(curl -s "${BASE}/api/v1/system/ping"); assert_ok "$R" "ping"
R=$(curl -s "${BASE}/api/v1/system/configs"); assert_ok "$R" "configs"

# ---- 2. BOOTSTRAP ----
yellow "── 2. Auth / Bootstrap ──"
R=$(curl -s -X POST "${BASE}/api/v1/auth/bootstrap-admin" \
     -d "username=root&password=admin123"); assert_ok "$R" "bootstrap-admin"

# ---- 3. ADMIN LOGIN ----
yellow "── 3. Auth / Admin Login ──"
R=$(curl -s -X POST "${BASE}/api/v1/auth/admin/login" \
     -d "username=root&password=admin123")
ADMIN=$(echo "$R" | python3 -c "import sys,json;print(json.load(sys.stdin)['data']['token'])" 2>/dev/null || echo "")
assert_ok "$R" "admin login (got token len=${#ADMIN})"

# ---- 4. OWNER CRUD ----
yellow "── 4. Admin / Owner CRUD ──"
R=$(curl -s -X POST "${BASE}/api/v1/admin/owner/create" \
     -H "Authorization: Bearer ${ADMIN}" -d "username=owner1&password=ownerpass"); assert_ok "$R" "owner create"

R=$(curl -s -X POST "${BASE}/api/v1/auth/owner/login" \
     -d "username=owner1&password=ownerpass")
OWNER=$(echo "$R" | python3 -c "import sys,json;print(json.load(sys.stdin)['data']['token'])" 2>/dev/null || echo "")
assert_ok "$R" "owner login"

# ---- 5. APP CRUD ----
yellow "── 5. App / CRUD ──"
R=$(curl -s -X POST "${BASE}/api/v1/app/create" \
     -H "Authorization: Bearer ${OWNER}" -d "name=Game"); assert_ok "$R" "app create"
APPID=$(echo "$R" | python3 -c "import sys,json;print(json.load(sys.stdin)['data']['appid'])" 2>/dev/null || echo "1")

R=$(curl -s "${BASE}/api/v1/app/list" -H "Authorization: Bearer ${OWNER}"); assert_ok "$R" "app list"

# ---- 6. LICENSE CREATE ----
yellow "── 6. License / Create & List ──"
R=$(curl -s -X POST "${BASE}/api/v1/license/create" \
     -H "Authorization: Bearer ${OWNER}" \
     -d "app_id=${APPID}&count=3&seconds_per_license=86400"); assert_ok "$R" "license create"
LIC=$(echo "$R" | python3 -c "import sys,json;print(json.load(sys.stdin)['data']['first_plain_license'])" 2>/dev/null || echo "")
assert_contains "$LIC" "TCV-" "first plain license present"

R=$(curl -s "${BASE}/api/v1/license/list?app_id=${APPID}" \
     -H "Authorization: Bearer ${OWNER}"); assert_ok "$R" "license list"

# ---- 7. CLIENT VERIFY ----
yellow "── 7. Client / Verify ──"
TS=$(date +%s)
NONCE=$(head -c 8 /dev/urandom | xxd -p)
R=$(curl -s "${BASE}/api/v1/client/license/verify?appid=${APPID}&license=${LIC}&timestamp=${TS}&nonce=${NONCE}")
assert_ok "$R" "client verify"
assert_contains "$R" '"remaining":86400' "remaining == 86400"

R=$(curl -s "${BASE}/api/v1/client/license/remaining?appid=${APPID}&license=${LIC}")
assert_ok "$R" "client remaining"

# ---- 8. BAN / UNBAN ----
yellow "── 8. License / Ban & Unban ──"
LICENSE_ID=1
R=$(curl -s -X POST "${BASE}/api/v1/license/ban" \
     -H "Authorization: Bearer ${OWNER}" -d "license_id=${LICENSE_ID}"); assert_ok "$R" "license ban"
R=$(curl -s -X POST "${BASE}/api/v1/license/unban" \
     -H "Authorization: Bearer ${OWNER}" -d "license_id=${LICENSE_ID}"); assert_ok "$R" "license unban"

# ---- 9. AUDIT ----
yellow "── 9. Admin / Audit ──"
R=$(curl -s "${BASE}/api/v1/admin/audit/list" -H "Authorization: Bearer ${ADMIN}"); assert_ok "$R" "audit list"

# ---- 10. LOGOUT ----
yellow "── 10. Auth / Logout ──"
R=$(curl -s -X POST "${BASE}/api/v1/auth/logout" -H "Authorization: Bearer ${ADMIN}"); assert_ok "$R" "admin logout"

echo
echo "=============================================="
if [[ $FAIL -eq 0 ]]; then
    green " 🎉 ALL ${TOTAL} TESTS PASSED"
else
    red " ❌ ${FAIL}/${TOTAL} TESTS FAILED"
fi
echo "=============================================="