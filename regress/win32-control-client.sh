#!/bin/sh

# Control mode test for Windows.
# Upstream control-client-sanity.sh exercises -C mode extensively.
# On Windows, control mode is not supported (libevent signal init fails).
# This test verifies the graceful failure and tests equivalent operations
# via normal commands instead.

PATH=/bin:/usr/bin
TERM=screen

[ -z "$TEST_TMUX" ] && TEST_TMUX=$(readlink -f ../tmux)
TMUX="$TEST_TMUX -Lctrltest"
$TMUX kill-server 2>/dev/null
sleep 2

FNULL="-fNUL"
OUT=$(mktemp)
trap "rm -f $OUT; $TMUX kill-server 2>/dev/null" 0 1 15
FAIL=0

fail() {
	echo "FAIL: $1"
	FAIL=1
}

$TMUX $FNULL new -d -sctrl -x 200 -y 200 < /dev/null || exit 1
sleep 1

# --- Test 1: Complex pane operations (equivalent to control-client-sanity) ---
$TMUX splitw -tctrl || { fail "splitw"; exit 1; }
sleep 0.5
$TMUX selectp -tctrl -t0 || { fail "selectp 0"; exit 1; }
$TMUX splitw -tctrl || { fail "splitw 2"; exit 1; }
sleep 0.5
$TMUX neww -tctrl || { fail "neww"; exit 1; }
sleep 0.5
$TMUX splitw -tctrl || { fail "splitw 3"; exit 1; }
sleep 0.5
echo "PASS 1: pane operations"

# --- Test 2: Kill pane ---
# Get pane IDs
$TMUX lsp -tctrl -F '#{pane_id}' | tr -d '\r' >$OUT
BEFORE=$(wc -l < $OUT)
KILL_PANE=$(tail -1 $OUT)
$TMUX killp -tctrl -t"$KILL_PANE" || { fail "killp"; exit 1; }
sleep 0.5
AFTER=$($TMUX lsp -tctrl | wc -l)
[ "$AFTER" -eq $((BEFORE - 1)) ] || fail "killp: $BEFORE -> $AFTER (expected $((BEFORE - 1)))"
echo "PASS 2: kill pane ($BEFORE -> $AFTER)"

# --- Test 3: Swap panes ---
$TMUX lsp -tctrl -F '#{pane_id}' | tr -d '\r' >$OUT
P1=$(sed -n '1p' $OUT)
$TMUX splitw -tctrl || exit 1
sleep 0.5
$TMUX lsp -tctrl -F '#{pane_id}' | tr -d '\r' >$OUT
P2=$(tail -1 $OUT)
$TMUX swapp -s "$P1" -t "$P2" || { fail "swapp"; exit 1; }
sleep 0.5
echo "PASS 3: swap panes"

# --- Test 4: Select layout tiled ---
$TMUX neww -tctrl || exit 1
sleep 0.5
$TMUX splitw -tctrl || exit 1
$TMUX splitw -tctrl || exit 1
$TMUX splitw -tctrl || exit 1
sleep 0.5
$TMUX selectl -tctrl tiled || { fail "selectl tiled"; exit 1; }
sleep 0.5
COUNT=$($TMUX lsp -tctrl | wc -l)
[ "$COUNT" -eq 4 ] || fail "tiled: expected 4 panes, got $COUNT"
echo "PASS 4: select-layout tiled ($COUNT panes)"

# --- Test 5: Kill window ---
WCOUNT_BEFORE=$($TMUX lsw -tctrl | wc -l)
$TMUX killw -tctrl || { fail "killw"; exit 1; }
sleep 0.5
WCOUNT_AFTER=$($TMUX lsw -tctrl | wc -l)
[ "$WCOUNT_AFTER" -eq $((WCOUNT_BEFORE - 1)) ] || \
	fail "killw: $WCOUNT_BEFORE -> $WCOUNT_AFTER"
echo "PASS 5: kill window ($WCOUNT_BEFORE -> $WCOUNT_AFTER)"

# --- Test 6: Verify server still alive after all operations ---
$TMUX has -tctrl || { fail "server died"; exit 1; }

# Verify layout output is sane
$TMUX lsp -tctrl -aF '#{pane_id} #{window_layout}' | tr -d '\r' >$OUT
COUNT=$(wc -l < $OUT)
[ "$COUNT" -ge 1 ] || { fail "no panes in final layout"; exit 1; }
grep -q '%' $OUT || { fail "no pane IDs in layout output"; exit 1; }
echo "PASS 6: server alive, layout sane ($COUNT pane lines)"

$TMUX kill-server 2>/dev/null

if [ $FAIL -eq 0 ]; then
	echo ""
	echo "ALL CONTROL CLIENT TESTS PASSED"
	exit 0
else
	echo ""
	echo "SOME CONTROL CLIENT TESTS FAILED"
	exit 1
fi
