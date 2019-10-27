#ifndef SOCKET_H
#define SOCKET_H

#include "fsw.h"

enum fswSocket_type
{
    FSW_SOCK_TCP          =  1,
    FSW_SOCK_UDP          =  2,
};

int fswSocket_create(int domain, int type, int protocol);
int fswSocket_bind(int sock, int type, char *host, int port);
int fswSocket_listen(int sock, int backlog);
int fswSocket_accept(int sock);
int fswSocket_close(int fd);
int fswSocket_shutdown(int sock, int how);

ssize_t fswSocket_recv(int sock, void *buf, size_t len, int flag);
ssize_t fswSocket_send(int sock, const void *buf, size_t len, int flag);
int fswSocket_set_nonblock(int sock);

#endif	/* SOCKET_H */
