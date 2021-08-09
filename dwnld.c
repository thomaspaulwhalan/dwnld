#include<dwnld.h>

int tcp_connect(char *host_name, char *host_port)
{
#if defined(_WIN32)
	WSADATA d;
	FAIL_IF_R_M(WSAStartup(MAKEWORD(2, 2), &d), 1, stderr, "Failed to initialize.\n");
#endif

	printf("Configuring remote address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	FAIL_IF_R_M(getaddrinfo(host_name, host_port, &hints, &peer_address), 1, stderr, "getaddrinfo() failed\n");

	printf("Remote address is: ");
	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
	printf("%s %s\n", address_buffer, service_buffer);


	printf("Creating socket...\n");
	SOCKET socket_peer;
	socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	FAIL_IF_R_M(!ISVALIDSOCKET(socket_peer), 1, stderr, "socket() failed\n")

	printf("Connecting...\n");
	FAIL_IF_R_M(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen), 1, stderr, "connect() failed\n");
	freeaddrinfo(peer_address);

	printf("Connected.\n");
	printf("To send data, enter text followed by enter.\n");

	while(1) {

		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(socket_peer, &reads);
#if !defined(_WIN32)
		FD_SET(0, &reads);
#endif

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		FAIL_IF_R_M(select(socket_peer+1, &reads, 0, 0, &timeout) < 0, 1, stderr, "socket() failed\n");

		if (FD_ISSET(socket_peer, &reads)) {
			char read[4096];
			int bytes_received = recv(socket_peer, read, 4096, 0);
			BREAK_IF_M(bytes_received < 1, stderr, "Connection closed by peer.\n");
			printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);
		}

#if defined(_WIN32)
		if(_kbhit()) {
#else
		if(FD_ISSET(0, &reads)) {
#endif
			char read[4096];
			if (!fgets(read, 4096, stdin)) break;
			printf("Sending: %s", read);
			int bytes_sent = send(socket_peer, read, strlen(read), 0);
			printf("Sent %d bytes.\n", bytes_sent);
		}
	}

	printf("Closing socket...\n");
	CLOSESOCKET(socket_peer);

#if defined(_WIN32)
	WSACleanup();
#endif

	printf("Finished.\n");
	return 0;
}

url_data *get_url_data(char *url_address)
{
	/* This allows for at least http://a.co */
	/* Change to FAIL_IF_R() when finished testing */
	FAIL_IF_R_M(strlen(url_address) < 11, NULL, stderr, "url not long enough\n");

	int counter = 0;
	url_data *dwnld_data;

	dwnld_data = (url_data *)malloc(sizeof(url_data));
	/* Change this to FAIL_IF_R() when testing is finished */
	FAIL_IF_R_M(dwnld_data == NULL, NULL, stderr, "Could not allocate memory to dwnld_data\n");

	dwnld_data->url_address = (char *)malloc(sizeof(char) * (strlen(url_address) + 1));
	strcpy(dwnld_data->url_address, url_address);
	dwnld_data->url_address[strlen(url_address)] = '\0';

	/* Get dwnld_data->protocol */
	char protocol[] = "https";
	for (counter = 0; counter < 5; counter++) {
		if ((counter < 4) && (protocol[counter] == url_address[counter])) {
			continue;
		}
		else if (counter == 4) {
			if (protocol[counter] == url_address[counter]) {
				if ((url_address[counter+1] == ':') && (url_address[counter+2] == '/') && (url_address[counter+3] == '/')) {
					dwnld_data->protocol = "https\0";
					counter = 7;
				}
				else {
					return NULL;
				}
			}
			else if ((url_address[counter] == ':') && (url_address[counter+1] == '/') && (url_address[counter+2] == '/')) {
				dwnld_data->protocol = "http\0";
				counter = 6;
			}
			else {
				return NULL;
			}
		}
		else {
			return NULL;
		}
	}

	/* Get dwnld_data->host_name */
	for (int host_counter = 0; (url_address[counter] != '\0') && (url_address[counter] != '/'); host_counter++) {
		if (host_counter == 0) {
			dwnld_data->host_name = (char *)malloc(sizeof(char) * 2);
			FAIL_IF_R_M(dwnld_data->host_name == NULL, NULL, stderr, "Could not allocate memory to dwnld_data->host_name\n");
		}
		else {
			dwnld_data->host_name = (char *)realloc(dwnld_data->host_name, sizeof(char) * (host_counter+2));
			FAIL_IF_R_M(dwnld_data->host_name == NULL, NULL, stderr, "Could not reallocate memory to dwnld_data->host_name\n");
		}

		dwnld_data->host_name[host_counter] = url_address[counter];
		dwnld_data->host_name[host_counter+1] = '\0';

		counter++;
	}

	int url_pos = counter;

	/* Get dwnld_data->file_name */
	for (counter = strlen(url_address); url_address[counter-1] != '/'; counter--) {
		if (counter <= url_pos) {
			return NULL;
		}
	}

	for (int file_name_counter = 0; url_address[counter] != '\0'; file_name_counter++) {
		if (file_name_counter == 0) {
			dwnld_data->file_name = (char *)malloc(sizeof(char) * 2);
			FAIL_IF_R_M(dwnld_data->host_name == NULL, NULL, stderr, "Could not allocate memory to dwnld_data->file_name\n");
		}
		else {
			dwnld_data->file_name = (char *)realloc(dwnld_data->file_name, sizeof(char) * (file_name_counter+2));
			FAIL_IF_R_M(dwnld_data->host_name == NULL, NULL, stderr, "Could not allocate memory to dwnld_data->host_name\n");
		}

		dwnld_data->file_name[file_name_counter] = url_address[counter];
		dwnld_data->file_name[file_name_counter+1] = '\0';

		counter++;
	}
	
	/* Get dwnld_data->file_address
	 * NOT YET IMPLEMENTED
	 * */

	return dwnld_data;
}

url_data *get_dwnld_size(url_data *dwnld_data, char *host_port)
{
#if defined(_WIN32)
	WSADATA d;
	FAIL_IF_R_M(WSAStartup(MAKEWORD(2, 2), &d), NULL, stderr, "Failed to initialize.\n");
#endif

	/* Configure remote address */
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	FAIL_IF_R_M(getaddrinfo(dwnld_data->host_name, host_port, &hints, &peer_address), NULL, stderr, "getaddrinfo() failed\n");

	/* Create socket */
	SOCKET socket_peer;
	socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	FAIL_IF_R_M(!ISVALIDSOCKET(socket_peer), NULL, stderr, "socket() failed\n")

	/* Connect */
	FAIL_IF_R_M(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen), NULL, stderr, "connect() failed\n");
	freeaddrinfo(peer_address);

	/* Send and receive data */

	while(1) {

		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(socket_peer, &reads);
		FD_SET(0, &reads);

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		FAIL_IF_R_M(select(socket_peer+1, &reads, 0, 0, &timeout) < 0, NULL, stderr, "socket() failed\n");

		if (FD_ISSET(socket_peer, &reads)) {
			char read[4096];
			int bytes_received = recv(socket_peer, read, 4096, 0);
			BREAK_IF_M(bytes_received < 1, stderr, "Connection closed by peer.\n");

			dwnld_data->header_size = strlen(read);
			dwnld_data->header_contents = (char *)malloc(sizeof(char) * strlen(read));
			strcpy(dwnld_data->header_contents, read);
			break;
		}

		if(!FD_ISSET(0, &reads)) {
			char read[256] = "HEAD ";
			strcat(read, dwnld_data->url_address);
			strcat(read, " HTTP/1.1\r\nHost: ");
			strcat(read, dwnld_data->host_name);
			strcat(read, "\r\nConnection: keep-alive\r\nKeep-Alive: 300\r\n\r\n");
			send(socket_peer, read, strlen(read), 0);
		}
	}

	int newline_counter = 0;
	char char_len[] = "Content-Length: ";
	char file_size_s[64];
	for (int i = 0; i < dwnld_data->header_size; i++) {
		if (dwnld_data->header_contents[i] == '\n') {
			newline_counter++;
		}
		else if (newline_counter == 4) {
			for (int x = 0; x < 16; x++) {
				FAIL_IF_R(char_len[x] != dwnld_data->header_contents[i], NULL);
				i++;
			}
			int z = 0;
			while (dwnld_data->header_contents[i] != '\n') {
				file_size_s[z] = dwnld_data->header_contents[i];
				i++;
				z++;
			}
			newline_counter++;

		}
	}
	dwnld_data->file_size = atoi(file_size_s);

	CLOSESOCKET(socket_peer);

	return dwnld_data;
}

int dwnld(char *src_address, char *host_port)
{
	url_data *dwnld_data;
	dwnld_data = get_url_data(src_address);
	FAIL_IF_R(dwnld_data == NULL, 1);

	dwnld_data = get_dwnld_size(dwnld_data, host_port);
	FAIL_IF_R(dwnld_data == NULL, 1);

#if defined(_WIN32)
	WSADATA d;
	FAIL_IF_R_M(WSAStartup(MAKEWORD(2, 2), &d), 1, stderr, "Failed to initialize.\n");
#endif

	/* Configure remote address */
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	FAIL_IF_R_M(getaddrinfo(dwnld_data->host_name, host_port, &hints, &peer_address), 1, stderr, "getaddrinfo() failed\n");

	/* Create socket */
	SOCKET socket_peer;
	socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	FAIL_IF_R_M(!ISVALIDSOCKET(socket_peer), 1, stderr, "socket() failed\n")

	/* Connect */
	FAIL_IF_R_M(connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen), 1, stderr, "connect() failed\n");
	freeaddrinfo(peer_address);

	while(1) {

		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(socket_peer, &reads);
#if !defined(_WIN32)
		FD_SET(0, &reads);
#endif

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		FAIL_IF_R_M(select(socket_peer+1, &reads, 0, 0, &timeout) < 0, 1, stderr, "socket() failed\n");

		if (FD_ISSET(socket_peer, &reads)) {
			char *read;
			read = (char *)malloc((sizeof(char) * dwnld_data->file_size) + 512);
			FAIL_IF_R(read == NULL, 1);
			int bytes_received = recv(socket_peer, read, dwnld_data->file_size + 512, 0);
			BREAK_IF_M(bytes_received < 1, stderr, "Connection closed by peer.\n");
			
			FILE *dwnld_dest;
			dwnld_dest = fopen(dwnld_data->file_name, "w");
			FAIL_IF_R(dwnld_dest == NULL, 1);
			/* Iterate through *read starting just ahead of the header info */
			for (int i = bytes_received-dwnld_data->file_size; i < bytes_received; i++) {
				fputc(read[i], dwnld_dest);
			}

			fclose(dwnld_dest);
			fprintf(stderr, "%d bytes written to %s\n", dwnld_data->file_size, dwnld_data->file_name);

			break;
		}

		/* Send a GET request for the file */
#if defined(_WIN32)
		if(_kbhit()) {
#else
		if(!FD_ISSET(0, &reads)) {
#endif
			char read[256] = "GET ";
			strcat(read, src_address);
			strcat(read, " HTTP/1.1\r\nHost: ");
			strcat(read, dwnld_data->host_name);
			strcat(read, "\r\nConnection: keep-alive\r\nKeep-Alive: 300\r\n\r\n");
			send(socket_peer, read, strlen(read), 0);
		}
	}

	CLOSESOCKET(socket_peer);

#if defined(_WIN32)
	WSACleanup();
#endif

	free(dwnld_data);

	return 0;
}
