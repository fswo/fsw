#ifndef SOCKET_H
#define SOCKET_H

#include "fsw.h"

enum fswSocket_type
{
    FSW_SOCK_TCP          =  1,
    FSW_SOCK_UDP          =  2,
};

int fswSocket_create(int domain, int type, int protocol);
bool fswSocket_bind(int sock, int type, char *host, int port);
bool fswSocket_listen(int sock, int backlog);
int fswSocket_accept(int sock);
bool fswSocket_close(int fd);
bool fswSocket_shutdown(int sock, int how);
bool fswSocket_set_option(int fd, int level, int optname, const void *optval, socklen_t optlen);
bool fswSocket_get_option(int fd, int level, int optname, void *optval, socklen_t *optlen);
bool fswSocket_getname(int fd, sockaddr *addr, socklen_t *len);
bool fswSocket_getpeername(int fd, sockaddr *addr, socklen_t *len);

ssize_t fswSocket_recv(int sock, void *buf, size_t len, int flag);
ssize_t fswSocket_send(int sock, const void *buf, size_t len, int flag);
int fswSocket_set_nonblock(int sock);

#endif	/* SOCKET_H */
