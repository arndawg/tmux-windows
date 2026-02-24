#!/bin/sh

# Windows port of upstream format-strings.sh.
# Tests format string expansion as described in tmux(1) FORMATS.
# Covers: plain strings, escapes, simple expansion, conditionals,
# boolean ops, comparisons, nested conditionals, and literal mode.

PATH=/bin:/usr/bin
TERM=screen

[ -z "$TEST_TMUX" ] && TEST_TMUX=$(readlink -f ../tmux)
TMUX="$TEST_TMUX -Ltest"
$TMUX kill-server 2>/dev/null
sleep 1

FNULL="-fNUL"

# test_format $format $expected_result
test_format()
{
	fmt="$1"
	exp="$2"

	out=$($TMUX display-message -p "$fmt" | tr -d '\r')

	if [ "$out" != "$exp" ]; then
		echo "Format test failed for '$fmt'."
		echo "Expected: '$exp'"
		echo "But got   '$out'"
		exit 1
	fi
}

# test_conditional_with_pane_in_mode $format $exp1 $exp2
test_conditional_with_pane_in_mode()
{
	fmt="$1"
	exp_true="$2"
	exp_false="$3"

	$TMUX copy-mode # enter copy mode
	test_format "$fmt" "$exp_true"
	$TMUX send-keys -X cancel # leave copy mode
	test_format "$fmt" "$exp_false"
}

# test_conditional_with_session_name $format $exp_summer $exp_winter
test_conditional_with_session_name()
{
	fmt="$1"
	exp_summer="$2"
	exp_winter="$3"

	$TMUX rename-session "Summer"
	test_format "$fmt" "$exp_summer"
	$TMUX rename-session "Winter"
	test_format "$fmt" "$exp_winter"
	$TMUX rename-session "Summer" # restore default
}

trap "$TMUX kill-server 2>/dev/null" 0 1 15

$TMUX $FNULL new-session -d || exit 1
sleep 1

# used later in conditionals
$TMUX rename-session "Summer" || exit 1
$TMUX set @true 1 || exit 1
$TMUX set @false 0 || exit 1
$TMUX set @warm Summer || exit 1
$TMUX set @cold Winter || exit 1

# Plain string without substitutions
test_format "abc xyz" "abc xyz"

# Basic escapes for "#", "{", "#{" "}", "#}", ","
test_format "##" "#"
test_format "#," ","
test_format "{" "{"
test_format "##{" "#{"
test_format "#}" "}"
test_format "###}" "#}"

# Simple expansion
test_format "#{pane_in_mode}" "0"

# Simple conditionals
test_format "#{?}" ""
test_format "#{?abc}" "abc"
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc}" "abc" ""
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,xyz}" "abc" "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,@true,xyz}" "abc" "xyz"
test_format "#{?@false,abc,@false,xyz}" ""
test_format "#{?@false,abc,@false,xyz,default}" "default"

# Expansion in conditionals
test_format "#{?#{@warm}}" "Summer"
test_conditional_with_pane_in_mode "#{?#{pane_in_mode},#{@warm}}" "Summer" ""
test_conditional_with_pane_in_mode "#{?#{pane_in_mode},#{@warm},#{@cold}}" "Summer" "Winter"

# Basic escapes in conditionals
test_conditional_with_pane_in_mode "#{?pane_in_mode,##,xyz}" "#" "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,#,,xyz}" "," "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,{,xyz}" "{" "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,##{,xyz}" "#{" "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,#},xyz}" "}" "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,###},xyz}" "#}" "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,##}" "abc" "#"
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,#,}" "abc" ","
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,{}" "abc" "{"
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,##{}" "abc" "#{"
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,#}}" "abc" "}"
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,###}}" "abc" "#}"
test_conditional_with_pane_in_mode "#{?pane_in_mode,{,#}}" "{" "}"
test_conditional_with_pane_in_mode "#{?pane_in_mode,#},{}" "}" "{"
test_conditional_with_pane_in_mode "#{?pane_in_mode,##{,###}}" "#{" "#}"
test_conditional_with_pane_in_mode "#{?pane_in_mode,###},##{}" "#}" "#{"

# Curly brackets {...} do not capture a comma inside of conditionals
test_conditional_with_pane_in_mode "#{?pane_in_mode,{abc,xyz},bonus}" "{abc,bonus}" "xyz,bonus}"

# Parenthesis (...) do not capture a comma
test_conditional_with_pane_in_mode "#{?pane_in_mode,(abc,xyz),bonus}" "(abc" ""
test_conditional_with_pane_in_mode "#{?pane_in_mode,(abc#,xyz),bonus}" "(abc,xyz)" "bonus"

# Brackets [...] do not capture a comma
test_conditional_with_pane_in_mode "#{?pane_in_mode,[abc,xyz],bonus}" "[abc" ""
test_conditional_with_pane_in_mode "#{?pane_in_mode,[abc#,xyz],bonus}" "[abc,xyz]" "bonus"

# Skipped: #() command substitution tests depend on Unix shell (cmd.exe on Windows).
# test_format "#{?pane_in_mode,#(echo #,),xyz}" "xyz"
# test_conditional_with_pane_in_mode "#{?pane_in_mode,#(echo #,),xyz}" "" "xyz"
# test_conditional_with_pane_in_mode "#{?pane_in_mode,#(echo ,)xyz}" "" ")xyz"

# Escape comma inside of #[...]
test_conditional_with_pane_in_mode "#{?pane_in_mode,#[fg=default#,bg=default]abc,xyz}" "#[fg=default,bg=default]abc" "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,#[fg=default,bg=default]abc}" "#[fg=default" "bg=default]abc"

# Conditionals with comparison
test_conditional_with_session_name "#{?#{==:#{session_name},Summer},abc,xyz}" "abc" "xyz"
# Conditionals with comparison and escaped commas
$TMUX rename-session ","
test_format "#{?#{==:#,,#{session_name}},abc,xyz}" "abc"
$TMUX rename-session "Summer"

# Conditional in conditional
test_conditional_with_pane_in_mode "#{?pane_in_mode,#{?#{==:#{session_name},Summer},ABC,XYZ},xyz}" "ABC" "xyz"
test_conditional_with_session_name "#{?pane_in_mode,#{?#{==:#{session_name},Summer},ABC,XYZ},xyz}" "xyz" "xyz"
test_conditional_with_pane_in_mode "#{?pane_in_mode,abc,#{?#{==:#{session_name},Summer},ABC,XYZ}}" "abc" "ABC"
test_conditional_with_session_name "#{?pane_in_mode,abc,#{?#{==:#{session_name},Summer},ABC,XYZ}}" "ABC" "XYZ"

# Skipped: fancy stacking test uses #() which depends on Unix shell.
# test_conditional_with_pane_in_mode "#{?#{==:#{?pane_in_mode,#{session_name},#(echo Spring)},Summer},abc,xyz}" "abc" "xyz"

# Boolean expressions: "0" and "" are false, everything else is true
test_format "#{!!:0}" "0"
test_format "#{!!:}" "0"
test_format "#{!!:1}" "1"
test_format "#{!!:2}" "1"
test_format "#{!!:non-empty string}" "1"
test_format "#{!!:-0}" "1"
test_format "#{!!:0.0}" "1"

# Logical operators
test_format "#{!:0}" "1"
test_format "#{!:1}" "0"

test_format "#{&&:0}" "0"
test_format "#{&&:1}" "1"
test_format "#{&&:0,0}" "0"
test_format "#{&&:0,1}" "0"
test_format "#{&&:1,0}" "0"
test_format "#{&&:1,1}" "1"
test_format "#{&&:0,0,0}" "0"
test_format "#{&&:0,1,1}" "0"
test_format "#{&&:1,0,1}" "0"
test_format "#{&&:1,1,0}" "0"
test_format "#{&&:1,1,1}" "1"

test_format "#{||:0}" "0"
test_format "#{||:1}" "1"
test_format "#{||:0,0}" "0"
test_format "#{||:0,1}" "1"
test_format "#{||:1,0}" "1"
test_format "#{||:1,1}" "1"
test_format "#{||:0,0,0}" "0"
test_format "#{||:1,0,0}" "1"
test_format "#{||:0,1,0}" "1"
test_format "#{||:0,0,1}" "1"
test_format "#{||:1,1,1}" "1"

# Literal option
test_format "#{l:#{}}" "#{}"
test_format "#{l:#{pane_in_mode}}" "#{pane_in_mode}"
test_format "#{l:#{?pane_in_mode,#{?#{==:#{session_name},Summer},ABC,XYZ},xyz}}" "#{?pane_in_mode,#{?#{==:#{session_name},Summer},ABC,XYZ},xyz}"

# Literal with escapes
test_format "#{l:##{}" "#{"
test_format "#{l:#{#}}}" "#{#}}"

echo "ALL FORMAT TESTS PASSED"
exit 0
