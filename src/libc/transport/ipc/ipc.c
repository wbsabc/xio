/*
  Copyright (c) 2013-2014 Dong Fang. All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "ipc.h"
#include "ds/list.h"

#define TP_IPC_SOCKDIR "/tmp/proxyio"
#define TP_IPC_BACKLOG 100

void ipc_close(int fd) {
    struct sockaddr_un addr = {};

    ipc_sockname(fd, addr.sun_path, sizeof(addr.sun_path));
    close(fd);
    if (strlen(addr.sun_path) > 0)
	unlink(addr.sun_path);
}


int ipc_bind(const char *sock) {
    int afd;
    struct sockaddr_un addr;
    socklen_t addr_len = sizeof(addr);
    
    ZERO(addr);
#ifdef SOCK_CLOEXEC
    if ((afd = socket(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0)
#else
    if ((afd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
#endif
	return -1;
    addr.sun_family = AF_LOCAL;
    snprintf(addr.sun_path,
	     sizeof(addr.sun_path), "%s/%s", TP_IPC_SOCKDIR, sock);
    unlink(addr.sun_path);
    if (bind(afd, (struct sockaddr *)&addr, addr_len) < 0
	|| listen(afd, TP_IPC_BACKLOG) < 0) {
	close(afd);
	return -1;
    }
    return afd;
}

int ipc_accept(int afd) {
    struct sockaddr_un addr = {};
    socklen_t addrlen = sizeof(addr);
    int fd = accept(afd, (struct sockaddr *) &addr, &addrlen);
    
    if (fd < 0 &&
	(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR ||
	 errno == ECONNABORTED || errno == EMFILE)) {
	errno = EAGAIN;
	return -1;
    }
    return fd;
}

int ipc_connect(const char *peer) {
    int fd;
    struct sockaddr_un addr;
    socklen_t addr_len = sizeof(addr);

    ZERO(addr);
    if ((fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
	return -1;
    addr.sun_family = AF_LOCAL;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s/%s", TP_IPC_SOCKDIR,
	     peer);
    if (connect(fd, (struct sockaddr *)&addr, addr_len) < 0) {
	close(fd);
	return -1;
    }
    return fd;
}

i64 ipc_recv(int sockfd, char *buf, i64 len) {
    i64 nbytes = recv(sockfd, buf, len, 0);

    if (nbytes == -1 &&
	(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
	errno = EAGAIN;
        return -1;
    } else if (nbytes == 0) {    //  Signalise peer failure.
	errno = EPIPE;
	return -1;
    }
    return nbytes;
}

i64 ipc_send(int sockfd, const char *buf, i64 len) {
    i64 nbytes = send(sockfd, buf, len, 0);

    if (nbytes == -1 &&
	(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)) {
	errno = EAGAIN;
        return -1;
    } else if (nbytes == -1) {
	// Signalise peer failure.
	errno = EPIPE;
	return -1;
    }
    return nbytes;
}


/* TODO:
 * getsockname/getpeername doesn't support unix domain socket.
 */

int ipc_sockname(int fd, char *sock, int size) {
    struct sockaddr_un addr = {};
    socklen_t addrlen = sizeof(addr);

    if (-1 == getsockname(fd, (struct sockaddr *)&addr, &addrlen))
	return -1;
    if (addrlen != sizeof(addr)) {
	errno = EINVAL;
	return -1;
    }
    size = size > strlen(addr.sun_path) ? strlen(addr.sun_path) : size;
    strncpy(sock, addr.sun_path, size);
    return 0;
}

int ipc_peername(int fd, char *peer, int size) {
    struct sockaddr_un addr = {};
    socklen_t addrlen = sizeof(addr);

    if (-1 == getpeername(fd, (struct sockaddr *)&addr, &addrlen))
	return -1;
    if (addrlen != sizeof(addr)) {
	errno = EINVAL;
	return -1;
    }
    size = size > strlen(addr.sun_path) ? strlen(addr.sun_path) : size;
    strncpy(peer, addr.sun_path, size);
    return 0;
}


static struct transport_vf ipc_ops = {
    .name = "ipc",
    .proto = TP_IPC,
    .init = 0,
    .exit = 0,
    .close = ipc_close,
    .bind = ipc_bind,
    .accept = ipc_accept,
    .connect = ipc_connect,
    .recv = ipc_recv,
    .send = ipc_send,
    .sendmsg = 0,
    .setopt = ipc_setopt,
    .getopt = NULL,
    .item = LIST_ITEM_INITIALIZE,
};

struct transport_vf *ipc_vfptr = &ipc_ops;


