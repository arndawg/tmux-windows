#!/bin/sh

# Windows-specific basic functionality tests.
# Tests ConPTY spawn, I/O, split windows, pane exit cleanup, session size,
# /dev/null config translation, Unix -S path warning, and default-shell guard.
# Must be run on Windows (Git Bash or similar).

PATH=/bin:/usr/bin
TERM=screen

[ -z "$TEST_TMUX" ] && TEST_TMUX=$(readlink -f ../tmux)
TMUX="$TEST_TMUX -Ltest"
$TMUX kill-server 2>/dev/null

OUT=$(mktemp)
trap "rm -f $OUT; $TMUX kill-server 2>/dev/null" 0 1 15

# On Windows, -f/dev/null doesn't work (native exe can't open Unix path).
# Use NUL instead.
FNULL="-fNUL"

# 1. Session lifecycle: new, ls, kill-session
$TMUX $FNULL new -d -sfoo < /dev/null || exit 1
$TMUX ls -F '#{session_name}' | tr -d '\r' >$OUT
printf "foo\n" | cmp -s $OUT - || exit 1
$TMUX kill-session -tfoo
$TMUX kill-server 2>/dev/null
sleep 1

# 2. Send-keys + capture-pane
$TMUX $FNULL new -d -sio < /dev/null || exit 1
sleep 1
$TMUX send-keys -tio "echo TMUX_TEST_OK" Enter
sleep 2
$TMUX capture-pane -tio -p | tr -d '\r' >$OUT
grep -q "TMUX_TEST_OK" $OUT || exit 1
$TMUX kill-server 2>/dev/null
sleep 1

# 3. Split-window + list-panes
$TMUX $FNULL new -d -ssplit < /dev/null || exit 1
sleep 1
$TMUX split-window -tsplit || exit 1
sleep 1
COUNT=$($TMUX list-panes -tsplit | wc -l)
[ "$COUNT" -eq 2 ] || exit 1
$TMUX kill-server 2>/dev/null
sleep 1

# 4. Pane exit cleanup (tests server_destroy_pane ConPTY path)
$TMUX $FNULL new -d -sexit < /dev/null || exit 1
sleep 1
$TMUX split-window -texit || exit 1
sleep 1
# Kill the second pane by sending exit
$TMUX send-keys -texit:0.1 "exit" Enter
sleep 3
COUNT=$($TMUX list-panes -texit | wc -l)
[ "$COUNT" -eq 1 ] || exit 1
$TMUX kill-server 2>/dev/null
sleep 1

# 5. Detached session size with explicit dimensions
$TMUX $FNULL new -d -x 120 -y 40 < /dev/null || exit 1
sleep 1
$TMUX ls -F "#{window_width} #{window_height}" | tr -d '\r' >$OUT
printf "120 40\n" | cmp -s $OUT - || exit 1
$TMUX kill-server 2>/dev/null
sleep 1

# 6. -f /dev/null translation: should work identically to -fNUL
$TMUX -f /dev/null new -d -snull < /dev/null || exit 1
$TMUX ls -F '#{session_name}' | tr -d '\r' >$OUT
printf "null\n" | cmp -s $OUT - || exit 1
$TMUX kill-server 2>/dev/null
sleep 1

# 7. -S with Unix path: should work as IPC label and emit warning
# MSYS_NO_PATHCONV prevents Git Bash from translating /tmp to C:/...
WARN=$(MSYS_NO_PATHCONV=1 $TEST_TMUX -S /tmp/unix-test-sock -fNUL new -d -sunix 2>&1)
echo "$WARN" | grep -qi "unix" || exit 1
MSYS_NO_PATHCONV=1 $TEST_TMUX -S /tmp/unix-test-sock ls -F '#{session_name}' | tr -d '\r' >$OUT
printf "unix\n" | cmp -s $OUT - || exit 1
MSYS_NO_PATHCONV=1 $TEST_TMUX -S /tmp/unix-test-sock kill-server 2>/dev/null
sleep 1

# 8. default-shell guard: Unix shell path falls back to cmd.exe
CFG=$(mktemp)
printf 'set -g default-shell /bin/bash\n' >$CFG
$TMUX -f "$CFG" new -d -sshell < /dev/null || exit 1
sleep 1
# Pane should still be alive (cmd.exe fallback worked)
COUNT=$($TMUX list-panes -tshell | wc -l)
[ "$COUNT" -eq 1 ] || exit 1
$TMUX kill-server 2>/dev/null
rm -f "$CFG"

exit 0
