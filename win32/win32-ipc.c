/*
 * IPC layer for Windows.
 *
 * Uses a named pipe for discovery/authentication and TCP for data.
 * The server creates \\.\pipe\tmux-<user>-<label> with a DACL restricting
 * access to the current user.  A background thread accepts pipe connections,
 * generates an ephemeral nonce, and writes "port:nonce\n" to the client.
 * The client then connects to the TCP port and sends the nonce.  This gives
 * kernel-level user isolation (pipe ACL) with libevent-compatible I/O (TCP).
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "win32-platform.h"
#include <sddl.h>

/*
 * RtlGenRandom is exported as SystemFunction036 from advapi32.dll.
 * It is the recommended way to generate cryptographic random bytes
 * without pulling in the full BCrypt API.
 */
#define RtlGenRandom SystemFunction036
BOOLEAN NTAPI RtlGenRandom(PVOID RandomBuffer, ULONG RandomBufferLength);

#define AUTH_TOKEN_LEN		32
#define MAX_PENDING_NONCES	16
#define NONCE_EXPIRY_MS		30000
#define PIPE_WAIT_TIMEOUT_MS	5000

struct pending_nonce {
	char	nonce[AUTH_TOKEN_LEN * 2 + 1];	/* 64 hex chars + NUL */
	DWORD	created;			/* GetTickCount() */
	int	used;
};

static struct pending_nonce	pending_nonces[MAX_PENDING_NONCES];
static CRITICAL_SECTION		nonce_lock;
static int			nonce_lock_init;

static HANDLE			discovery_pipe = INVALID_HANDLE_VALUE;
static HANDLE			pipe_thread_handle;
static HANDLE			pipe_shutdown_event;
static volatile int		pipe_thread_running;
static uint16_t			server_tcp_port;
static char			server_pipe_name[256];

/*
 * Get the named pipe path for a label.
 * The label (socket_path) is already "tmux-<username>-<label>" from
 * make_label() in tmux.c, so we just prepend the pipe namespace.
 * Returns pointer to a static buffer: \\.\pipe\tmux-<USERNAME>-<label>
 */
static const char *
get_pipe_name(const char *label)
{
	static char	name[256];

	snprintf(name, sizeof name, "\\\\.\\pipe\\%s", label);
	return (name);
}

/*
 * Create a named pipe with a DACL granting access only to the current user.
 * The pipe is created with FILE_FLAG_OVERLAPPED so the accept thread can
 * use overlapped ConnectNamedPipe with a shutdown event.
 * Returns pipe HANDLE or INVALID_HANDLE_VALUE on failure.
 */
static HANDLE
create_discovery_pipe(const char *pipe_name)
{
	HANDLE			h, token;
	DWORD			len;
	TOKEN_USER		*tu = NULL;
	char			*sid_str = NULL;
	char			sddl[256];
	SECURITY_ATTRIBUTES	sa;
	SECURITY_DESCRIPTOR	*sd = NULL;

	/* Get current user SID. */
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
		return (INVALID_HANDLE_VALUE);

	GetTokenInformation(token, TokenUser, NULL, 0, &len);
	tu = malloc(len);
	if (tu == NULL ||
	    !GetTokenInformation(token, TokenUser, tu, len, &len)) {
		free(tu);
		CloseHandle(token);
		return (INVALID_HANDLE_VALUE);
	}
	CloseHandle(token);

	if (!ConvertSidToStringSidA(tu->User.Sid, &sid_str)) {
		free(tu);
		return (INVALID_HANDLE_VALUE);
	}
	free(tu);

	/* Build SDDL: DACL grants only current user full access. */
	snprintf(sddl, sizeof sddl, "D:P(A;;GA;;;%s)", sid_str);
	LocalFree(sid_str);

	if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(
	    sddl, SDDL_REVISION_1, (PSECURITY_DESCRIPTOR *)&sd, NULL))
		return (INVALID_HANDLE_VALUE);

	memset(&sa, 0, sizeof sa);
	sa.nLength = sizeof sa;
	sa.lpSecurityDescriptor = sd;
	sa.bInheritHandle = FALSE;

	h = CreateNamedPipeA(pipe_name,
	    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
	    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
	    1,		/* max instances — prevents pipe name squatting */
	    256, 256,	/* output/input buffer sizes */
	    0, &sa);

	LocalFree(sd);
	return (h);
}

/*
 * Generate a nonce and add it to the pending list.
 * Writes the hex string to nonce_out.
 * Returns 0 on success, -1 if the table is full or RNG fails.
 */
static int
add_pending_nonce(char *nonce_out)
{
	unsigned char	random_bytes[AUTH_TOKEN_LEN];
	int		i;
	DWORD		now;

	if (!RtlGenRandom(random_bytes, sizeof random_bytes))
		return (-1);

	for (i = 0; i < AUTH_TOKEN_LEN; i++)
		snprintf(nonce_out + i * 2, 3, "%02x", random_bytes[i]);
	nonce_out[AUTH_TOKEN_LEN * 2] = '\0';

	EnterCriticalSection(&nonce_lock);
	now = GetTickCount();

	/* Expire stale entries. */
	for (i = 0; i < MAX_PENDING_NONCES; i++) {
		if (pending_nonces[i].used &&
		    (now - pending_nonces[i].created) > NONCE_EXPIRY_MS)
			pending_nonces[i].used = 0;
	}

	/* Find a free slot. */
	for (i = 0; i < MAX_PENDING_NONCES; i++) {
		if (!pending_nonces[i].used) {
			strlcpy(pending_nonces[i].nonce, nonce_out,
			    sizeof pending_nonces[i].nonce);
			pending_nonces[i].created = now;
			pending_nonces[i].used = 1;
			LeaveCriticalSection(&nonce_lock);
			return (0);
		}
	}

	LeaveCriticalSection(&nonce_lock);
	return (-1); /* Table full. */
}

/*
 * Look up and consume a nonce from the pending list.
 * Returns 0 if found and valid, -1 otherwise.
 */
static int
consume_pending_nonce(const char *nonce)
{
	int	i;
	DWORD	now;

	EnterCriticalSection(&nonce_lock);
	now = GetTickCount();

	for (i = 0; i < MAX_PENDING_NONCES; i++) {
		if (!pending_nonces[i].used)
			continue;
		if ((now - pending_nonces[i].created) > NONCE_EXPIRY_MS) {
			pending_nonces[i].used = 0;
			continue;
		}
		if (strcmp(pending_nonces[i].nonce, nonce) == 0) {
			pending_nonces[i].used = 0;
			LeaveCriticalSection(&nonce_lock);
			return (0);
		}
	}

	LeaveCriticalSection(&nonce_lock);
	return (-1);
}

/*
 * Pipe accept thread.
 * Waits for clients on the named pipe, generates a nonce, writes
 * "port:nonce\n" to the client, then disconnects and loops.
 * Uses overlapped ConnectNamedPipe so it can be woken by the shutdown event.
 */
static DWORD WINAPI
pipe_accept_thread(LPVOID arg)
{
	OVERLAPPED	ov;
	HANDLE		events[2];
	char		nonce[AUTH_TOKEN_LEN * 2 + 1];
	char		response[128];
	DWORD		wait_result, bytes;

	(void)arg;

	memset(&ov, 0, sizeof ov);
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL)
		return (1);

	events[0] = ov.hEvent;		/* ConnectNamedPipe completed */
	events[1] = pipe_shutdown_event;/* Shutdown requested */

	while (pipe_thread_running) {
		ResetEvent(ov.hEvent);

		if (ConnectNamedPipe(discovery_pipe, &ov)) {
			/* Client connected immediately. */
		} else {
			DWORD err = GetLastError();
			if (err == ERROR_IO_PENDING) {
				wait_result = WaitForMultipleObjects(2, events,
				    FALSE, INFINITE);
				if (wait_result != WAIT_OBJECT_0) {
					/* Shutdown or error. */
					CancelIo(discovery_pipe);
					GetOverlappedResult(discovery_pipe,
					    &ov, &bytes, TRUE);
					break;
				}
			} else if (err != ERROR_PIPE_CONNECTED) {
				break;
			}
		}

		if (!pipe_thread_running) {
			DisconnectNamedPipe(discovery_pipe);
			break;
		}

		/* Generate nonce and send port:nonce to client. */
		if (add_pending_nonce(nonce) == 0) {
			OVERLAPPED	ov_w;
			DWORD		written;

			snprintf(response, sizeof response, "%u:%s\n",
			    (unsigned)server_tcp_port, nonce);

			memset(&ov_w, 0, sizeof ov_w);
			ov_w.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			if (!WriteFile(discovery_pipe, response,
			    (DWORD)strlen(response), NULL, &ov_w)) {
				if (GetLastError() == ERROR_IO_PENDING)
					GetOverlappedResult(discovery_pipe,
					    &ov_w, &written, TRUE);
			}
			FlushFileBuffers(discovery_pipe);
			if (ov_w.hEvent != NULL)
				CloseHandle(ov_w.hEvent);
		}

		DisconnectNamedPipe(discovery_pipe);
	}

	CloseHandle(ov.hEvent);
	return (0);
}

/*
 * Discover server via named pipe and establish a TCP connection.
 * Fills nonce_out with the nonce received from the server.
 * Returns a connected TCP SOCKET or INVALID_SOCKET on failure.
 */
static SOCKET
pipe_discover_and_connect(const char *label, char *nonce_out, size_t nonce_size)
{
	const char		*pipe_name;
	HANDLE			pipe;
	char			buf[128];
	DWORD			nread;
	unsigned		port;
	SOCKET			s;
	struct sockaddr_in	addr;

	pipe_name = get_pipe_name(label);

	/*
	 * Open the pipe using the MSDN-recommended retry pattern.
	 * CreateFile may fail with ERROR_PIPE_BUSY if the single pipe
	 * instance is between DisconnectNamedPipe and the next
	 * ConnectNamedPipe in the accept thread.  WaitNamedPipe waits
	 * for the instance to become available again.  If no pipe
	 * exists at all (ERROR_FILE_NOT_FOUND), fail immediately —
	 * this is the normal "no server running" fast-fail path.
	 */
	for (;;) {
		pipe = CreateFileA(pipe_name, GENERIC_READ | GENERIC_WRITE,
		    0, NULL, OPEN_EXISTING, 0, NULL);
		if (pipe != INVALID_HANDLE_VALUE)
			break;
		if (GetLastError() != ERROR_PIPE_BUSY)
			return (INVALID_SOCKET);
		if (!WaitNamedPipeA(pipe_name, PIPE_WAIT_TIMEOUT_MS))
			return (INVALID_SOCKET);
	}

	/* Read "port:nonce\n" from pipe. */
	if (!ReadFile(pipe, buf, sizeof buf - 1, &nread, NULL) || nread == 0) {
		CloseHandle(pipe);
		return (INVALID_SOCKET);
	}
	CloseHandle(pipe);
	buf[nread] = '\0';

	/* Strip trailing newline and parse. */
	{
		char *nl = strchr(buf, '\n');
		if (nl != NULL)
			*nl = '\0';
	}
	{
		char *colon = strchr(buf, ':');
		if (colon == NULL)
			return (INVALID_SOCKET);
		*colon = '\0';
		port = (unsigned)atoi(buf);
		if (port == 0 || port > 65535)
			return (INVALID_SOCKET);
		strlcpy(nonce_out, colon + 1, nonce_size);
	}

	/* TCP connect to server. */
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return (INVALID_SOCKET);

	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons((uint16_t)port);

	if (connect(s, (struct sockaddr *)&addr, sizeof addr) != 0) {
		closesocket(s);
		return (INVALID_SOCKET);
	}

	return (s);
}

/*
 * Create a server: TCP listener on loopback + named pipe for discovery.
 * Starts the pipe accept thread. Returns the listening socket FD or -1.
 */
int
win32_ipc_create_server(const char *label, uint16_t *out_port)
{
	SOCKET			s;
	struct sockaddr_in	addr;
	int			addrlen = sizeof addr;
	const char		*pipe_name;

	win32_wsa_init();

	/* Initialize nonce tracking. */
	if (!nonce_lock_init) {
		InitializeCriticalSection(&nonce_lock);
		memset(pending_nonces, 0, sizeof pending_nonces);
		nonce_lock_init = 1;
	}

	/* Create TCP listener on loopback, OS-assigned port. */
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return (-1);

	{
		BOOL one = TRUE;
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&one,
		    sizeof one);
	}

	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = 0;

	if (bind(s, (struct sockaddr *)&addr, sizeof addr) != 0) {
		closesocket(s);
		return (-1);
	}

	if (getsockname(s, (struct sockaddr *)&addr, &addrlen) != 0) {
		closesocket(s);
		return (-1);
	}
	server_tcp_port = ntohs(addr.sin_port);

	if (listen(s, 128) != 0) {
		closesocket(s);
		return (-1);
	}

	/* Create named pipe for client discovery. */
	pipe_name = get_pipe_name(label);
	strlcpy(server_pipe_name, pipe_name, sizeof server_pipe_name);

	discovery_pipe = create_discovery_pipe(server_pipe_name);
	if (discovery_pipe == INVALID_HANDLE_VALUE) {
		closesocket(s);
		return (-1);
	}

	/* Start pipe accept thread. */
	pipe_shutdown_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	pipe_thread_running = 1;
	pipe_thread_handle = CreateThread(NULL, 0, pipe_accept_thread,
	    NULL, 0, NULL);
	if (pipe_thread_handle == NULL) {
		CloseHandle(discovery_pipe);
		discovery_pipe = INVALID_HANDLE_VALUE;
		if (pipe_shutdown_event != NULL) {
			CloseHandle(pipe_shutdown_event);
			pipe_shutdown_event = NULL;
		}
		closesocket(s);
		return (-1);
	}

	if (out_port != NULL)
		*out_port = server_tcp_port;

	return ((int)s);
}

/*
 * Connect to the server via named pipe discovery.
 * Returns socket FD or -1.
 */
int
win32_ipc_connect(const char *label)
{
	SOCKET	s;
	char	nonce[AUTH_TOKEN_LEN * 2 + 1];
	size_t	nlen;

	win32_wsa_init();

	s = pipe_discover_and_connect(label, nonce, sizeof nonce);
	if (s == INVALID_SOCKET) {
		errno = ECONNREFUSED;
		return (-1);
	}

	/* Send nonce + newline. */
	nlen = strlen(nonce);
	{
		char *line = malloc(nlen + 2);
		if (line != NULL) {
			memcpy(line, nonce, nlen);
			line[nlen] = '\n';
			send(s, line, (int)(nlen + 1), 0);
			free(line);
		}
	}

	return ((int)s);
}

/*
 * Generate a random token for tty channel correlation.
 * Writes 32 hex chars + NUL to buf (buf must be >= 65 bytes).
 */
void
win32_generate_tty_token(char *buf, size_t bufsize)
{
	unsigned char random_bytes[AUTH_TOKEN_LEN];
	int i;

	if (bufsize < AUTH_TOKEN_LEN * 2 + 1) {
		buf[0] = '\0';
		return;
	}

	if (!RtlGenRandom(random_bytes, sizeof random_bytes)) {
		buf[0] = '\0';
		return;
	}

	for (i = 0; i < AUTH_TOKEN_LEN; i++)
		snprintf(buf + i * 2, 3, "%02x", random_bytes[i]);
}

/*
 * Verify auth from a client connection.
 * Reads a line from fd and looks up the nonce in the pending list.
 *
 * The line may be:
 *   "<nonce>\n"                - regular imsg connection
 *   "TTY:<tty_token>:<nonce>\n" - tty channel connection
 *
 * Returns:
 *   0  = regular imsg connection (auth ok)
 *   1  = tty channel connection (auth ok, tty_token_out filled)
 *  -1  = auth failure
 */
int
win32_ipc_verify_auth(int fd, const char *label, char *tty_token_out,
    size_t tty_token_size)
{
	char	buf[256];
	int	n, i, is_tty = 0;
	char	ch;
	char	*nonce_part;

	(void)label; /* Nonce verified from pending list, not file. */

	/*
	 * The accepted socket may be non-blocking (inherited from the
	 * listening socket on Winsock). Set it to blocking temporarily
	 * so the auth recv completes reliably.
	 */
	{
		u_long zero = 0;
		ioctlsocket((SOCKET)fd, FIONBIO, &zero);
	}

	/* Read one byte at a time until newline. */
	for (i = 0; i < (int)(sizeof buf - 1); i++) {
		n = recv((SOCKET)fd, &ch, 1, 0);
		if (n != 1)
			break;
		if (ch == '\n')
			break;
		buf[i] = ch;
	}
	buf[i] = '\0';

	/* Check if this is a TTY channel: "TTY:<token>:<nonce>" */
	if (strncmp(buf, "TTY:", 4) == 0) {
		char *colon = strchr(buf + 4, ':');
		if (colon == NULL)
			return (-1);
		*colon = '\0';
		if (tty_token_out != NULL && tty_token_size > 0) {
			strncpy(tty_token_out, buf + 4, tty_token_size - 1);
			tty_token_out[tty_token_size - 1] = '\0';
		}
		nonce_part = colon + 1;
		is_tty = 1;
	} else {
		nonce_part = buf;
	}

	if (consume_pending_nonce(nonce_part) != 0)
		return (-1);

	return (is_tty ? 1 : 0);
}

/*
 * Connect a tty channel to the server via named pipe discovery.
 * Sends "TTY:<tty_token>:<nonce>\n" as the handshake line.
 * Returns socket FD or -1.
 */
int
win32_ipc_connect_tty(const char *label, const char *tty_token)
{
	SOCKET	s;
	char	nonce[AUTH_TOKEN_LEN * 2 + 1];
	char	*line;
	size_t	len;

	win32_wsa_init();

	s = pipe_discover_and_connect(label, nonce, sizeof nonce);
	if (s == INVALID_SOCKET)
		return (-1);

	/* Send "TTY:<tty_token>:<nonce>\n". */
	len = 4 + strlen(tty_token) + 1 + strlen(nonce) + 1;
	line = malloc(len + 1);
	if (line != NULL) {
		snprintf(line, len + 1, "TTY:%s:%s\n", tty_token, nonce);
		send(s, line, (int)len, 0);
		free(line);
	}

	return ((int)s);
}

/*
 * Clean up IPC resources.
 * On the server side, shuts down the pipe accept thread and closes the pipe.
 * On the client side (no pipe created), this is a no-op.
 */
void
win32_ipc_cleanup(const char *label)
{
	(void)label;

	/* Signal the pipe accept thread to shut down. */
	pipe_thread_running = 0;
	if (pipe_shutdown_event != NULL)
		SetEvent(pipe_shutdown_event);

	if (pipe_thread_handle != NULL) {
		WaitForSingleObject(pipe_thread_handle, 3000);
		CloseHandle(pipe_thread_handle);
		pipe_thread_handle = NULL;
	}

	if (discovery_pipe != INVALID_HANDLE_VALUE) {
		CloseHandle(discovery_pipe);
		discovery_pipe = INVALID_HANDLE_VALUE;
	}

	if (pipe_shutdown_event != NULL) {
		CloseHandle(pipe_shutdown_event);
		pipe_shutdown_event = NULL;
	}
}
