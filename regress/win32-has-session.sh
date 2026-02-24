#!/bin/sh

# Windows port of upstream has-session-return.sh.
# Validates exit codes for has-session command.

PATH=/bin:/usr/bin
TERM=screen

[ -z "$TEST_TMUX" ] && TEST_TMUX=$(readlink -f ../tmux)
TMUX="$TEST_TMUX -Ltest"
$TMUX kill-server 2>/dev/null
sleep 1

FNULL="-fNUL"
trap "$TMUX kill-server 2>/dev/null" 0 1 15

# has-session with no server should fail
$TMUX $FNULL has -tfoo </dev/null 2>/dev/null && exit 1

# start server, has-session for non-existent session should fail
$TMUX $FNULL new -d </dev/null || exit 1
sleep 1
$TMUX has -tfoo 2>/dev/null && exit 1

# create named session, has-session should succeed
$TMUX kill-server 2>/dev/null
sleep 1
$TMUX $FNULL new -dsfoo </dev/null || exit 1
sleep 1
$TMUX has -tfoo </dev/null 2>/dev/null || exit 1

$TMUX kill-server 2>/dev/null

echo "ALL HAS-SESSION TESTS PASSED"
exit 0
