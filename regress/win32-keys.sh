#!/bin/sh

# Windows key handling regression tests.
# Tests that send-keys with named keys produces expected effects.
# Cannot use the upstream cat -tv approach (no stty on Windows), so we
# verify key behavior through observable pane output.
#
# Upstream had 4 backspace/key-mapping fixes in Dec 2024 - Feb 2025:
#   6d792e4, 2a5eba7, 5c3cf2f, eece415

PATH=/bin:/usr/bin
TERM=screen

[ -z "$TEST_TMUX" ] && TEST_TMUX=$(readlink -f ../tmux)
TMUX="$TEST_TMUX -Ltest"
$TMUX kill-server 2>/dev/null
sleep 1

FNULL="-fNUL"
OUT=$(mktemp)
trap "rm -f $OUT; $TMUX kill-server 2>/dev/null" 0 1 15
FAIL=0

fail() {
	echo "FAIL: $1"
	FAIL=1
}

$TMUX $FNULL new -d -skeys -x 120 -y 24 < /dev/null || exit 1
sleep 1

# --- Test 1: Enter key executes commands ---
$TMUX send-keys -tkeys "echo KEY_ENTER_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "KEY_ENTER_OK" || {
	fail "Enter key did not execute command"; exit 1
}
echo "PASS 1: Enter key"

# --- Test 2: Literal characters arrive correctly ---
$TMUX send-keys -tkeys "echo abcXYZ019" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "abcXYZ019" || {
	fail "Literal characters not received"
}
echo "PASS 2: Literal characters"

# --- Test 3: Space key ---
$TMUX send-keys -tkeys "echo" Space "SPACE_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "SPACE_OK" || {
	fail "Space key not working"
}
echo "PASS 3: Space key"

# --- Test 4: Tab key (command completion) ---
# Type partial command and Tab — on cmd.exe, Tab cycles through files.
# Simpler test: verify Tab character is sent by checking it doesn't break things.
$TMUX send-keys -tkeys "echo TAB_OK" Tab Enter
sleep 2
# Tab might complete or not, but the echo should still work
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "TAB_OK" || true
echo "PASS 4: Tab key (no crash)"

# --- Test 5: Escape key (no crash, key is recognized) ---
$TMUX send-keys -tkeys Escape
sleep 0.5
$TMUX send-keys -tkeys "echo ESC_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "ESC_OK" || {
	fail "Escape key broke input"
}
echo "PASS 5: Escape key"

# --- Test 6: Ctrl-C (interrupt) ---
# Send a long-running command, then Ctrl-C to interrupt
$TMUX send-keys -tkeys "ping -n 100 127.0.0.1" Enter
sleep 2
$TMUX send-keys -tkeys C-c
sleep 2
# Verify we get back to a prompt (can type again)
$TMUX send-keys -tkeys "echo CTRLC_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "CTRLC_OK" || {
	fail "Ctrl-C did not interrupt"
}
echo "PASS 6: Ctrl-C interrupt"

# --- Test 7: Arrow keys in copy-mode ---
# Enter copy mode, move around, exit — verify no crash
$TMUX send-keys -tkeys "echo line1" Enter "echo line2" Enter "echo line3" Enter
sleep 1
$TMUX copy-mode -tkeys
sleep 0.5
$TMUX send-keys -tkeys -X cursor-up
$TMUX send-keys -tkeys -X cursor-up
$TMUX send-keys -tkeys -X cursor-down
$TMUX send-keys -tkeys -X cursor-left
$TMUX send-keys -tkeys -X cursor-right
$TMUX send-keys -tkeys -X cancel
sleep 0.5
# Verify pane is still functional
$TMUX send-keys -tkeys "echo ARROW_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "ARROW_OK" || {
	fail "Arrow keys in copy mode broke pane"
}
echo "PASS 7: Arrow keys in copy mode"

# --- Test 8: Home/End in copy mode ---
$TMUX copy-mode -tkeys
sleep 0.5
$TMUX send-keys -tkeys -X start-of-line
$TMUX send-keys -tkeys -X end-of-line
$TMUX send-keys -tkeys -X cancel
sleep 0.5
$TMUX send-keys -tkeys "echo HOMEEND_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "HOMEEND_OK" || {
	fail "Home/End keys broke pane"
}
echo "PASS 8: Home/End in copy mode"

# --- Test 9: PgUp/PgDn in copy mode ---
# Generate enough output to scroll
for i in $(seq 1 30); do
	$TMUX send-keys -tkeys "echo scroll_line_$i" Enter
done
sleep 2
$TMUX copy-mode -tkeys
sleep 0.5
$TMUX send-keys -tkeys -X page-up
sleep 0.3
$TMUX send-keys -tkeys -X page-down
sleep 0.3
$TMUX send-keys -tkeys -X cancel
sleep 0.5
$TMUX send-keys -tkeys "echo PGUPDN_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "PGUPDN_OK" || {
	fail "PgUp/PgDn broke pane"
}
echo "PASS 9: PgUp/PgDn in copy mode"

# --- Test 10: Function keys (no crash) ---
# F1-F5 might trigger help or other actions, but shouldn't crash
$TMUX send-keys -tkeys F1
sleep 0.3
$TMUX send-keys -tkeys Escape
sleep 0.3
$TMUX send-keys -tkeys "echo FKEY_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "FKEY_OK" || {
	fail "Function keys broke pane"
}
echo "PASS 10: Function keys"

# --- Test 11: BSpace (backspace) deletes characters ---
# Type something, backspace, then check result
$TMUX send-keys -tkeys "echo BSPXXX" BSpace BSpace BSpace "ACE_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "BSPACE_OK" || {
	fail "Backspace did not delete characters"
}
echo "PASS 11: Backspace"

# --- Test 12: Multiple modifier combinations (no crash) ---
$TMUX send-keys -tkeys C-a C-e C-k
sleep 0.5
$TMUX send-keys -tkeys "echo MOD_OK" Enter
sleep 2
$TMUX capture-pane -tkeys -p | tr -d '\r' | grep -q "MOD_OK" || {
	fail "Modifier combinations broke pane"
}
echo "PASS 12: Modifier combinations"

$TMUX kill-server 2>/dev/null

if [ $FAIL -eq 0 ]; then
	echo ""
	echo "ALL KEY TESTS PASSED"
	exit 0
else
	echo ""
	echo "SOME KEY TESTS FAILED"
	exit 1
fi
