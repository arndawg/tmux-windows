cmake_minimum_required(VERSION 3.16)
project(tmux C)

# Require Windows 10 1809+ for ConPTY
set(CMAKE_SYSTEM_VERSION 10.0.17763.0)

# Use MSVC runtime
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Options
option(TMUX_ENABLE_UTF8PROC "Enable utf8proc support" OFF)

# Find libevent
find_package(libevent CONFIG QUIET)
if(NOT libevent_FOUND)
    # Try pkg-config as fallback
    find_package(PkgConfig)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(LIBEVENT REQUIRED libevent)
    else()
        # Manual search
        find_path(LIBEVENT_INCLUDE_DIRS event2/event.h)
        find_library(LIBEVENT_LIBRARIES NAMES event libevent)
        if(NOT LIBEVENT_INCLUDE_DIRS OR NOT LIBEVENT_LIBRARIES)
            message(FATAL_ERROR "libevent not found. Install via vcpkg: vcpkg install libevent")
        endif()
    endif()
endif()

# cmd-parse.c generation from cmd-parse.y
# Try to find bison (WinFlexBison on Windows)
find_program(BISON_EXECUTABLE NAMES win_bison bison)
if(BISON_EXECUTABLE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cmd-parse.c
        COMMAND ${BISON_EXECUTABLE} -o ${CMAKE_CURRENT_SOURCE_DIR}/cmd-parse.c
                ${CMAKE_CURRENT_SOURCE_DIR}/cmd-parse.y
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cmd-parse.y
        COMMENT "Generating cmd-parse.c from cmd-parse.y"
    )
else()
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmd-parse.c)
        message(FATAL_ERROR
            "bison/win_bison not found and cmd-parse.c does not exist.\n"
            "Install WinFlexBison via vcpkg: vcpkg install winflex")
    else()
        message(STATUS "Using pre-generated cmd-parse.c")
    endif()
endif()

# Core tmux source files
set(TMUX_SOURCES
    alerts.c
    arguments.c
    attributes.c
    cfg.c
    client.c
    cmd.c
    cmd-attach-session.c
    cmd-bind-key.c
    cmd-break-pane.c
    cmd-capture-pane.c
    cmd-choose-tree.c
    cmd-command-prompt.c
    cmd-confirm-before.c
    cmd-copy-mode.c
    cmd-detach-client.c
    cmd-display-menu.c
    cmd-display-message.c
    cmd-display-panes.c
    cmd-find.c
    cmd-find-window.c
    cmd-if-shell.c
    cmd-join-pane.c
    cmd-kill-pane.c
    cmd-kill-server.c
    cmd-kill-session.c
    cmd-kill-window.c
    cmd-list-buffers.c
    cmd-list-clients.c
    cmd-list-keys.c
    cmd-list-panes.c
    cmd-list-sessions.c
    cmd-list-windows.c
    cmd-load-buffer.c
    cmd-lock-server.c
    cmd-move-window.c
    cmd-new-session.c
    cmd-new-window.c
    cmd-parse.c
    cmd-paste-buffer.c
    cmd-pipe-pane.c
    cmd-queue.c
    cmd-refresh-client.c
    cmd-rename-session.c
    cmd-rename-window.c
    cmd-resize-pane.c
    cmd-resize-window.c
    cmd-respawn-pane.c
    cmd-respawn-window.c
    cmd-rotate-window.c
    cmd-run-shell.c
    cmd-save-buffer.c
    cmd-select-layout.c
    cmd-select-pane.c
    cmd-select-window.c
    cmd-send-keys.c
    cmd-server-access.c
    cmd-set-buffer.c
    cmd-set-environment.c
    cmd-set-option.c
    cmd-show-environment.c
    cmd-show-messages.c
    cmd-show-options.c
    cmd-show-prompt-history.c
    cmd-source-file.c
    cmd-split-window.c
    cmd-swap-pane.c
    cmd-swap-window.c
    cmd-switch-client.c
    cmd-unbind-key.c
    cmd-wait-for.c
    colour.c
    control.c
    control-notify.c
    environ.c
    file.c
    format.c
    format-draw.c
    grid.c
    grid-reader.c
    grid-view.c
    hyperlinks.c
    input.c
    input-keys.c
    job.c
    key-bindings.c
    key-string.c
    layout.c
    layout-custom.c
    layout-set.c
    log.c
    menu.c
    mode-tree.c
    names.c
    notify.c
    options.c
    options-table.c
    paste.c
    popup.c
    proc.c
    regsub.c
    resize.c
    screen.c
    screen-redraw.c
    screen-write.c
    server.c
    server-acl.c
    server-client.c
    server-fn.c
    session.c
    sort.c
    spawn.c
    status.c
    style.c
    tmux.c
    tty.c
    tty-acs.c
    tty-draw.c
    tty-features.c
    tty-keys.c
    tty-term.c
    utf8.c
    utf8-combined.c
    window.c
    window-buffer.c
    window-client.c
    window-clock.c
    window-copy.c
    window-customize.c
    window-tree.c
    xmalloc.c
)

# Windows-specific osdep
set(TMUX_OSDEP_SOURCES
    osdep-windows.c
)

# compat library sources (subset needed for Windows)
# Note: closefrom, daemon, setenv, getprogname, getpeereid, clock_gettime
# are provided by win32/win32-compat.c; forkpty/fdforkpty are replaced by ConPTY
set(TMUX_COMPAT_SOURCES
    compat/imsg.c
    compat/imsg-buffer.c
    compat/vis.c
    compat/unvis.c
    compat/strlcpy.c
    compat/strlcat.c
    compat/strtonum.c
    compat/asprintf.c
    compat/reallocarray.c
    compat/recallocarray.c
    compat/explicit_bzero.c
    compat/freezero.c
    compat/getdtablecount.c
    compat/getdtablesize.c
    # htonll.c/ntohll.c not needed - winsock2.h provides these
    compat/base64.c
    compat/strndup.c
    # strnlen: MSVC CRT provides strnlen; we define HAVE_STRNLEN in platform.h
    compat/memmem.c
    compat/getline.c
    compat/fgetln.c
    compat/strcasestr.c
    compat/getopt_long.c
    compat/err.c
)

# win32 layer sources
set(TMUX_WIN32_SOURCES
    win32/win32-compat.c
    win32/win32-ipc.c
    win32/win32-process.c
    win32/win32-pty.c
    win32/win32-signal.c
    win32/win32-terminfo.c
    win32/win32-tty.c
    win32/win32-regex.c
)

# Optional: utf8proc
if(TMUX_ENABLE_UTF8PROC)
    find_package(utf8proc REQUIRED)
    list(APPEND TMUX_COMPAT_SOURCES compat/utf8proc.c)
endif()

# Build tmux executable
add_executable(tmux
    ${TMUX_SOURCES}
    ${TMUX_OSDEP_SOURCES}
    ${TMUX_COMPAT_SOURCES}
    ${TMUX_WIN32_SOURCES}
)

# Compiler definitions
target_compile_definitions(tmux PRIVATE
    _WIN32
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_DEPRECATE
    PACKAGE_NAME="tmux"
    PACKAGE_VERSION="3.6a-win32"
    TMUX_CONF="${CMAKE_INSTALL_PREFIX}/etc/tmux.conf"
    # Functions provided by compat/ source files we compile:
    # Do NOT define HAVE_X for these - compat.h needs to declare them
    # and the compat .c files provide implementations.
    # The HAVE_IMSG flag tells tmux to use the compat imsg.
    HAVE_IMSG
    HAVE_EVENT2_EVENT_H
    HAVE_STDINT_H
)

# Include directories
target_include_directories(tmux PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/compat
    ${CMAKE_CURRENT_SOURCE_DIR}/win32
)

# libevent include/link
if(libevent_FOUND)
    target_link_libraries(tmux PRIVATE libevent::core libevent::extra)
elseif(LIBEVENT_INCLUDE_DIRS)
    target_include_directories(tmux PRIVATE ${LIBEVENT_INCLUDE_DIRS})
    target_link_libraries(tmux PRIVATE ${LIBEVENT_LIBRARIES})
endif()

# Windows system libraries
target_link_libraries(tmux PRIVATE
    ws2_32
    advapi32
    kernel32
    user32
    shell32
    ole32
    ntdll
)

# utf8proc
if(TMUX_ENABLE_UTF8PROC AND utf8proc_FOUND)
    target_link_libraries(tmux PRIVATE utf8proc)
    target_compile_definitions(tmux PRIVATE ENABLE_UTF8PROC)
endif()

# MSVC-specific settings
if(MSVC)
    target_compile_options(tmux PRIVATE
        /W3
        /wd4005  # macro redefinition
        /wd4018  # signed/unsigned mismatch
        /wd4028  # parameter differs
        /wd4047  # indirection levels differ
        /wd4090  # const qualifier
        /wd4133  # incompatible types
        /wd4146  # unary minus on unsigned
        /wd4244  # conversion, possible loss of data
        /wd4267  # size_t to int
        /wd4456  # local declaration hides
        /wd4457  # parameter hides
        /wd4701  # potentially uninitialized
        /wd4702  # unreachable code
        /wd4996  # deprecated POSIX names
    )
    # Use C11
    set_property(TARGET tmux PROPERTY C_STANDARD 11)
endif()

# Install
install(TARGETS tmux RUNTIME DESTINATION bin)

# Print summary
message(STATUS "")
message(STATUS "tmux Windows build configuration:")
message(STATUS "  Version:    3.6a-win32")
message(STATUS "  UTF8PROC:   ${TMUX_ENABLE_UTF8PROC}")
message(STATUS "")
