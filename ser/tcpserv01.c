#include        <netinet/in.h>
#include        <errno.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/socket.h>
#include        <unistd.h>
#include	<dirent.h>
#define MAXLINE                  4096
#define LISTENQ                  1024    /* 2nd argument to listen() */
#define SERV_PORT                9877
#define SA      struct sockaddr

void str_echo(int);
ssize_t readline(int, void *, size_t);
static ssize_t my_read(int, char *);

int readFileList(char *basePath,int sockfd){
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	char path[1000];
        ssize_t  n;
	if((dir = opendir(basePath))==NULL){
		perror("Open dir error...");
		exit(1);
	}
	
	while((ptr = readdir(dir)) != NULL){
                //printf("in path\n");
		if(strcmp(ptr->d_name,".")==0||strcmp(ptr->d_name,"..")==0)
			continue;
		else if(ptr->d_type == 8){
                        //printf("in path8\n");
			memset(path,'\0',sizeof(path));
			strcpy(path,basePath);
			strcat(path,"/");
			strcat(path,ptr->d_name);
			//path=basePath;
                        //printf("path:%s\n",path);
			write(sockfd, path, sizeof(path));
                        printf("path1:%s\n",path);
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			}
		else if(ptr->d_type == 10){
                        //printf("in path10\n");
			memset(path,'\0',sizeof(path));
			strcpy(path,basePath);
			strcat(path,"/");
			strcat(path,ptr->d_name);
                        //n = readline(sockfd, path, MAXLINE);
                        //printf("%s\n",path);
			write(sockfd, path, n);
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			}
		else if(ptr->d_type == 4){
			memset(base,'\0',sizeof(base));
			strcpy(base,basePath);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			readFileList(base,sockfd);
		}
	}
	closedir(dir);
	return 1;
}

int
main(int argc, char **argv)
{
    int  listenfd, connfd;
    pid_t  childpid;
    socklen_t  clilen;
    struct sockaddr_in  cliaddr,servaddr;
    
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error.\n");
        exit(1);
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    if (bind(listenfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        printf("bind error.\n");
        exit(1);
    }       
    
    if (listen(listenfd,LISTENQ) < 0) {
        printf("listen error.\n");
        exit(1);
    }

    for( ; ;) {
        clilen = sizeof(cliaddr);
        if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
            printf("accept error.\n");
            exit(1);
        }

        if((childpid = fork()) == 0) {
	    close(listenfd) ;
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
}   	

void
str_echo(int sockfd)
{
        ssize_t  n;
        char line[MAXLINE];
        char *s="1\n";
        char buffer[80];
        getcwd(buffer,sizeof(buffer));
	//printf("dir is:%s\n",buffer);
	if(buffer == NULL){
		perror("getcwd error");
	}
        printf("dir is:%s\n",buffer);
        for ( ; ; ) {
                if ( (n = readline(sockfd, line, MAXLINE)) == 0){
                       
                        return;         /* connection closed by other end */
                }
                        
                //printf("line:%s",line);
                //printf("s:%s",s);
                if(strcmp(s,line)==0){
                        printf("in\n");
                        readFileList(buffer,sockfd);
                }
                //printf("jump\n");
                write(sockfd, line, n);
        }
}
/*end str_echo*/

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
        int n, rc;
        char c, *ptr;

        ptr = vptr;
        for (n = 1; n < maxlen; n++) {
                if ( (rc = my_read(fd, &c)) == 1) {
                        *ptr++ = c;
                        if (c == '\n')
                                break;  /* newline is stored, like fgets() */
                } else if (rc == 0) {
                        if (n == 1)
                                return(0);      /* EOF, no data read */
                        else
                                break;          /* EOF, some data was read */
                } else
                        return(-1);             /* error, errno set by read() */
        }

        *ptr = 0;       /* null terminate like fgets() */
        return(n);
}
/* end readline */

static ssize_t
my_read(int fd, char *ptr)
{
        static int      read_cnt = 0;
        static char     *read_ptr;
        static char     read_buf[MAXLINE];

        if (read_cnt <= 0) {
again:
                if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
                        if (errno == EINTR)
                                goto again;
                        return(-1);
                } else if (read_cnt == 0)
                        return(0);
                read_ptr = read_buf;
        }

        read_cnt--;
        *ptr = *read_ptr++;
        return(1);
}
/*end ssize_t*/
