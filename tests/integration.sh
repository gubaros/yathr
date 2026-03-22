#!/usr/bin/env bash
#
# Integration tests for YATHR.
#
# Starts ./http_server, fires curl requests, and checks HTTP status codes
# and headers.  Must be run from the project root (where config.txt and
# zlog.conf live), or via `make test` which handles this automatically.
#

PASS=0
FAIL=0
SERVER_PID=""

# ── helpers ──────────────────────────────────────────────────────────

cleanup() {
    if [ -n "$SERVER_PID" ]; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

check() {
    local name="$1"
    local expected="$2"
    local actual="$3"
    if [ "$expected" = "$actual" ]; then
        echo "  PASS  $name"
        PASS=$((PASS + 1))
    else
        echo "  FAIL  $name"
        echo "        expected : $expected"
        echo "        got      : $actual"
        FAIL=$((FAIL + 1))
    fi
}

# ── start server ─────────────────────────────────────────────────────

./http_server >/dev/null 2>&1 &
SERVER_PID=$!

# Wait up to 2 s for the server to accept connections.
ready=0
for i in $(seq 1 20); do
    if curl -s --max-time 0.1 -o /dev/null "http://localhost:8080/" 2>/dev/null; then
        ready=1
        break
    fi
    sleep 0.1
done

if [ "$ready" -eq 0 ]; then
    echo "FATAL: server did not start within 2 s"
    exit 1
fi

echo "--- Integration Tests ---"

# ── status code checks ───────────────────────────────────────────────

for route in google amazon youtube netflix reddit wikipedia; do
    status=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8080/$route")
    check "HTTP 302 for /$route" "302" "$status"
done

# Unknown route → 404
status=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8080/doesnotexist")
check "HTTP 404 for /doesnotexist" "404" "$status"

# Empty path → 404 (no key to look up)
status=$(curl -s -o /dev/null -w "%{http_code}" "http://localhost:8080/")
check "HTTP 404 for /" "404" "$status"

# ── Location header checks ───────────────────────────────────────────

check_location() {
    local route="$1"
    local expected_url="$2"
    local location
    location=$(curl -sI "http://localhost:8080/$route" \
        | grep -i "^location:" \
        | tr -d '\r' \
        | sed 's/^[Ll]ocation: //')
    check "Location for /$route" "$expected_url" "$location"
}

check_location "google"    "https://www.google.com"
check_location "youtube"   "https://www.youtube.com"
check_location "wikipedia" "https://www.wikipedia.org"

# ── summary ──────────────────────────────────────────────────────────

echo ""
echo "Results: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
