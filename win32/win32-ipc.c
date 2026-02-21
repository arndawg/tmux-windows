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

/*
 * RtlGenRandom is exported as SystemFunction036 from advapi32.dll.
 * It is the recommended way to generate cryptographic random bytes
 * without pulling in the full BCrypt API.
 */
#define RtlGenRandom SystemFunction036
BOOLEAN NTAPI RtlGenRandom(PVOID RandomBuffer, ULONG RandomBufferLength);

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
	FILE *f;
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

	f = fopen(path, "w");
	if (f != NULL) {
		fprintf(f, "%s", hex);
		fclose(f);
	}
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
	FILE *f;

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

	if (listen(s, 128) != 0) {
		closesocket(s);
		return (-1);
	}

	/* Write port to file. */
	port_path = get_port_file_path(label);
	if (port_path != NULL) {
		f = fopen(port_path, "w");
		if (f != NULL) {
			fprintf(f, "%u", (unsigned)port);
			fclose(f);
		}
		free(port_path);
	}

	/* Create auth token. */
	win32_ipc_create_auth_token(label);

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
 * Verify auth token from a client connection.
 * Reads a line from fd, compares to stored token.
 * Returns 0 on success, -1 on failure.
 */
int
win32_ipc_verify_auth_token(int fd, const char *label)
{
	char *expected, buf[AUTH_TOKEN_LEN * 2 + 2];
	int n;

	expected = read_auth_token(label);
	if (expected == NULL)
		return (0); /* No auth token file, allow anyway. */

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
	 * Read auth token one byte at a time until newline.
	 * This ensures we consume exactly the auth line and don't
	 * leave stray bytes in the socket buffer for the imsg layer.
	 */
	{
		int i;
		char ch;

		for (i = 0; i < (int)(sizeof buf - 1); i++) {
			n = recv((SOCKET)fd, &ch, 1, 0);
			if (n != 1)
				break;
			if (ch == '\n')
				break;
			buf[i] = ch;
		}
		buf[i] = '\0';
	}

	if (strcmp(buf, expected) != 0) {
		free(expected);
		return (-1);
	}

	free(expected);
	return (0);
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
