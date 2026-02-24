# tmux-windows TODO

## Open Work

### Claude Code Integration

- [ ] **Fix the Claude Code isTTY gate** — Split-pane mode is blocked on
  Windows due to a `process.stdout.isTTY` gate in Claude Code's Bun SFE binary
  ([#26244](https://github.com/anthropics/claude-code/issues/26244)). This is
  a Claude Code bug, not a tmux bug, but it's the #1 blocker.

- [ ] **Claude Code user documentation** — Add a "Using with Claude Code Agent
  Teams" section to `README_WIN32.MD` with setup instructions: install tmux,
  set `teammateMode: "tmux"`, and the known isTTY workaround.

### Performance

- [ ] **ConPTY I/O benchmarking** — Compare ConPTY latency vs Unix PTY to
  identify bottlenecks that could affect swarm responsiveness.

### Security (open findings from 2026-02-22 audit)

3 of 13 findings remain open after the named pipe IPC overhaul resolved 8
and targeted fixes resolved #9 and #10.

| # | Severity | Finding | Notes |
|---|----------|---------|-------|
| 3 | Critical | `getpeereid()` stub returns uid=0 always | Structural, affects Unix compat layer |
| 7 | High | Socketpair TOCTOU race allows hijacking signal/PTY pipes | `win32-compat.c` pipe creation race |
| 11 | Medium | ConPTY bridge sockets inherit socketpair race | Inherits #7 |

### Nice to Have

- [ ] **Windows Terminal integration** —
  [#24384](https://github.com/anthropics/claude-code/issues/24384) proposes
  `wt.exe split-pane` as a native alternative. If Claude Code adds that
  backend, tmux won't be needed on Windows.

- [ ] **Upstream contribution** — Submit the port (or parts of it) to
  `tmux/tmux`. Even if not merged, the discussion increases visibility.

- [ ] **PowerShell tab completion** — tmux has bash completion; add
  PowerShell completion for Windows-native shell users.

---

## Completed Milestones

### Release Pipeline (v3.5a-win32 → v3.6a-win32.4)

- **CI** — GitHub Actions builds Debug + Release with MSVC, runs 8 regression
  test scripts on every push. vcpkg packages cached.
- **GitHub Releases** — Tagged releases (`v*`) produce downloadable zips with
  `tmux.exe` + `event_core.dll` + SHA256 hash.
- **Winget** — Published as `arndawg.tmux-windows`. PRs merged for v3.5a-win32
  ([#341456](https://github.com/microsoft/winget-pkgs/pull/341456)),
  v3.6a-win32 ([#341769](https://github.com/microsoft/winget-pkgs/pull/341769)),
  v3.6a-win32.1 ([#341850](https://github.com/microsoft/winget-pkgs/pull/341850)),
  v3.6a-win32.3 ([#341919](https://github.com/microsoft/winget-pkgs/pull/341919)),
  v3.6a-win32.4 ([#341990](https://github.com/microsoft/winget-pkgs/pull/341990)).
  v3.6a-win32.2 skipped (went to .3).
- **Repo** — Published at https://github.com/arndawg/tmux-windows.

### TTY Race Condition Fix & Security Hardening (v3.6a-win32.4)

Fixed startup race where the TTY channel's second named-pipe round-trip
delayed `accept()` past `MSG_IDENTIFY_DONE`, causing "open terminal failed"
or blank screens. Three-part fix: early TTY connection (client-side),
deferred cmdq processing with blocking callback + 2s timeout (server-side),
and terminal dimension preservation across `tty_init()` memset for late
arrivals. Also added IPC label sanitization and truecolor-by-default.

### Regression Test Expansion (v3.6a-win32.3)

Expanded from 2 test scripts (18 assertions) to 8 scripts (230+ assertions):
`win32-basic.sh`, `win32-claude-swarm.sh`, `win32-format-strings.sh`,
`win32-conf-syntax.sh`, `win32-keys.sh`, `win32-has-session.sh`,
`win32-layout.sh`, `win32-control-client.sh`.

### Named Pipe IPC Overhaul (v3.6a-win32.2)

Replaced TCP auth token file system with named pipe discovery. Server creates
`\\.\pipe\tmux-<user>-<label>` with kernel-enforced DACL. Clients receive
ephemeral nonce + TCP port via pipe, authenticate over TCP. Eliminated `.auth`
and `.port` files, exponential backoff retry, and 7 security audit findings.
Cold start improved from 2.3s → ~250ms.

### Server Exit Fix

Windows `-D` mode set `CLIENT_NOFORK` which disabled `exit-empty`. Fixed with
`#ifndef _WIN32` guard in `server.c`. Server now auto-exits when all sessions
are destroyed.

### Control Mode

`tmux -CC` is not supported on Windows — libevent `evsig_init_` socketpair
fails. Documented as a known limitation.
