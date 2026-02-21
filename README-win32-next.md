# tmux Windows Port — Next Steps for Claude Code Agent Teams

## Project Summary

This is a Windows port of tmux built on top of the upstream `tmux/tmux` repository.
Across 7 commits and ~5,950 lines of new code (75 files touched), it replaces
Unix-specific subsystems with Windows equivalents:

| Unix Mechanism | Windows Replacement |
|---|---|
| Unix domain sockets | TCP loopback + auth tokens |
| `/dev/pty` | ConPTY (`CreatePseudoConsole`) |
| `SIGCHLD` / `SIGWINCH` | Watcher thread + console events |
| ncurses/terminfo | Hardcoded xterm-256color table |
| `fork()` / `forkpty()` | `CreateProcess` + ConPTY bridge threads |
| `SCM_RIGHTS` fd passing | TTY channel with token auth |

It builds with CMake + MSVC, targets Windows 10 1809+, and has 8 regression
tests covering session lifecycle, ConPTY I/O, split panes, pane cleanup, CLI
compatibility, and more.

## How Claude Code Agent Teams Use tmux

Claude Code's split-pane agent teams mode uses tmux to give each teammate its
own visible terminal pane. The key tmux commands it depends on:

- `tmux new-session` / `tmux has-session` — session management
- `tmux split-window -h/-v` — create teammate panes
- `tmux send-keys` — deliver input to teammates
- `tmux capture-pane` — read teammate output
- `tmux list-panes` / `tmux kill-session` — lifecycle management

### Current Windows Status

Split-pane mode is **blocked on Windows** even with tmux available, due to a
`process.stdout.isTTY` gate in Claude Code's Bun SFE binary
([GitHub issue #26244](https://github.com/anthropics/claude-code/issues/26244)).
The `in-process` mode works on Windows but has no visual pane separation.

## Next Steps

### Critical (Must-Have)

#### 1. Fix the Claude Code isTTY Gate

Even with a working Windows tmux, Claude Code's split-pane detection is broken
on Windows (issue #26244). This is a Claude Code bug, not a tmux bug, but it's
the #1 blocker. Engage with that issue or submit a PR to the Claude Code repo
showing that tmux is now available on Windows.

#### 2. Publish Pre-Built Binaries

Other developers need a `tmux.exe` they can drop into `%PATH%`. Create GitHub
Releases with a built `tmux.exe` + `event_core.dll`. Without this, nobody will
use it.

#### 3. Fork to a Dedicated Repo

Currently this sits on top of the upstream `tmux/tmux` remote. Create a
dedicated fork (e.g., `tmux-windows` or `tmux-win32`) so you can accept PRs,
track issues, and publish releases independently.

### High Priority

#### 4. vcpkg or winget Package

Claude Code's docs tell users to install tmux via their "system's package
manager." On Windows, that means `winget install tmux` or `vcpkg install tmux`
should work. A winget manifest or scoop bucket would dramatically lower the
barrier.

#### 5. CI Pipeline

Add a GitHub Actions workflow that builds on Windows with MSVC and runs
`regress/win32-basic.sh`. This proves the build stays green and gives
contributors confidence.

#### 6. Test `tmux -CC` Control Mode

The docs mention control mode as the "suggested entrypoint" via iTerm2. Verify
it works on Windows, or document if it doesn't. Claude Code may use this path
on some platforms.

#### 7. Test the Specific Claude Code Command Sequences

Write a test that exercises exactly what Claude Code does: `new-session`,
`split-window`, `send-keys`, `capture-pane`, `list-panes`, `kill-session` in
rapid succession. The existing tests cover most of these individually but not
the rapid-fire sequencing an agent swarm would do.

See [Detailed Plan for #7](#detailed-plan-for-7-claude-code-command-sequence-test)
below.

### Medium Priority

#### 8. Harden Concurrent Session Handling

Agent teams can spawn 4-6 teammates, each in its own pane. Test that multiple
simultaneous sessions with many panes and rapid `send-keys` traffic is stable
and doesn't leak ConPTY handles.

#### 9. Windows Terminal Integration as Alternative

[Issue #24384](https://github.com/anthropics/claude-code/issues/24384) proposes
using `wt.exe split-pane` as a native Windows alternative to tmux. If Claude
Code adds that backend, tmux won't be needed. But until then, tmux is the only
path.

#### 10. Documentation for Claude Code Users

Add a section to `README_WIN32.MD` specifically titled "Using with Claude Code
Agent Teams" with setup instructions: build/install tmux, set
`teammateMode: "tmux"`, and the known isTTY workaround.

#### 11. Upstream Contribution

Consider submitting the port (or parts of it) to the upstream `tmux/tmux`
project. Even if they don't merge it, opening the discussion increases
visibility.

### Nice to Have

#### 12. MSI Installer or Portable ZIP

Package `tmux.exe` + DLLs into a single downloadable artifact.

#### 13. Shell Completion for PowerShell

tmux has bash completion; add PowerShell tab completion for Windows-native
shell users.

#### 14. Performance Benchmarking

Compare ConPTY I/O latency vs Unix PTY to identify bottlenecks that could
affect swarm responsiveness.

---

## Detailed Plan for #7: Claude Code Command Sequence Test

### Goal

Validate that the tmux Windows port can handle the exact command sequences that
Claude Code agent teams emit, including rapid-fire session/pane creation, I/O
delivery, output capture, and teardown — all within a single test run.

### What Claude Code Does

When a user starts an agent team with split-pane mode, Claude Code executes
roughly this sequence per teammate:

```
tmux -V                                  # version check at startup
tmux has-session -t <session>            # check if session exists
tmux new-session -d -s <session> -x W -y H   # create detached session
tmux split-window -h -t <session>        # horizontal split for teammate 1
tmux split-window -v -t <session>        # vertical split for teammate 2
tmux split-window -v -t <session>        # vertical split for teammate 3
tmux send-keys -t <session>:<win>.<pane> "claude ..." Enter  # launch teammate
tmux capture-pane -t <session>:<win>.<pane> -p               # read output
tmux list-panes -t <session> -F '...'    # check pane status
tmux kill-session -t <session>           # teardown
```

Key stress points:
- Multiple `split-window` calls in quick succession (ConPTY handle allocation)
- `send-keys` + `capture-pane` round-trips (ConPTY I/O bridge latency)
- Concurrent pane I/O (multiple bridge threads active simultaneously)
- Rapid teardown after activity (ConPTY handle cleanup under load)

### Test Script Structure

The test should be added as `regress/win32-claude-swarm.sh` and cover:

1. **Version gate** — `tmux -V` succeeds and returns a version string
2. **Session create with dimensions** — `new-session -d -s swarm -x 120 -y 40`
3. **Rapid multi-split** — 3 consecutive `split-window` calls (1 horizontal +
   2 vertical), verify 4 panes exist via `list-panes`
4. **Parallel send-keys** — Send a unique echo command to each of the 4 panes
5. **Capture-pane round-trip** — Wait briefly, then `capture-pane -p` each pane
   and verify the expected output appears
6. **Pane targeting** — Use `<session>:<window>.<pane>` format (e.g.,
   `swarm:0.0`, `swarm:0.1`) to confirm targeting works
7. **Rapid teardown** — `kill-session` while panes are still active, verify
   clean exit (no hanging ConPTY handles)
8. **Repeat under load** — Run the full cycle 3 times back-to-back to catch
   handle leaks or state pollution between sessions

### Success Criteria

- All 4 panes created and addressable
- `send-keys` + `capture-pane` round-trip succeeds for each pane
- `kill-session` exits cleanly (no error output, no hung processes)
- 3 consecutive cycles complete without failure
- No orphaned `conhost.exe` processes after teardown
