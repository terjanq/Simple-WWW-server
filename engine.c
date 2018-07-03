// terjanq
// 20.05.2018
// Server WWW


#include "engine.h"
#include "sockwrap.h"


// -3 invalid host -2 not found, 0 forbidden
int isValidFilepath(r_header *header, char *filepath){
	if( strcmp(header-> host, "domena1.sieci.edu") != 0 && strcmp(header->host, "domena2.sieci.edu") != 0 ) return -3;
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL){
		      perror("getcwd() error");
		      exit(-1);
	}
	strcat( cwd, "/");
	strcat( cwd, header->host );

    char rpath[PATH_MAX];
    char *res = realpath(filepath, rpath);

    if (res == NULL) {
        return -2;
    }

	if( !startsWith(cwd, rpath) ) return 0;

	return 1;
}

int startsWith(const char *pre, const char *str){
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}


int notImplemented(r_header *header){
	return header->path[0] == '\0' || header->host[0] == '\0';
}



void sendForbidden(r_header *header){
	char *fancymsg = "<h1>HTTP 403</h1> <p>Forbidden</p>";
	char msg[1000];

	int len = sprintf(msg, "HTTP/1.1 403 Forbidden\r\nContent-length: %d\r\n\r\n%s", (int)strlen(fancymsg), fancymsg);
	Send(header->fd, msg, len, 0);
}
void sendNotImplemented(r_header *header){
	char *fancymsg = "<h1>HTTP 501</h1> <p>Non Implemented</p>";
	char msg[1000];

	int len = sprintf(msg, "HTTP/1.1 501 Not Implemented\r\nContent-length: %d\r\n\r\n%s", (int)strlen(fancymsg), fancymsg);
	Send(header->fd, msg, len, 0);
}
void sendNotFound(r_header *header){
	char *fancymsg = "<h1>HTTP 404</h1><p>Not Found</p>";
	char msg[1000];

	int len = sprintf(msg, "HTTP/1.1 404 Not Found\r\nContent-length: %d\r\n\r\n%s", (int)strlen(fancymsg), fancymsg);
	Send(header->fd, msg, len, 0);
}

int sendRedirect(r_header *header){
	char msg[100];
	int len = sprintf(msg, "HTTP/1.1 301 Moved Permanently\r\nLocation: %sindex.html\r\n\r\n", header->path);
	Send(header->fd, msg, len, 0);
	return 1;
}

int needsRedirecting(char *line){
	char *ptr = line;

	while(*(ptr + 1) != '\0'){
		ptr++;
	}

	if(*ptr == '/') return 1;
	return 0;
}


int sendFile(r_header *header){

	char filepath[255];

	sprintf(filepath, "%s/%s", header->host, header->path);

	printf("[Filepath]: %s\n", filepath);

	int isv = isValidFilepath(header, filepath);

	if( isv == -3 ){
		sendNotImplemented(header);
		return 1;
	}
	if( isv == 0 ){
		sendForbidden(header);
		return 1;
	}
	if( isv == -2 ){
		sendNotFound(header);
		return 1;
	}


	int f = open(filepath, O_RDONLY);
	if ( f == -1 ){
    	fprintf (stderr, "failed to open %s\n", filepath);
        sendNotFound(header);
        return 0;
    }
	const int buf_size = 2000000;
	char buffer[buf_size];
	int buf_len = read(f, buffer, buf_size);

	char response_header[1000];
	char *ct = generateContentType(header->path);

	int header_len = sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-type: %s\r\nContent-length: %d\r\n\r\n", ct, buf_len);
	Send(header->fd, response_header, header_len, 0);
	Send(header->fd, buffer, buf_len, 0);

	return 1;
}

char *getExt(char *line){
	int i;
	char *ptr = line;

	int len = 0;
	while(*ptr != '\0'){
		ptr++;
		len++;
	}
	for(i=0; i<=len && *ptr != '.'; i++, ptr--);
	if(i == len + 1) return "";
	return ptr + 1;
}

char *generateContentType(char* filepath){

	char *ext = getExt(filepath);

	if( strcmp(ext, "txt") == 0 ) return "text/plain";
	if( strcmp(ext, "html") == 0 ) return "text/html";
	if( strcmp(ext, "css") == 0 ) return "text/css";
	if( strcmp(ext, "jpg") == 0 ) return "image/jpeg";
	if( strcmp(ext, "jpeg") == 0 ) return "image/jpeg";
	if( strcmp(ext, "png") == 0 ) return "image/png";
	if( strcmp(ext, "pdf") == 0 ) return "application/pdf";
	return "application/octet-stream";
}

// GET /asdasdasd HTTP/1.1
void readPath(r_header *header, char *line){
	char *ptr_l = line + 4;
	for(int i=0; i<PATH_SIZE && *ptr_l != ' '; i++, ptr_l++) header->path[i] = *ptr_l;
}

// Connection: close
void readConnStatus(r_header *header, char *line){
	if( strcmp("close", line + 12) == 0 ) {
		header->conn_close = 1;
	} 
}

// Host: localhost
void readHost(r_header *header, char *line){
	char *ptr_l = line + 6;
	for(int i=0; i<HOST_SIZE && *ptr_l != '\0' && *ptr_l != ':'; i++, ptr_l++) header->host[i] = *ptr_l;
}

int nextLine(char *start, char *line){
	char *st_ptr = start; 
	char *ln_ptr = line;

	while( *st_ptr != '\r' &&  *(st_ptr + 1) != '\n') *(ln_ptr++) = *(st_ptr++);
	*(ln_ptr) = '\0';
	return st_ptr - start;

}


int SendResponse(r_header *header){
	int line;
	char *start = header->ptr_start;
	char hd[HEADER_LENGTH];
	while( *start == '\n' || *start == '\r' ) start++;
	while( (line = nextLine(start, hd)) ){
		if( strncmp("GET", hd, 3) == 0 ) readPath(header, hd);
		if( strncmp("Connection:", hd, 11) == 0 ) readConnStatus(header, hd);
		if( strncmp("Host:", hd, 5) == 0 ) readHost(header, hd);
		start += line + 2;
	}



	if( needsRedirecting(header->path) ) sendRedirect(header);
	else 
		if( notImplemented(header) ) sendNotImplemented(header);
		else{
			sendFile(header);
		}
	printf("[Host]: %s\n[Path]: %s\n", header->host, header->path);

	if( header->conn_close ) return CONNCLOSE;
	return 0;
}


void clearHeader(r_header *header, char *start, int fd){
	header->ptr_start = start;
	for(int i=0; i<PATH_SIZE; i++) header->path[i] = '\0';
	for(int i=0; i<HOST_SIZE; i++) header->host[i] = '\0';
	header->conn_close = 0;
	header->fd = fd;
}