#include        <netinet/in.h>
#include        <errno.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/socket.h>
#define MAXLINE                  4096
#define LISTENQ                  1024    /* 2nd argument to listen() */
#define SERV_PORT                9877
#define SA      struct sockaddr

void str_cli(FILE *,int);
ssize_t readline(int, void *, size_t);
static ssize_t my_read(int, char *);

int
main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in  servaddr;

    if (argc != 2) {
	printf("usage:tcpcli01 <IPaddress>");
        exit(1);
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket error.\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
        printf("connect error.\n");
        exit(1);
    }

    str_cli(stdin, sockfd);
    exit(0);
}   

void
str_cli(FILE *fp,int sockfd)
{
        char sendline[MAXLINE], recvline[MAXLINE];

        while (fgets(sendline,MAXLINE, fp) != NULL) {
                write(sockfd, sendline, strlen(sendline));
                //printf("readline=%s\n",readline);
                while(readline(sockfd, recvline, MAXLINE)){
                        printf("in readline while\n");
		        /*if (readline(sockfd, recvline, MAXLINE) == 0) {
			        printf("str_cli:server terminated prematurely.\n");
                                exit(1);
                        }*/
		        fputs(recvline, stdout);
                        printf("\n");
                        //printf("in readline while1\n");
                }

        }
}
/*end str_cli */

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
        static int read_cnt = 0;
        static char *read_ptr;
        static char read_buf[MAXLINE];

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

	
