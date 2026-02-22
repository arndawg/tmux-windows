/*
 * IPC layer for Windows.
 * Replaces Unix domain sockets with localhost TCP.
 * Implements auth token management for security.
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

/*
 * Create a file accessible only by the current user.
 * Uses a DACL with a single ACE granting GENERIC_ALL to the current user SID.
 * Returns a HANDLE or INVALID_HANDLE_VALUE on failure.
 */
static HANDLE
create_user_only_file(const char *path)
{
	HANDLE h, token;
	DWORD len;
	TOKEN_USER *tu = NULL;
	char *sid_str = NULL;
	char sddl[256];
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR *sd = NULL;

	/* Get current user SID. */
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
		return (INVALID_HANDLE_VALUE);

	GetTokenInformation(token, TokenUser, NULL, 0, &len);
	tu = malloc(len);
	if (tu == NULL || !GetTokenInformation(token, TokenUser, tu, len, &len)) {
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

	/* Build SDDL: owner=current user, DACL grants only current user full access. */
	snprintf(sddl, sizeof sddl, "D:P(A;;GA;;;%s)", sid_str);
	LocalFree(sid_str);

	if (!ConvertStringSecurityDescriptorToSecurityDescriptorA(
	    sddl, SDDL_REVISION_1, (PSECURITY_DESCRIPTOR *)&sd, NULL))
		return (INVALID_HANDLE_VALUE);

	memset(&sa, 0, sizeof sa);
	sa.nLength = sizeof sa;
	sa.lpSecurityDescriptor = sd;
	sa.bInheritHandle = FALSE;

	h = CreateFileA(path, GENERIC_WRITE, 0, &sa, CREATE_ALWAYS,
	    FILE_ATTRIBUTE_NORMAL, NULL);

	LocalFree(sd);
	return (h);
}

/*
 * Write a string to a user-only file. Returns 0 on success, -1 on failure.
 */
static int
write_user_only_file(const char *path, const char *content)
{
	HANDLE h;
	DWORD written;
	DWORD len = (DWORD)strlen(content);

	h = create_user_only_file(path);
	if (h == INVALID_HANDLE_VALUE)
		return (-1);

	WriteFile(h, content, len, &written, NULL);
	CloseHandle(h);
	return (written == len ? 0 : -1);
}

#define AUTH_TOKEN_LEN 32

/*
 * Derive a deterministic port from a label string.
 * Returns a port in the ephemeral range (49152-65535).
 */
static uint16_t
label_to_port(const char *label)
{
	uint32_t hash = 5381;
	const char *p;

	for (p = label; *p != '\0'; p++)
		hash = ((hash << 5) + hash) + (unsigned char)*p;

	return (uint16_t)(49152 + (hash % (65535 - 49152)));
}

/*
 * Get the path for the port file and auth token.
 * Returns allocated string: %LOCALAPPDATA%\tmux\<label>.port
 */
static char *
get_port_file_path(const char *label)
{
	char *appdata, *path;
	char dir[MAX_PATH];

	appdata = getenv("LOCALAPPDATA");
	if (appdata == NULL)
		appdata = getenv("APPDATA");
	if (appdata == NULL)
		appdata = "C:\\";

	snprintf(dir, sizeof dir, "%s\\tmux", appdata);
	_mkdir(dir);

	if (asprintf(&path, "%s\\%s.port", dir, label) == -1)
		return (NULL);
	return (path);
}

static char *
get_auth_token_path(const char *label)
{
	char *appdata, *path;
	char dir[MAX_PATH];

	appdata = getenv("LOCALAPPDATA");
	if (appdata == NULL)
		appdata = getenv("APPDATA");
	if (appdata == NULL)
		appdata = "C:\\";

	snprintf(dir, sizeof dir, "%s\\tmux", appdata);
	_mkdir(dir);

	if (asprintf(&path, "%s\\%s.auth", dir, label) == -1)
		return (NULL);
	return (path);
}

/*
 * Create auth token file.
 * Generates random bytes and writes them as hex to a file.
 */
void
win32_ipc_create_auth_token(const char *label)
{
	char *path;
	unsigned char random_bytes[AUTH_TOKEN_LEN];
	char hex[AUTH_TOKEN_LEN * 2 + 1];
	int i;

	path = get_auth_token_path(label);
	if (path == NULL)
		return;

	/* Generate random bytes. */
	if (!RtlGenRandom(random_bytes, sizeof random_bytes)) {
		free(path);
		return;
	}

	for (i = 0; i < AUTH_TOKEN_LEN; i++)
		snprintf(hex + i * 2, 3, "%02x", random_bytes[i]);

	write_user_only_file(path, hex);
	free(path);
}

/*
 * Read auth token from file.
 * Returns allocated string or NULL.
 */
static char *
read_auth_token(const char *label)
{
	char *path, *token = NULL;
	FILE *f;
	char buf[AUTH_TOKEN_LEN * 2 + 2];

	path = get_auth_token_path(label);
	if (path == NULL)
		return (NULL);

	f = fopen(path, "r");
	free(path);
	if (f == NULL)
		return (NULL);

	if (fgets(buf, sizeof buf, f) != NULL)
		token = strdup(buf);
	fclose(f);
	return (token);
}

/*
 * Create a server socket bound to localhost on a port derived from the label.
 * Writes the actual port to the port file.
 * Returns the listening socket FD or -1.
 */
int
win32_ipc_create_server(const char *label, uint16_t *out_port)
{
	SOCKET s;
	struct sockaddr_in addr;
	int addrlen = sizeof addr;
	uint16_t port;
	char *port_path;

	win32_wsa_init();

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return (-1);

	port = label_to_port(label);

	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons(port);

	/* Try the derived port first. */
	if (bind(s, (struct sockaddr *)&addr, sizeof addr) != 0) {
		/* Port busy; let the OS pick one. */
		addr.sin_port = 0;
		if (bind(s, (struct sockaddr *)&addr, sizeof addr) != 0) {
			closesocket(s);
			return (-1);
		}
	}

	if (getsockname(s, (struct sockaddr *)&addr, &addrlen) != 0) {
		closesocket(s);
		return (-1);
	}
	port = ntohs(addr.sin_port);

	/* Create auth token before listen() to prevent unauthenticated window. */
	win32_ipc_create_auth_token(label);

	if (listen(s, 128) != 0) {
		closesocket(s);
		return (-1);
	}

	/* Write port to file (user-only ACL). */
	port_path = get_port_file_path(label);
	if (port_path != NULL) {
		char port_str[16];
		snprintf(port_str, sizeof port_str, "%u", (unsigned)port);
		write_user_only_file(port_path, port_str);
		free(port_path);
	}

	if (out_port != NULL)
		*out_port = port;

	return ((int)s);
}

/*
 * Connect to the server.
 * Reads port from port file and connects to localhost.
 * Returns socket FD or -1.
 */
int
win32_ipc_connect(const char *label)
{
	SOCKET s;
	struct sockaddr_in addr;
	char *port_path, *auth_token;
	FILE *f;
	unsigned port;

	win32_wsa_init();

	port_path = get_port_file_path(label);
	if (port_path == NULL)
		return (-1);

	f = fopen(port_path, "r");
	free(port_path);
	if (f == NULL)
		return (-1);

	if (fscanf(f, "%u", &port) != 1) {
		fclose(f);
		return (-1);
	}
	fclose(f);

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return (-1);

	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons((uint16_t)port);

	if (connect(s, (struct sockaddr *)&addr, sizeof addr) != 0) {
		closesocket(s);
		return (-1);
	}

	/* Send auth token + newline as a single send to avoid TCP split. */
	auth_token = read_auth_token(label);
	if (auth_token != NULL) {
		size_t tlen = strlen(auth_token);
		char *line = malloc(tlen + 2);
		if (line != NULL) {
			memcpy(line, auth_token, tlen);
			line[tlen] = '\n';
			send(s, line, (int)(tlen + 1), 0);
			free(line);
		}
		free(auth_token);
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
 * Verify auth token from a client connection.
 * Reads a line from fd, compares to stored token.
 *
 * The line may be:
 *   "<auth_token>\n"          - regular imsg connection
 *   "TTY:<tty_token>:<auth_token>\n" - tty channel connection
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
	char *expected;
	char buf[256];
	int n, i, is_tty = 0;
	char ch;
	char *auth_part;

	expected = read_auth_token(label);
	if (expected == NULL)
		return (-1); /* No auth token file, reject connection. */

	/*
	 * The accepted socket may be non-blocking (inherited from the
	 * listening socket on Winsock). Set it to blocking temporarily
	 * so the auth recv completes reliably.
	 */
	{
		u_long zero = 0;
		ioctlsocket((SOCKET)fd, FIONBIO, &zero);
	}

	/*
	 * Read one byte at a time until newline.
	 */
	for (i = 0; i < (int)(sizeof buf - 1); i++) {
		n = recv((SOCKET)fd, &ch, 1, 0);
		if (n != 1)
			break;
		if (ch == '\n')
			break;
		buf[i] = ch;
	}
	buf[i] = '\0';

	/*
	 * Check if this is a TTY channel: "TTY:<token>:<auth>"
	 */
	if (strncmp(buf, "TTY:", 4) == 0) {
		char *colon = strchr(buf + 4, ':');
		if (colon == NULL) {
			free(expected);
			return (-1);
		}
		*colon = '\0';
		if (tty_token_out != NULL && tty_token_size > 0) {
			strncpy(tty_token_out, buf + 4, tty_token_size - 1);
			tty_token_out[tty_token_size - 1] = '\0';
		}
		auth_part = colon + 1;
		is_tty = 1;
	} else {
		auth_part = buf;
	}

	if (strcmp(auth_part, expected) != 0) {
		free(expected);
		return (-1);
	}

	free(expected);
	return (is_tty ? 1 : 0);
}

/*
 * Connect a tty channel to the server.
 * Sends "TTY:<tty_token>:<auth_token>\n" as the handshake line.
 * Returns socket FD or -1.
 */
int
win32_ipc_connect_tty(const char *label, const char *tty_token)
{
	SOCKET s;
	struct sockaddr_in addr;
	char *port_path, *auth_token;
	FILE *f;
	unsigned port;
	char *line;
	size_t len;

	win32_wsa_init();

	port_path = get_port_file_path(label);
	if (port_path == NULL)
		return (-1);

	f = fopen(port_path, "r");
	free(port_path);
	if (f == NULL)
		return (-1);

	if (fscanf(f, "%u", &port) != 1) {
		fclose(f);
		return (-1);
	}
	fclose(f);

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		return (-1);

	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons((uint16_t)port);

	if (connect(s, (struct sockaddr *)&addr, sizeof addr) != 0) {
		closesocket(s);
		return (-1);
	}

	/* Send "TTY:<tty_token>:<auth_token>\n" as a single send. */
	auth_token = read_auth_token(label);
	if (auth_token != NULL) {
		/* "TTY:" + token + ":" + auth + "\n" */
		len = 4 + strlen(tty_token) + 1 + strlen(auth_token) + 1;
		line = malloc(len + 1);
		if (line != NULL) {
			snprintf(line, len + 1, "TTY:%s:%s\n",
			    tty_token, auth_token);
			send(s, line, (int)len, 0);
			free(line);
		}
		free(auth_token);
	}

	return ((int)s);
}

/*
 * Clean up port and auth files for a label.
 */
void
win32_ipc_cleanup(const char *label)
{
	char *path;

	path = get_port_file_path(label);
	if (path != NULL) {
		_unlink(path);
		free(path);
	}

	path = get_auth_token_path(label);
	if (path != NULL) {
		_unlink(path);
		free(path);
	}
}
