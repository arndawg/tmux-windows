# tmux Windows Port

This document describes the Windows port of tmux, which enables tmux to build and
run on Windows 10+ using MSVC and the Windows ConPTY API.

## Overview

The port preserves the existing Unix codebase almost entirely intact, adding a
Windows platform abstraction layer in the `win32/` directory (10 new files,
~3,600 lines) and making targeted `#ifdef _WIN32` modifications across 62
existing source files (~1,170 insertions, ~60 deletions).

### Verified Commands

The following tmux commands have been tested and work on Windows:

```
tmux new-session -d -s test
tmux ls
tmux send-keys -t test "echo hello" Enter
tmux capture-pane -t test -p
tmux split-window -t test
tmux list-panes -t test
tmux kill-session -t test
```

Both manual server start (`tmux -D`) and automatic server launch (client spawns
a detached server process) are supported.

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
   pipe, emulating `waitpid()` behavior.

6. **File I/O: Direct CRT writes for stdout/stderr.**
   libevent's `select()` on Windows only works with Winsock sockets. The port
   bypasses `bufferevent` for stdout/stderr file descriptors and writes directly
   via `_write()`, preventing the event loop from freezing.

## New Files (win32/)

| File | Lines | Purpose |
|------|------:|---------|
| `win32-platform.h` | 1,081 | POSIX type mappings, constants, signal stubs, struct termios/winsize, function declarations |
| `win32-compat.c` | 730 | POSIX shims: flock, clock_gettime, socketpair, waitpid, kill, wcwidth, fnmatch, tparm, etc. |
| `win32-regex.c` | 429 | Simple POSIX ERE regex engine (character classes, quantifiers, anchors, alternation) |
| `win32-regex.h` | 51 | POSIX regex interface header |
| `win32-ipc.c` | 323 | TCP-based IPC replacing Unix domain sockets, with auth token management |
| `win32-pty.c` | 286 | ConPTY wrapper: pseudo-console creation, process spawn, bidirectional bridge threads |
| `win32-process.c` | 271 | Process watcher thread, `win32_launch_server()`, child spawn helpers |
| `win32-terminfo.c` | 213 | Hardcoded xterm-256color terminal capability table (170+ entries) |
| `win32-signal.c` | 157 | Signal emulation via console events, resize polling, self-pipe delivery |
| `win32-tty.c` | 98 | Console mode management: raw VT mode, size queries, UTF-8 code page |

## Modified Files

### Core Changes (significant new logic)

| File | +Lines | Changes |
|------|-------:|---------|
| `spawn.c` | +120 | ConPTY spawn path; `environ_to_win32_block()` for Unicode env blocks; cmd.exe invocation |
| `compat/imsg-buffer.c` | +114 | `WSAGetLastError()` for Winsock non-blocking errors; Windows sendmsg/recvmsg |
| `tmux.c` | +104 | Shell detection (COMSPEC), label paths, `setblocking()` via ioctlsocket, `find_home()` via USERPROFILE |
| `job.c` | +101 | ConPTY for PTY jobs; `win32_process_spawn()` for non-PTY jobs; no fork/setsid |
| `proc.c` | +95 | Signal pipe integration; `proc_fork_and_daemon()` via CreateProcess; closesocket |
| `client.c` | +89 | TCP IPC connection with poll-retry; `win32_launch_server()` fallback; fd passing disabled |
| `input.c` | +86 | Conditional includes; input handling adjustments |
| `server.c` | +61 | `win32_ipc_create_server()`; label-based identification; auth token verification |
| `cmd-pipe-pane.c` | +50 | Windows pipe handling |
| `server-client.c` | +48 | IPC client handling for Windows |
| `tty-term.c` | +43 | `win32_terminfo_*()` lookups instead of terminfo database |
| `tty.c` | +41 | `win32_tty_raw_mode()` / `win32_tty_restore()`; console TTY support |
| `file.c` | +37 | Skip bufferevent for stdout/stderr/stdin; direct `_write()` for output |
| `window.c` | +27 | Window management adjustments |
| `cmd-source-file.c` | +19 | Path handling for Windows |

### Minor Changes (conditional includes and guards)

The remaining ~45 files have 2-12 line changes each, primarily:
- `#ifndef _WIN32` guards around Unix-specific includes (`<signal.h>`, `<unistd.h>`, `<sys/socket.h>`, etc.)
- Minor compatibility adjustments (path separators, type casts)
- Safe fallbacks for missing POSIX functions

Affected files: `arguments.c`, `cmd-attach-session.c`, `cmd-find.c`, `cmd-if-shell.c`,
`cmd-join-pane.c`, `cmd-kill-server.c`, `cmd-list-windows.c`, `cmd-load-buffer.c`,
`cmd-new-session.c`, `cmd-new-window.c`, `cmd-parse.y`, `cmd-queue.c`, `cmd-run-shell.c`,
`cmd-save-buffer.c`, `cmd-server-access.c`, `cmd-show-messages.c`, `cmd-split-window.c`,
`cmd.c`, `compat.h`, `compat/base64.c`, `compat/getdtablecount.c`, `compat/getdtablesize.c`,
`compat/getline.c`, `compat/htonll.c`, `compat/imsg.c`, `compat/ntohll.c`,
`compat/recallocarray.c`, `control.c`, `environ.c`, `format.c`, `log.c`, `mode-tree.c`,
`names.c`, `options.c`, `popup.c`, `regsub.c`, `screen.c`, `server-acl.c`, `server-fn.c`,
`session.c`, `status.c`, `tmux-protocol.h`, `tmux.h`, `tty-keys.c`, `window-buffer.c`,
`window-client.c`, `window-copy.c`

## Build System

The project uses CMake with MSVC:

```
CMakeLists.txt    # CMake build configuration
```

Build with:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
MSBuild.exe build/tmux.sln -p:Configuration=Release -m
```

The resulting `tmux.exe` is in `build/Release/`.

## Runtime Requirements

- Windows 10 version 1809+ (for ConPTY support)
- Windows Terminal or a VT-capable console host
- No external dependencies (ncurses, libevent, and imsg are compiled in)

## Known Limitations

- No Unix domain socket support; IPC uses TCP loopback with auth tokens
- No file descriptor passing between processes; proxy model used instead
- Regex engine is simplified (covers tmux's pattern matching needs)
- `SIGCHLD` is emulated via process watcher thread; timing may differ from Unix
- Some terminal features may behave differently under ConPTY vs. native Unix PTYs
