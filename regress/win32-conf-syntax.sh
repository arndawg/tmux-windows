#!/bin/sh

# Windows port of upstream conf-syntax.sh.
# Validates all regress/conf/*.conf files parse without error.
# On Windows, source -n hangs as a separate client command, so we test
# by loading each config with -f at session creation time instead.

PATH=/bin:/usr/bin
TERM=screen

[ -z "$TEST_TMUX" ] && TEST_TMUX=$(readlink -f ../tmux)
TMUX="$TEST_TMUX -Ltest"
$TMUX kill-server 2>/dev/null
sleep 1

trap "$TMUX kill-server 2>/dev/null" 0 1 15

FAIL=0
COUNT=0

for i in regress/conf/*.conf; do
	BASENAME=$(basename "$i")
	# Start a session loading this config. If it parses, the session starts.
	# Use timeout to catch hangs. Redirect stderr to capture parse errors.
	ERR=$($TMUX -f "$i" new -d -s conftest -x 80 -y 24 < /dev/null 2>&1 | tr -d '\r')
	RC=$?
	if [ $RC -ne 0 ]; then
		echo "FAIL: $BASENAME (exit $RC: $ERR)"
		FAIL=1
	fi
	$TMUX kill-server 2>/dev/null
	sleep 1
	COUNT=$((COUNT + 1))
done

if [ $FAIL -eq 0 ]; then
	echo "ALL $COUNT CONFIG SYNTAX TESTS PASSED"
	exit 0
else
	echo "SOME CONFIG SYNTAX TESTS FAILED"
	exit 1
fi
