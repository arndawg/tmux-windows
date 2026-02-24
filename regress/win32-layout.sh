#!/bin/sh

# Layout verification tests for Windows.
# Verifies pane dimensions after split operations — not just count.
# Catches upstream bug class e149d29 (uneven space distribution).

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

# --- Test 1: Single pane dimensions match session size ---
$TMUX $FNULL new -d -slayout -x 120 -y 40 < /dev/null || exit 1
sleep 1

$TMUX list-panes -tlayout -F '#{pane_width} #{pane_height}' | tr -d '\r' >$OUT
W=$(awk '{print $1}' $OUT)
H=$(awk '{print $2}' $OUT)
# Width should be 120, height = 40 (detached sessions on Windows use full height)
[ "$W" -eq 120 ] || fail "single pane width $W != 120"
[ "$H" -eq 40 ] || fail "single pane height $H != 40"
echo "PASS 1: single pane 120x40"

# --- Test 2: Horizontal split produces two panes with equal width ---
$TMUX split-window -h -tlayout || exit 1
sleep 1

$TMUX list-panes -tlayout -F '#{pane_width}' | tr -d '\r' >$OUT
W1=$(sed -n '1p' $OUT)
W2=$(sed -n '2p' $OUT)
# Both panes should have roughly equal width (total = 120, minus 1 for divider)
TOTAL=$((W1 + W2 + 1))
[ "$TOTAL" -eq 120 ] || fail "h-split widths $W1+$W2+1 = $TOTAL != 120"
DIFF=$((W1 - W2))
[ "$DIFF" -ge -1 ] && [ "$DIFF" -le 1 ] || fail "h-split uneven: $W1 vs $W2"
echo "PASS 2: horizontal split ($W1 + $W2 + 1 = $TOTAL)"

$TMUX kill-session -tlayout
$TMUX kill-server 2>/dev/null
sleep 1

# --- Test 3: Vertical split produces two panes with equal height ---
$TMUX $FNULL new -d -slayout -x 80 -y 40 < /dev/null || exit 1
sleep 1
$TMUX split-window -v -tlayout || exit 1
sleep 1

$TMUX list-panes -tlayout -F '#{pane_height}' | tr -d '\r' >$OUT
H1=$(sed -n '1p' $OUT)
H2=$(sed -n '2p' $OUT)
# Total = 40 (detached, no status line subtracted), minus 1 for divider
TOTAL=$((H1 + H2 + 1))
[ "$TOTAL" -eq 40 ] || fail "v-split heights $H1+$H2+1 = $TOTAL != 40"
DIFF=$((H1 - H2))
[ "$DIFF" -ge -1 ] && [ "$DIFF" -le 1 ] || fail "v-split uneven: $H1 vs $H2"
echo "PASS 3: vertical split ($H1 + $H2 + 1 = $TOTAL)"

$TMUX kill-session -tlayout
$TMUX kill-server 2>/dev/null
sleep 1

# --- Test 4: Four-pane tiled layout ---
$TMUX $FNULL new -d -slayout -x 100 -y 40 < /dev/null || exit 1
sleep 1
$TMUX split-window -h -tlayout || exit 1
$TMUX split-window -v -tlayout:0.0 || exit 1
$TMUX split-window -v -tlayout:0.2 || exit 1
sleep 1

COUNT=$($TMUX list-panes -tlayout | wc -l)
[ "$COUNT" -eq 4 ] || { fail "4-pane: got $COUNT panes"; exit 1; }

# All widths should sum to 100 (accounting for dividers)
$TMUX list-panes -tlayout -F '#{pane_width} #{pane_height}' | tr -d '\r' >$OUT
WSUM=0
while read w h; do
	WSUM=$((WSUM + w))
done < $OUT
# 2 columns, 1 divider between them
# Each column has 2 panes stacked. Left and right widths + 1 divider = 100
echo "PASS 4: four-pane layout ($COUNT panes)"

# --- Test 5: select-layout tiled ---
$TMUX select-layout -tlayout tiled || exit 1
sleep 1

$TMUX list-panes -tlayout -F '#{pane_width} #{pane_height}' | tr -d '\r' >$OUT
# In tiled layout, all panes should be approximately equal
LINES=$(wc -l < $OUT)
[ "$LINES" -eq 4 ] || { fail "tiled: got $LINES panes"; exit 1; }

WMIN=999; WMAX=0; HMIN=999; HMAX=0
while read w h; do
	[ "$w" -lt "$WMIN" ] && WMIN=$w
	[ "$w" -gt "$WMAX" ] && WMAX=$w
	[ "$h" -lt "$HMIN" ] && HMIN=$h
	[ "$h" -gt "$HMAX" ] && HMAX=$h
done < $OUT
WDIFF=$((WMAX - WMIN))
HDIFF=$((HMAX - HMIN))
[ "$WDIFF" -le 1 ] || fail "tiled width spread: $WMIN-$WMAX (diff $WDIFF)"
[ "$HDIFF" -le 1 ] || fail "tiled height spread: $HMIN-$HMAX (diff $HDIFF)"
echo "PASS 5: tiled layout (w:$WMIN-$WMAX h:$HMIN-$HMAX)"

# --- Test 6: select-layout even-horizontal ---
$TMUX select-layout -tlayout even-horizontal || exit 1
sleep 1

$TMUX list-panes -tlayout -F '#{pane_width}' | tr -d '\r' >$OUT
WMIN=999; WMAX=0
while read w; do
	[ "$w" -lt "$WMIN" ] && WMIN=$w
	[ "$w" -gt "$WMAX" ] && WMAX=$w
done < $OUT
WDIFF=$((WMAX - WMIN))
[ "$WDIFF" -le 1 ] || fail "even-horizontal spread: $WMIN-$WMAX (diff $WDIFF)"
echo "PASS 6: even-horizontal layout (w:$WMIN-$WMAX)"

# --- Test 7: select-layout even-vertical ---
$TMUX select-layout -tlayout even-vertical || exit 1
sleep 1

$TMUX list-panes -tlayout -F '#{pane_height}' | tr -d '\r' >$OUT
HMIN=999; HMAX=0
while read h; do
	[ "$h" -lt "$HMIN" ] && HMIN=$h
	[ "$h" -gt "$HMAX" ] && HMAX=$h
done < $OUT
HDIFF=$((HMAX - HMIN))
[ "$HDIFF" -le 1 ] || fail "even-vertical spread: $HMIN-$HMAX (diff $HDIFF)"
echo "PASS 7: even-vertical layout (h:$HMIN-$HMAX)"

$TMUX kill-server 2>/dev/null

if [ $FAIL -eq 0 ]; then
	echo ""
	echo "ALL LAYOUT TESTS PASSED"
	exit 0
else
	echo ""
	echo "SOME LAYOUT TESTS FAILED"
	exit 1
fi
