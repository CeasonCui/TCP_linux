#include        <netinet/in.h>
#include        <errno.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/socket.h>
#define MAXLINE                  4096
#define LISTENQ                  1024    /* 2nd argument to listen() */
#define SERV_PORT                9877
#define SA      struct sockaddr

void str_echo(int);
ssize_t readline(int, void *, size_t);
static ssize_t my_read(int, char *);

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
        ssize_t         n;
        char            line[MAXLINE];

        for ( ; ; ) {
                if ( (n = readline(sockfd, line, MAXLINE)) == 0)
                        return;         /* connection closed by other end */

                write(sockfd, line, n);
        }
}
/*end str_echo*/

ssize_t
readline(int fd, void *vptr, size_t maxlen)
{
        int             n, rc;
        char    c, *ptr;

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
