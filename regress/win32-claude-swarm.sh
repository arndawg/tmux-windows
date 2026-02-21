#!/bin/sh

# Claude Code agent-team swarm simulation test.
# Exercises the exact tmux command sequences that Claude Code emits when
# running split-pane agent teams: rapid multi-split, parallel send-keys,
# capture-pane round-trips, pane targeting, and teardown under load.
# Must be run on Windows (Git Bash or similar).
#
# Usage:
#   TEST_TMUX=./build/Debug/tmux.exe bash regress/win32-claude-swarm.sh

PATH=/bin:/usr/bin
TERM=screen

[ -z "$TEST_TMUX" ] && TEST_TMUX=$(readlink -f ../tmux)
TMUX="$TEST_TMUX -Lswarm"
$TMUX kill-server 2>/dev/null
sleep 1

OUT=$(mktemp)
trap "rm -f $OUT; $TMUX kill-server 2>/dev/null" 0 1 15

FNULL="-fNUL"
FAIL=0

fail() {
	echo "FAIL: $1"
	FAIL=1
}

# --- Test 1: Version gate ---
# Claude Code runs `tmux -V` at startup to detect availability.
VER=$($TEST_TMUX -V 2>&1 | tr -d '\r')
case "$VER" in
	tmux\ *) ;;
	*) fail "tmux -V returned unexpected: $VER"; exit 1 ;;
esac
echo "PASS 1: tmux -V => $VER"

# --- Test 2: has-session on non-existent session ---
# Claude Code checks session existence before creating.
$TMUX has-session -t swarm 2>/dev/null
if [ $? -eq 0 ]; then
	fail "has-session should fail for non-existent session"
	exit 1
fi
echo "PASS 2: has-session correctly reports missing session"

# Run the full swarm lifecycle CYCLES times to catch handle leaks.
CYCLES=3
CYCLE=0
while [ $CYCLE -lt $CYCLES ]; do
	CYCLE=$((CYCLE + 1))
	echo "--- Cycle $CYCLE/$CYCLES ---"

	# --- Test 3: Session create with explicit dimensions ---
	$TMUX $FNULL new-session -d -s swarm -x 120 -y 40 < /dev/null || {
		fail "cycle $CYCLE: new-session failed"; exit 1
	}
	sleep 1

	# --- Test 4: Rapid multi-split (1 horizontal + 2 vertical) ---
	# Claude Code splits once per teammate. Typical team = 3-5 teammates.
	$TMUX split-window -h -t swarm || { fail "cycle $CYCLE: split-window -h failed"; exit 1; }
	$TMUX split-window -v -t swarm || { fail "cycle $CYCLE: split-window -v (1) failed"; exit 1; }
	$TMUX split-window -v -t swarm:0.0 || { fail "cycle $CYCLE: split-window -v (2) failed"; exit 1; }
	sleep 2

	# Verify 4 panes exist.
	COUNT=$($TMUX list-panes -t swarm | wc -l)
	[ "$COUNT" -eq 4 ] || { fail "cycle $CYCLE: expected 4 panes, got $COUNT"; exit 1; }
	echo "  PASS 4.$CYCLE: 4 panes created"

	# --- Test 5: Pane targeting with session:window.pane format ---
	# Claude Code addresses each teammate pane individually.
	PANE=0
	while [ $PANE -lt 4 ]; do
		$TMUX send-keys -t "swarm:0.$PANE" "echo SWARM_PANE_${PANE}_OK" Enter || {
			fail "cycle $CYCLE: send-keys to pane $PANE failed"; exit 1
		}
		PANE=$((PANE + 1))
	done
	sleep 3

	# --- Test 6: Capture-pane round-trip for each pane ---
	PANE=0
	while [ $PANE -lt 4 ]; do
		$TMUX capture-pane -t "swarm:0.$PANE" -p | tr -d '\r' >$OUT
		grep -q "SWARM_PANE_${PANE}_OK" $OUT || {
			fail "cycle $CYCLE: capture-pane $PANE missing expected output"
			echo "  pane $PANE content:"
			$TMUX capture-pane -t "swarm:0.$PANE" -p | tr -d '\r' | head -5
			exit 1
		}
		PANE=$((PANE + 1))
	done
	echo "  PASS 6.$CYCLE: all 4 panes echo round-trip OK"

	# --- Test 7: list-panes format string (Claude Code reads pane metadata) ---
	$TMUX list-panes -t swarm \
		-F '#{pane_index} #{pane_width} #{pane_height} #{pane_active}' \
		| tr -d '\r' >$OUT
	FCOUNT=$(wc -l < $OUT)
	[ "$FCOUNT" -eq 4 ] || { fail "cycle $CYCLE: list-panes format returned $FCOUNT lines"; exit 1; }
	echo "  PASS 7.$CYCLE: list-panes format output OK"

	# --- Test 8: Rapid teardown while panes are active ---
	$TMUX kill-session -t swarm || { fail "cycle $CYCLE: kill-session failed"; exit 1; }
	sleep 1

	# Verify session is gone.
	$TMUX has-session -t swarm 2>/dev/null
	if [ $? -eq 0 ]; then
		fail "cycle $CYCLE: session still exists after kill-session"
		exit 1
	fi
	echo "  PASS 8.$CYCLE: session torn down cleanly"

	$TMUX kill-server 2>/dev/null
	sleep 2
done

# --- Test 9: Multi-session (simulates multiple teams) ---
# Claude Code could run multiple teams or restart teams.
echo "--- Multi-session test ---"
$TMUX $FNULL new-session -d -s team-a -x 100 -y 30 < /dev/null || { fail "new-session team-a"; exit 1; }
$TMUX $FNULL new-session -d -s team-b -x 100 -y 30 < /dev/null || { fail "new-session team-b"; exit 1; }
sleep 1

$TMUX split-window -h -t team-a || { fail "split team-a"; exit 1; }
$TMUX split-window -h -t team-b || { fail "split team-b"; exit 1; }
sleep 1

# Verify both sessions have 2 panes each.
CA=$($TMUX list-panes -t team-a | wc -l)
CB=$($TMUX list-panes -t team-b | wc -l)
[ "$CA" -eq 2 ] && [ "$CB" -eq 2 ] || { fail "multi-session: team-a=$CA team-b=$CB panes"; exit 1; }

# Cross-session send-keys + capture.
$TMUX send-keys -t team-a:0.0 "echo TEAM_A" Enter
$TMUX send-keys -t team-b:0.0 "echo TEAM_B" Enter
sleep 2
$TMUX capture-pane -t team-a:0.0 -p | tr -d '\r' | grep -q "TEAM_A" || { fail "capture team-a"; exit 1; }
$TMUX capture-pane -t team-b:0.0 -p | tr -d '\r' | grep -q "TEAM_B" || { fail "capture team-b"; exit 1; }

$TMUX kill-session -t team-a
$TMUX kill-session -t team-b
$TMUX kill-server 2>/dev/null
sleep 1
echo "PASS 9: multi-session OK"

# --- Test 10: Rapid create-destroy cycle (handle leak detection) ---
echo "--- Rapid create-destroy test ---"
RAPID=0
while [ $RAPID -lt 5 ]; do
	$TMUX $FNULL new-session -d -s "rapid$RAPID" -x 80 -y 24 < /dev/null || {
		fail "rapid cycle $RAPID: new-session failed"; exit 1
	}
	$TMUX split-window -h -t "rapid$RAPID" || {
		fail "rapid cycle $RAPID: split-window failed"; exit 1
	}
	$TMUX kill-session -t "rapid$RAPID" || {
		fail "rapid cycle $RAPID: kill-session failed"; exit 1
	}
	RAPID=$((RAPID + 1))
done
$TMUX kill-server 2>/dev/null
sleep 1
echo "PASS 10: 5 rapid create-destroy cycles OK"

if [ $FAIL -eq 0 ]; then
	echo ""
	echo "ALL TESTS PASSED"
	exit 0
else
	echo ""
	echo "SOME TESTS FAILED"
	exit 1
fi
