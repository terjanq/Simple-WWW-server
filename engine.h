#ifndef __ENGINE_H_
#define __ENGINE_H_

#include "utils.h"

#define PATH_SIZE 100
#define HOST_SIZE 20
#define HEADER_LENGTH 1000

#define HEADER_CONTENT_LENGTH 1000
#define TIMEOUT -2
#define BADREQUEST -3
#define CONNCLOSE -4
#define TIMEOUT_VALUE 3


typedef struct r_header{
	char *ptr_start;
	char path[PATH_SIZE + 1];
	char host[HOST_SIZE + 1];
	int conn_close;
	int fd;

} r_header;




// GET /asda HTTP1.1
void readPath(r_header *header, char *line);

// Connection: close
void readConnStatus(r_header *header, char *line);

// Host: localhost
void readHost(r_header *header, char *line);

int nextLine(char *start, char *line);


int SendResponse(r_header *header);


void clearHeader(r_header *header, char *start, int fd);

char *generateContentType(char* ext);
char *getExt(char *line);
int readFile(char *filepath);
int needsRedirecting(char *line);
int sendRedirect(r_header *header);

void sendNotFound(r_header *header);
int notImplemented(r_header *header);
void sendNotImplemented(r_header *header);
int isValidFilepath(r_header *header, char *filepath);
int startsWith(const char *pre, const char *str);

#endif