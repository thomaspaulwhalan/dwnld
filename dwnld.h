#ifndef DWNLD_H
#define DWNLD_H

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include<winsock2.h>
#include<ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include<conio.h>

#else
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>

#endif


#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif


#include<stdio.h>
#include<string.h>
#include<nerror.h>

/* This comment can be removed when the
 * get_url_toks() function is completed.
 * The purpose of this function is to take
 * a url address, say:
 * http://mycoderepository.com.au/code/filman/main.c
 * and split it into:
 * protocol = "http"
 * host_name = "mycoderepository.com.au"
 * file_address = {"code", "filman"}
 * file = "main.c"
 *
 * I also hope to be able to do one that
 * takes an ip address. say:
 * http://172.105.182.26/code/filman/main.c
 * protocol = "http"
 * host_name = "172.105.182.26"
 * file_address = {"code", "filman"}
 * file = "main.c"
 *
 * There will be no need for a user ever
 * needing to access the elements of the
 * struct themselves. Instead, the functions
 * that will be in dwnld.h will take the
 * information and know what to do with it.
 * */
typedef struct url_data {
	char *url_address;
	char *protocol;
	char *host_name;
	char *file_name;
	int file_size;
	int header_size;
	char *header_contents;
	int num_of_file_addr_toks;
}url_data;

int tcp_connect(char *host_name, char *host_port);
int dwnld(char *src_address, char *host_port);
url_data *get_url_data(char url_address[]);
url_data *get_dwnld_size(url_data *dwnld_data, char *host_port);

#endif // DWNLD_H
