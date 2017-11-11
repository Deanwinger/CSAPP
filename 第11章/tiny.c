#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void serve_dynamic(int fd, char *filename, char *cigargs);
void get_filetype(char *filename, char *filetype);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv)
{
	int listenfd, connfd;
	char hostname[MAXLINE], port[MAXLINE];
	socklen_t clientlen;
	struct sockaddr_storage clientaddr;

	// check command-lines args
	if( argc != 2){
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	} 

	listenfd = Open_listenfd(argv[1]);
	while(1){
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
		Getnameinfo = ((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
		printf("Accepted connection from (%s %s)\n", hostname, port);

		doit(connfd);
		Close(connfd);
	}
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXLINE];
	//build the http response body
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

	// print the HTTP response
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)(strlen(body)));
	Rio_writen(fd, buf, strlen(buf));
	Rio_writen(fd, buf, strlen(body));


}

void doit(int fd)
{
	int is_static;
	struct stat sbuf;
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;

	//read request line and headers
	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, MAXLINE);
	printf("Request headers: \n");
	printf("%s", buf);
	sscanf(buf, "%s %s %s%", method, uri, version);
	if (strcasecmp(method, "GET")){
		clienterror(fd, method, "501", "Not implemented",
			"Tiny does not support this method");
		return;
	}
	read_requesthdrs(&rio);
	// parse uri from GET request
	is_static = parse_uri(uri, filename, cgiargs);
	if (stat(filename, &stat) < 0){
		clienterror(fd, method, "404", "Not found",
			"Tiny could not find this file.");
		return;
	}

	if (is_static){
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
			clienterror(fd, method, "403", "Forbidden",
			"Tiny could not read this file.");
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else{
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
			clienterror(fd, method, "403", "Forbidden",
			"Tiny could not run the CGI program.");
		}
		serve_dynamic(fd, filename, cgiargs);
	}

	// parse uri from GET request 

	
}


void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];

	Rio_readlineb(rp, buf, MAXLINE);
	while(strcmp(buf, "\r\n")){
		Rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);
	}
	return;
}

parse_uri(char *uri, char *filename, char *cgiargs)
{
	char *ptr;
	if (！strstr(uri, "cgi-bin")){
		/* code */
		strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		if (uri[strlen(uri) - 1] == '/')
		{
			/* code */
			strcat(filename, "home.html");
			return 1;
		}

	}
	else{
		ptr = index(uri, "?");
		if (ptr){
			/* code */
			strcpy(cgiargs, ptr+1);
			*ptr = '\0';
		}
		else
			strcpy(cgiargs, "");
		strcpy(filename, "");
		strcat(filename, ".");
		return 0;
	}
}

void serve_static(int fd, char *filename, int filesize)
{
	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXLINE];

	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	Rio_writen(fd, buf, strlen(buf));

	srcfd = Open(filename，O_RDONLY, 0);
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	Close(srcfd);
	Rio_writen(fd, srcp, filesize);
	Munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype)
{
	// strstr() This function returns a pointer
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else 
		strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	Rio_writen(fd, buf, strlen(buf));

	if (Fork() == 0){
		setenv("QUERY_STRING", cgiargs, 1);
		// Redirect stdout to client		
		Dup2(fd, STDOUT_FILENO);
		Execve(filename, emptylist, environ);
	}
	Wait(NULL);
}