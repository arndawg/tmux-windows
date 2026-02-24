# tmux-windows Release Pipeline

## Stage 1: GitHub Actions CI (build + test on every push) — DONE

- [x] Create `.github/workflows/ci.yml`
- [x] Install libevent via vcpkg on `windows-latest` runner
- [x] Build Debug + Release with CMake + MSVC
- [x] Run `regress/win32-basic.sh` via Git Bash
- [x] Run `regress/win32-claude-swarm.sh` via Git Bash
- [x] Cache vcpkg packages for faster CI runs

Note: WinFlexBison not needed — pre-generated `cmd-parse.c` is committed.

## Stage 2: GitHub Release on tag push — DONE

- [x] Add release job to CI workflow (triggered on `v*` tags)
- [x] Build Release configuration
- [x] Zip `tmux.exe` + `event_core.dll` into `tmux-windows-<version>.zip`
- [x] Generate SHA256 hash of the zip
- [x] Create GitHub Release with zip + hash as assets
- [x] Include brief install instructions in release notes

First release: https://github.com/arndawg/tmux-windows/releases/tag/v3.5a-win32

## Stage 3: Winget manifest — DONE (pending merge)

- [x] Create winget manifest YAML (`arndawg.tmux-windows`)
- [x] Use `portable` installer type (same as fzf, ripgrep)
- [x] Point `InstallerUrl` at GitHub Release asset
- [x] Include SHA256 hash
- [x] Set `Commands: [tmux]` so winget creates the symlink
- [x] Submit PR to `microsoft/winget-pkgs`
- [x] Validation pipeline passed, labeled `Azure-Pipeline-Passed`
- [ ] **Check back**: Wait for Microsoft reviewer to approve and merge
  - PR: https://github.com/microsoft/winget-pkgs/pull/341456
  - Typical turnaround: 1-3 days
  - After merge, verify: `winget install arndawg.tmux-windows`

## Winget v3.6a-win32 PR

- PR: https://github.com/microsoft/winget-pkgs/pull/341769
- Includes security fixes (auth race, DACL, constant-time compare) + version bump
- Waiting for Microsoft review

## Winget v3.6a-win32.1 PR

- PR: https://github.com/microsoft/winget-pkgs/pull/341850
- Includes server exit fix
- Waiting for Microsoft review

## Winget v3.6a-win32.2

- [ ] Submit PR to `microsoft/winget-pkgs` with updated hash + version
- [ ] Waiting for Microsoft review

## Server Exit Fix (2026-02-23)

- [x] Root cause: Windows always uses `-D` (no fork), which sets `CLIENT_NOFORK`,
  which disables `exit-empty` — server never exits after last session destroyed
- [x] Fix: `#ifndef _WIN32` guard around exit-empty=0 in `server.c:308`
- [x] Both regression suites pass
- [x] Manual test confirms server exits cleanly after `kill-session`
- [x] Committed and pushed (54505353)

## Upgrade Process

When `winget upgrade` replaces `tmux.exe`, any resident server process will hold a
file lock on the old binary. Windows cannot overwrite a running executable.

**Workaround**: Run `tmux kill-server` before upgrading. This is the same pattern
used by other server-mode CLI tools on Windows.

With the server exit fix above, the server now auto-exits when all sessions are
destroyed, so the "stuck resident server" scenario should be much less common.

## Target User Experience

```powershell
# After winget merge:
winget install arndawg.tmux-windows

# Current (Stage 2):
# Download tmux-windows-v3.5a-win32.zip from GitHub Releases
# Extract to a folder in %PATH%
# Run: tmux new-session
```

---

## Named Pipe IPC Overhaul (2026-02-23) — DONE

Replaced the TCP auth token file system with Windows Named Pipe discovery.
Released as v3.6a-win32.2.

### Architecture

Server creates `\\.\pipe\tmux-<user>-<label>` with kernel-enforced DACL (current user
only). Clients connect to the pipe, receive a one-time nonce + TCP port, then
authenticate over TCP with the nonce. Data path stays on TCP for libevent compatibility.

### What Changed

| File | Changes |
|------|---------|
| `win32-ipc.c` | Major rewrite: +379/-352 lines. Deleted auth token/port file system, added pipe discovery + nonce auth |
| `client.c` | Removed exponential backoff retry loop, replaced with simple post-launch retry |
| `server.c` | Updated cleanup comment (pipe close instead of file delete) |
| `win32-platform.h` | Removed `win32_ipc_create_auth_token` declaration |

### What Was Eliminated

- `.auth` and `.port` files in `%LOCALAPPDATA%\tmux\`
- `create_user_only_file()`, `write_user_only_file()`, `timingsafe_strcmp()`
- `label_to_port()`, `get_port_file_path()`, `get_auth_token_path()`
- `win32_ipc_create_auth_token()`, `read_auth_token()`
- Non-blocking connect + select() timeout logic
- Exponential backoff retry loop

### Cold Start Performance

Also resolved the 2.3s restart stall from v3.6a-win32. The stall was caused by
TCP TIME_WAIT on the stale port file. Named pipes eliminate port files entirely —
`WaitNamedPipe` replaces the retry loop and connects as soon as the server's pipe
is ready. Restart time: ~250ms.

### Remaining Optimization Opportunities

- [ ] **Reduce TTY channel overhead** — the second pipe handshake + TCP connection for
  TTY I/O (`win32_ipc_connect_tty`) adds ~50-100ms during interactive attach. Could
  potentially be pipelined with the identify handshake.

---

## Regression Test Expansion (2026-02-23)

CI currently runs only 2 win32 tests (18 assertions total). The upstream `tmux/tmux`
repo has 33 test scripts with hundreds of assertions that are NOT run on Windows.
Reviewing upstream commits from Dec 2024 – Feb 2025 shows 15 bug fixes (4 crash
fixes, 4 key-handling fixes, 3 parser fixes) — none of which our test suite would
catch. Goal: port the highest-value upstream tests and add Windows-specific coverage.

### 1. Port `format-strings.sh` (171 assertions) — DONE

- [x] Port upstream `format-strings.sh` to Windows
- 171 assertions pass (4 `#()` command substitution tests skipped — runs via cmd.exe)
- Adaptation: `tr -d '\r'` in `test_format()`, `-fNUL`, skip `#()` tests
- File: `regress/win32-format-strings.sh`

### 2. Port `conf-syntax.sh` (21 config files) — DONE

- [x] Port upstream `conf-syntax.sh` to Windows
- Validates all 21 `regress/conf/*.conf` files parse without error
- Uses `-f "$i"` at session creation (upstream `source -n` hangs on Windows)
- Bug found & fixed: `file.c:file_get_path()` and `cmd-source-file.c` didn't
  recognize MSYS2-escaped drive paths (`C\:/foo`) as absolute
- File: `regress/win32-conf-syntax.sh`

### 3. Write `win32-keys.sh` (12 key tests) — DONE

- [x] Write Windows-specific key handling test
- 12 tests: Enter, literals, Space, Tab, Escape, Ctrl-C interrupt, arrow keys
  in copy mode, Home/End, PgUp/PgDn, F-keys, Backspace, modifier combos
- Tests key EFFECTS not raw codes (`cat -tv`/`stty` unavailable on ConPTY)
- File: `regress/win32-keys.sh`

### 4. Port `has-session-return.sh` (exit code validation) — DONE

- [x] Port upstream `has-session-return.sh` to Windows
- Tests: no server → fail, server + no session → fail, named session → success
- File: `regress/win32-has-session.sh`

### 5. Write layout verification test (7 tests) — DONE

- [x] Write test that verifies pane dimensions after split operations
- 7 tests: single pane dims, h-split, v-split, 4-pane, tiled, even-horizontal,
  even-vertical — verifies widths/heights sum correctly and are evenly distributed
- Note: detached sessions on Windows use full height (no status line subtracted)
- File: `regress/win32-layout.sh`

### 6. Port `control-client-sanity.sh` (6 pane operation tests) — DONE

- [x] Port upstream control-client operations to Windows
- Control mode (`-C`) not supported on Windows (libevent signal init fails)
- Tests equivalent pane operations via normal commands: splitw, selectp, killp,
  swapp, selectl tiled, killw, verify server survives
- Bug found & fixed: `TMP` variable shadowed Windows `%TMP%`, causing
  `GetTempFileName()` ACCESS_DENIED — renamed to `OUT`
- File: `regress/win32-control-client.sh`

### CI Integration — DONE

- [x] Add all 6 tests to `.github/workflows/ci.yml`
- Each test step runs `taskkill //F //IM tmux.exe` cleanup before execution
- Key handling test has `timeout-minutes: 5` (many sleep waits)

---

## Security Audit (2026-02-22)

Full security review of the Windows IPC/auth layer. 13 findings across 4 severity levels.

**Update (2026-02-23):** The named pipe IPC overhaul (v3.6a-win32.2) resolved 7 of
13 findings by eliminating the auth token file system entirely. Findings #1-6, #8,
#12, #13 are no longer applicable — there are no auth files, no port files, no
secret comparison, and no predictable TCP port discovery.

### Critical

| # | Finding | Status |
|---|---------|--------|
| 1 | Auth token file has no Windows ACL | ~~Fixed~~ → Eliminated (no auth files) |
| 2 | Port file also unprotected | ~~Fixed~~ → Eliminated (no port files) |
| 3 | `getpeereid()` stub returns uid=0 always | Deferred (structural) |

### High

| # | Finding | Status |
|---|---------|--------|
| 4 | Non-constant-time auth comparison | ~~Fixed~~ → Eliminated (no secret comparison, nonces are ephemeral) |
| 5 | Race: server listens before auth token exists | ~~Fixed~~ → Eliminated (pipe created before TCP listen) |
| 6 | TCP loopback not user-isolated; port predictable | Resolved (pipe ACL enforces user isolation, port only disclosed via pipe) |
| 7 | Socketpair TOCTOU race allows hijacking signal/PTY pipes | Deferred |

### Medium

| # | Finding | Status |
|---|---------|--------|
| 8 | Auth token not scrubbed from memory | Eliminated (no persistent auth tokens) |
| 9 | Unbounded pending TTY queue — DoS vector | Deferred |
| 10 | Potential command injection in label embedding | Deferred |
| 11 | ConPTY bridge sockets inherit socketpair race | Deferred |

### Low

| # | Finding | Status |
|---|---------|--------|
| 12 | Blocking auth read can stall single-threaded server | Eliminated (nonce lookup is in-memory) |
| 13 | No token rotation mechanism | Eliminated (nonces are single-use, expire in 30s) |

### Remaining Open Issues

- **#3** `getpeereid()` stub — structural, affects Unix compat layer
- **#7** Socketpair TOCTOU — signal/PTY pipe creation race in `win32-compat.c`
- **#9** Unbounded pending TTY queue — nonce list is now capped at 16 entries, but
  TTY queue in `server.c` is still unbounded
- **#10** Label command injection — `win32-process.c` label embedding
- **#11** ConPTY bridge socketpair race — inherits #7
