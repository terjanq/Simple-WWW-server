// terjanq
// 20.05.2018
// Server WWW

#include "sockwrap.h"
#include "engine.h"
#include "utils.h"



int GetDataTillStreamEnd (int fd, char *buffer, int buffer_size, int timeout)
{
	struct timeval tv; tv.tv_sec = timeout; tv.tv_usec = 0;		// pozostały czas
	char *buff_ptr = buffer;
	int total_bytes_read = 0;

	r_header header;

	clearHeader(&header, buffer, fd);

	while (total_bytes_read < buffer_size) {
		printf ("[dbg:]: current value of tv = %.3f\n", tv.tv_sec + tv.tv_usec * 1.0 / 1000000);

		fd_set descriptors;
		FD_ZERO (&descriptors);
		FD_SET (fd, &descriptors);
		int ready = Select(fd+1, &descriptors, NULL, NULL, &tv);
		if (!ready) return -2; 		


		int bytes_read = Recv(fd, buff_ptr, buffer_size - total_bytes_read, 0);
		if (bytes_read <= 0) return 0;
		char* new_buff_ptr = buff_ptr + bytes_read;

		for(; buff_ptr < new_buff_ptr; buff_ptr++){
			if( buff_ptr - header.ptr_start >= 3 && strncmp("\r\n\r\n", buff_ptr - 3, 4) == 0){
				int sr = SendResponse(&header);
				if( sr == BADREQUEST ) return BADREQUEST;
				if( sr == CONNCLOSE ) return CONNCLOSE;
				clearHeader(&header, buff_ptr, fd);

			}
		}

		total_bytes_read += bytes_read;
	}
	return 0;
}
	
int main(int argc, char **argv)
{

	if(argc != 3) {
		printf("[USAGE]: %s PORT WWW-ROOT", argv[0]);
		exit(1);
	}
	int PORT = atoi(argv[1]);

	if ( chdir(argv[2]) != 0 ){
		printf ("[ERROR]: CHDIR error %s\n", strerror(errno));
		exit(1);
	}


	int sockfd = Socket(AF_INET, SOCK_STREAM, 0);   
	struct sockaddr_in server_address;
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	Bind (sockfd, &server_address, sizeof(server_address));
	Listen (sockfd, 64);        

	while (1) {	


		struct sockaddr_in client_address;
		socklen_t len = sizeof(client_address);
		int conn_sockfd = Accept (sockfd, &client_address, &len);
		char ip_address[20];
		inet_ntop (AF_INET, &client_address.sin_addr, ip_address, sizeof(ip_address));
		printf ("[dbg]: New client %s:%d\n", ip_address, ntohs(client_address.sin_port));

		const int BUFFER_SIZE = 1000;
		char recv_buffer[BUFFER_SIZE+1];


		int n = GetDataTillStreamEnd (conn_sockfd, recv_buffer, BUFFER_SIZE, TIMEOUT_VALUE);

		if (n == TIMEOUT) {				
			printf ("[dbg] Client Timeouted\n");
		} else if (n == -1) {
			printf ("[dbg]: Read error: %s\n", strerror(errno));
		} else if (n == 0 || n == CONNCLOSE) {
			printf ("[dbg]: Client closed connection");
		} 
		
		Close (conn_sockfd);
		printf ("[dbg]: Disconnected\n");
	}
}

