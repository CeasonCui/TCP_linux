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
    while(1){
        if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) < 0) {
                printf("connect error.\n");
                exit(1);
        }

        str_cli(stdin, sockfd);
        exit(0);
    }
}   



void
str_cli(FILE *fp,int sockfd)
{
        ssize_t  n;
        char sendline[MAXLINE], recvline[MAXLINE+1];
        printf("--MENU--\n");
        printf("1.list\n");
        printf("2.download file. eg: down:text.txt\n");
        printf("3.up file. eg: up:text.txt\n");
        printf("4.else echo\n");
        printf("--------\n");
        while (fgets(sendline,MAXLINE, fp) != NULL) {
               // while(1){
                        write(sockfd, sendline, strlen(sendline));
                        if((sendline[0]=='u'&&sendline[1]=='p')||(sendline[0]=='d'&&sendline[1]=='o'&&sendline[2]=='w'&&sendline[3]=='n'&&sendline[4]==':'))
			{file_updown(sendline,sockfd);}
                        else{
                        if (n = read(sockfd, recvline, MAXLINE) == 0) {
			        printf("str_cli:server terminated prematurely.\n");
                                exit(1);
                        }
  
                int i=0;
		fputs(recvline, stdout);
                        }
                printf("\n");
                //}
                
        }
}
/*end str_cli */

void
file_updown(char *sendline,int sockfd,char *recvline){
        const char *s="1\n";
	const char *s2="2\n";
	const char *s3="resent.";
	///char *u="up:";
	//char *d="down:";
        char errormessage[10]="error";
        char file_name[256];
        char buffer[MAXLINE+1];
	char filebuffer[8000];
        if(sendline[0]=='u'&&sendline[1]=='p'&&sendline[2]==':'){
		//printf("\n�������ļ���:\n");
		char file_name_up[256];
		bzero(file_name_up,256);
		for(int i=3;sendline[i]!='\n';i++){
			file_name_up[i-3]=sendline[i];
		}
		printf("filename:%s\n",file_name_up);
		FILE *fp1=fopen(file_name_up,"r");
		if(fp1==NULL){
			printf("FILE:%s Not Found\n",file_name_up);
		}
		else{
			//write(sockfd, sendline, strlen(sendline));
			bzero(filebuffer,0);
			int length=0;
			length = fread(filebuffer,sizeof(char),8000,fp1);
						//printf("length=%d\n",length);
						
				if(write(sockfd,filebuffer,length)<0){
					printf("Send File:%s Failed.\n",file_name_up);
				}
				fputs(filebuffer, stdout);
				bzero(filebuffer,8000);
			fclose(fp1);
			printf("File:%s Transfer Successful!\n",file_name_up);
		}		
	}
	else{
		if(sendline[0]=='d'&&sendline[1]=='o'&&sendline[2]=='w'&&sendline[3]=='n'&&sendline[4]==':'){
				//printf("\n�������ļ���:\n");
			        for(int i=5;sendline[i]!='\n';i++){
			                file_name[i-5]=sendline[i];
                                }
			bzero(buffer,MAXLINE+1);
			int length=0;
		        length=read(sockfd, buffer, MAXLINE);
                        printf("buffer=%s",buffer);
                        if(strcmp(buffer,errormessage)==0){
                                printf("FILE:%s Not Found\n",file_name);
                                exit(1);
                        }
                        FILE *fp=fopen(file_name,"w");
			if(fp==NULL){
				printf("file can not open\n");
				exit(1);
			}
			if(fwrite(buffer,sizeof(char),length,fp)<length){
				printf("file write faile\n");
			}
			fputs(buffer, stdout);
			bzero(buffer,MAXLINE+1);
			//fputs(buffer,stdout);
				
		
		        //write(sockfd, sendline, strlen(sendline));
                        fclose(fp);
                        printf("receive file successful\n");
                        }
			//if(sendline=)
                } 			
	}

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

	
