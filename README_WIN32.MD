# tmux Windows Port

This document describes the Windows port of tmux, which enables tmux to build and
run on Windows 10+ using MSVC and the Windows ConPTY API.

## Overview

The port preserves the existing Unix codebase almost entirely intact, adding a
Windows platform abstraction layer in the `win32/` directory (10 new files,
~3,800 lines) and making targeted `#ifdef _WIN32` modifications across existing
source files.

### Verified Commands

The following tmux commands have been tested and work on Windows:

```
tmux new-session -d -s test
tmux ls
tmux send-keys -t test "echo hello" Enter
tmux capture-pane -t test -p
tmux split-window -t test
tmux list-panes -t test
tmux attach -t test
tmux kill-session -t test
```

Both manual server start (`tmux -D`) and automatic server launch (client spawns
a detached server process) are supported. Terminal attach with full-window
rendering and dynamic resize works correctly.

## Architecture

### Key Design Decisions

1. **IPC: TCP loopback instead of Unix domain sockets.**
   Windows lacks `AF_UNIX` support in older builds. The port uses localhost TCP
   connections with deterministic port derivation from the session label and
   cryptographic auth tokens stored in `%LOCALAPPDATA%\tmux\`.

2. **PTY: ConPTY (Windows Pseudo Console) instead of /dev/pty.**
   Child processes are spawned with `CreatePseudoConsole()`. Two bridge threads
   shuttle I/O between the ConPTY pipe handles and a Winsock socket pair so that
   libevent can drive all I/O through `select()`.

3. **Signals: Console events + polling + self-pipe pattern.**
   A console control handler catches Ctrl+C (SIGINT) and window close (SIGTERM).
   A polling thread detects console resize (SIGWINCH). All signals are delivered
   to the libevent loop via a TCP socketpair self-pipe.

4. **Terminal capabilities: Hardcoded xterm-256color database.**
   No ncurses dependency on Windows. 170+ terminfo entries are compiled into
   `win32-terminfo.c` with VT escape sequences that Windows Terminal and modern
   consoles support natively.

5. **Process management: Watcher thread instead of SIGCHLD.**
   A background thread monitors child process handles via
   `WaitForMultipleObjects` and delivers exit notifications through the signal
   pipe, emulating `waitpid()` behavior. Exit codes are encoded in Unix
   wait-status format (`code << 8`) so `WIFEXITED`/`WEXITSTATUS` macros work
   unchanged.

6. **File I/O: Direct CRT writes for stdout/stderr.**
   libevent's `select()` on Windows only works with Winsock sockets. The port
   bypasses `bufferevent` for stdout/stderr file descriptors and writes directly
   via `_write()`, preventing the event loop from freezing.

7. **TTY channel separation for attach.**
   On Unix, the client's terminal fd is passed to the server via `SCM_RIGHTS`.
   On Windows, `c->fd` is a TCP socket — it cannot carry terminal semantics.
   Instead, the client opens a second TCP connection authenticated with a
   one-time tty token (`MSG_IDENTIFY_TTYTOKEN`). The server correlates this
   connection to the client and uses it exclusively for escape sequence I/O,
   with `send()` instead of `write()` to avoid CRT invalid parameter aborts
   on Winsock sockets.

8. **Client-driven terminal sizing.**
   The server runs as a `DETACHED_PROCESS` with no console, so
   `GetConsoleScreenBufferInfo()` fails. The client sends its console dimensions
   via `MSG_RESIZE` with a `struct msg_resize_data` payload — both at startup
   (after `MSG_IDENTIFY_DONE`) and on each `SIGWINCH`. The server applies these
   via `tty_set_size()` instead of calling `tty_resize()` (which returns
   immediately on Windows).

## CLI Compatibility

The Windows port accepts the same command-line flags as Unix tmux. The following
cross-platform differences are handled automatically:

| Flag / Feature | Unix | Windows |
|---------------|------|---------|
| `-f /dev/null` | Opens `/dev/null` | Translated to `NUL` automatically |
| `-S /path/to/sock` | Unix domain socket file | Basename extracted as IPC label (warning emitted) |
| `-L name` | Socket label in `/tmp/tmux-$UID/` | IPC label in `%LOCALAPPDATA%\tmux\` |
| `default-shell` | Must start with `/` | Unix paths (starting with `/`) rejected with fallback to `cmd.exe` |
| `$SHELL` | Required to start with `/` | Falls back to `$COMSPEC` then `cmd.exe` |
| `$TMUX_TMPDIR` | Controls socket directory | Not used (IPC directory is `%LOCALAPPDATA%\tmux\`) |
| `~/.tmux.conf` | `~` = `$HOME` or pw_dir | `~` = `$USERPROFILE` or `$HOME` or `C:\` |
| `-u` (UTF-8) | Opt-in | Auto-enabled on Windows |

## New Files (win32/)

| File | Lines | Purpose |
|------|------:|---------|
| `win32-platform.h` | 1,081 | POSIX type mappings, constants, signal stubs, struct termios/winsize, function declarations |
| `win32-compat.c` | 730 | POSIX shims: flock, clock_gettime, socketpair, waitpid, kill, wcwidth, fnmatch, tparm, etc. |
| `win32-ipc.c` | 462 | TCP-based IPC replacing Unix domain sockets, with auth token management and tty channel support |
| `win32-regex.c` | 429 | Simple POSIX ERE regex engine (character classes, quantifiers, anchors, alternation) |
| `win32-regex.h` | 51 | POSIX regex interface header |
| `win32-pty.c` | 286 | ConPTY wrapper: pseudo-console creation, process spawn, bidirectional bridge threads |
| `win32-process.c` | 271 | Process watcher thread, `win32_launch_server()`, child spawn helpers |
| `win32-terminfo.c` | 213 | Hardcoded xterm-256color terminal capability table (170+ entries) |
| `win32-signal.c` | 157 | Signal emulation via console events, resize polling, self-pipe delivery |
| `win32-tty.c` | 98 | Console mode management: raw VT mode, size queries, UTF-8 code page |

## Modified Files

### Core Changes (significant new logic)

| File | Changes |
|------|---------|
| `spawn.c` | ConPTY spawn path; `environ_to_win32_block()` for Unicode env blocks; cmd.exe invocation |
| `compat/imsg-buffer.c` | `WSAGetLastError()` for Winsock non-blocking errors; Windows sendmsg/recvmsg |
| `tmux.c` | Shell detection (COMSPEC), label paths, `/dev/null` → `NUL` for `-f`, `-S` path sanitization, `checkshell()` Unix path guard |
| `job.c` | ConPTY for PTY jobs; `win32_process_spawn()` for non-PTY jobs; no fork/setsid |
| `proc.c` | Signal pipe integration; `proc_fork_and_daemon()` via CreateProcess; closesocket |
| `client.c` | TCP IPC connection with poll-retry; `win32_launch_server()` fallback; tty channel with token auth; client-driven resize via `MSG_RESIZE` |
| `input.c` | Conditional includes; input handling adjustments |
| `server.c` | `win32_ipc_create_server()`; label-based identification; auth token verification; tty channel correlation |
| `server-client.c` | IPC client handling; `MSG_RESIZE` with `msg_resize_data` payload; `MSG_IDENTIFY_TTYTOKEN` |
| `tty.c` | `send()` instead of `write()` for tty_raw on Winsock sockets; `CLIENT_DEAD` guard; `tty_resize()` no-op on Windows |
| `server-fn.c` | ConPTY cleanup via `win32_pty_close()` in `server_destroy_pane()` |
| `tmux-protocol.h` | `struct msg_resize_data`, `MSG_IDENTIFY_TTYTOKEN`, Windows-specific message types |
| `cmd-pipe-pane.c` | Windows pipe handling |
| `tty-term.c` | `win32_terminfo_*()` lookups instead of terminfo database |
| `file.c` | Skip bufferevent for stdout/stderr/stdin; direct `_write()` for output |
| `window.c` | ConPTY cleanup on pane destruction |
| `cmd-source-file.c` | Path handling for Windows |

### Minor Changes (conditional includes and guards)

The remaining ~45 files have 2-12 line changes each, primarily:
- `#ifndef _WIN32` guards around Unix-specific includes (`<signal.h>`, `<unistd.h>`, `<sys/socket.h>`, etc.)
- Minor compatibility adjustments (path separators, type casts)
- Safe fallbacks for missing POSIX functions

## Regression Tests

Windows-specific tests are in `regress/`. Run with:

```bash
TEST_TMUX=./build/Debug/tmux.exe bash regress/win32-basic.sh
TEST_TMUX=./build/Debug/tmux.exe bash regress/win32-claude-swarm.sh
```

### win32-basic.sh

1. Session lifecycle (new, ls, kill-session)
2. Send-keys + capture-pane (ConPTY I/O)
3. Split-window + list-panes (multiple ConPTY instances)
4. Pane exit cleanup (`server_destroy_pane()` ConPTY path)
5. Detached session size with explicit dimensions
6. `-f /dev/null` → `NUL` translation
7. `-S` with Unix-style path (basename extraction + warning)
8. `default-shell` fallback from Unix path to `cmd.exe`

### win32-claude-swarm.sh

Simulates the exact tmux command sequences used by Claude Code agent teams
(split-pane mode). Tests run across 3 full cycles:

1. Version gate (`tmux -V`)
2. `has-session` on missing session
3. Rapid multi-split (4 panes via `split-window`)
4. Parallel `send-keys` to all panes
5. `capture-pane` round-trip verification
6. Pane targeting (`send-keys -t session:0.N`)
7. `list-panes` format output
8. Rapid session teardown (`kill-session`)
9. Multi-session management
10. Rapid create-destroy cycles (5 iterations)

Note: The tests use `OUT` instead of `TMP` for temp file variables to avoid
shadowing the Windows `%TMP%` environment variable (which would cause
`GetTempFileName()` to fail with ACCESS_DENIED in the server process).

## Installation

### Via winget

```powershell
winget install arndawg.tmux-windows
```

### Manual

1. Download the latest `tmux-windows-*.zip` from
   [GitHub Releases](https://github.com/arndawg/tmux-windows/releases)
2. Extract `tmux.exe` and `event_core.dll` to a folder in your `%PATH%`
3. Run `tmux new-session`

### Upgrading

Run `tmux kill-server` before upgrading. Windows cannot overwrite a running
executable, and `winget upgrade` will fail if the server is resident.

With the exit-empty fix, the server auto-exits when all sessions are destroyed,
so the server is only resident while sessions are active.

## Build System

The project uses CMake with MSVC. Requires [vcpkg](https://vcpkg.io/) for
libevent:

```powershell
vcpkg install libevent:x64-windows
cmake -S . -B build -G "Visual Studio 17 2022" `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_INSTALLATION_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --config Release
```

The resulting `tmux.exe` is in `build/Release/`. For debug builds, use
`--config Debug` (output in `build/Debug/`). Copy `tmux.exe` and
`event_core.dll` to a directory in `%PATH%`.

CI builds run automatically on every push via GitHub Actions
(`.github/workflows/ci.yml`). Tagged releases (`v*`) produce downloadable
zip artifacts.

## Debugging

Use the `-v` flag for verbose logging. Each `-v` increases the log level:

```bash
tmux -vvv new-session
```

This creates timestamped log files in the current directory:
- `tmux-client-<pid>.log` — client-side timing (connect, identify, attach)
- `tmux-server-<pid>.log` — server-side timing (socket, auth, ConPTY, events)

Each line has microsecond-precision timestamps (`epoch.usec`) for profiling:

```
1771885428.291080 client started (43248): version 3.6a-win32, ...
1771885428.295389 connect failed, starting server
1771885428.512943 add peer ...: connected
```

## Runtime Requirements

- Windows 10 version 1809+ (for ConPTY support)
- Windows Terminal or a VT-capable console host
- `event_core.dll` (libevent, included in release zip)

## Security

The Windows port uses TCP loopback for IPC instead of Unix domain sockets.
The following hardening measures are in place:

- **Auth tokens**: Each server generates a 256-bit random token via
  `RtlGenRandom`, stored in `%LOCALAPPDATA%\tmux\<label>.auth`. Clients
  must present the token on connect.
- **File ACLs**: Auth token and port files are created with a DACL restricting
  access to the current user's SID only (no admin/SYSTEM/other user access).
- **Constant-time comparison**: Auth token verification uses a timing-safe
  XOR-accumulator comparison to prevent side-channel attacks.
- **Auth-before-listen**: The auth token file is created before the server
  begins accepting connections, closing the race window.

IPC files are stored in `%LOCALAPPDATA%\tmux\`:
- `<label>.port` — TCP port number
- `<label>.auth` — 64-character hex auth token

The server cleans up these files on exit. If stale files remain (e.g., after
a force-kill), the next client automatically cleans them up before launching
a new server.

## Known Limitations

- No Unix domain socket support; IPC uses TCP loopback with auth tokens
- No file descriptor passing between processes; tty channel model used instead
- Regex engine is simplified (covers tmux's pattern matching needs)
- `SIGCHLD` is emulated via process watcher thread; timing may differ from Unix
- Some terminal features may behave differently under ConPTY vs. native Unix PTYs
- `TMUX_TMPDIR` environment variable is not used; IPC files are always in `%LOCALAPPDATA%\tmux\`
- tmux output uses `\r\n` line endings; scripts should strip `\r` when comparing output
