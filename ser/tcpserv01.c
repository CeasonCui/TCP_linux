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

char path_n[1000]="\0";
void str_echo(int);
ssize_t readline(int, void *, size_t);
static ssize_t my_read(int, char *);

char *readFileList(char *basePath,char *path_n){
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	char path[1000];
        ssize_t  n;
	if((dir = opendir(basePath))==NULL){
		perror("Open dir error...");
		exit(1);
	}
	//memset(path_n,'\0',sizeof(path_n));
	while((ptr = readdir(dir)) != NULL){
                //printf("in path\n");
                printf("type:%d\n",ptr->d_type);
		if(strcmp(ptr->d_name,".")==0||strcmp(ptr->d_name,"..")==0)
			continue;
		else if(ptr->d_type == 8){
                        //printf("in path8\n");
			memset(path,'\0',sizeof(path));
			strcpy(path,basePath);
			strcat(path,"/");
			strcat(path,ptr->d_name);
                        //strcpy(path_n,path);
			//path=basePath;
                        //printf("path:%s\n",path);
                        strcat(path_n,path);
                        strcat(path_n,"\n");
			//write(sockfd, path, sizeof(path));
                        //printf("path_n:%s\n",path_n);
                        //printf("path1:%s\n",path);
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
                        //printf("outprintf\n");
			}
		else if(ptr->d_type == 10){
                        //printf("in path10\n");
			memset(path,'\0',sizeof(path));
			strcpy(path,basePath);
			strcat(path,"/");
			strcat(path,ptr->d_name);
                        //n = readline(sockfd, path, MAXLINE);
                        //printf("%s\n",path);
                        strcat(path_n,path);
			//write(sockfd, path, n);
			//printf("d_name:%s/%s\n",basePath,ptr->d_name);
			}
		else if(ptr->d_type == 4){
			memset(base,'\0',sizeof(base));
			strcpy(base,basePath);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			readFileList(base,path_n);
		}
	}
	closedir(dir);
	return path_n;
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
        connfd=0;
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
        char *s="1";
        char buffer[80];
        char *path;
        const char *s2="2\n";
	char s3[1000]="resent.";
	char file_name_up[256];
	//const char *s4="udpserv1.c";
	socklen_t len;
	char mesg[MAXLINE];
	char mesg_n[MAXLINE];
	char filebuffer[8000];
	char fileupbuffer[8000];
	char filetap[1000]="Please enter the file name: ";
        path_n[1000]="\0";
        getcwd(buffer,sizeof(buffer));
	//printf("dir is:%s\n",buffer);
	if(buffer == NULL){
		perror("getcwd error");
	}
        printf("dir is:%s\n",buffer);
        for ( ; ; ) {
                if ( (n = read(sockfd, line, MAXLINE)) == 0){
                       
                        return;         /* connection closed by other end */
                }
                        
                //printf("line:%s\n",line);
                //printf("n:%d\n",n);
                char line_n[MAXLINE]="\0";
                for(int i=0;i<n-1;i++){
                        line_n[i]=line[i];
                }
                //printf("line_n:%s\n",line_n);
                if(strcmp(s,line_n)==0){
                        path_n[0]='\0';
                        //printf("in\n");
                        path=readFileList(buffer,path_n);
                        printf("path:%s\n",path);
                        // fputs(path_n, stdout);
                        write(sockfd,path_n,sizeof(path_n));
                        printf("send success\n");
                }
                else if(*line=='2'){
			char file_name[256];
			bzero(file_name,256);
			//write(sockfd, s3, sizeof(s3));
			
			for(int i=1;line[i]!='\n';i++){
				file_name[i-1]=line[i];
			}
			printf("change to %s\n",file_name);
			FILE*fp = fopen(file_name,"r");
			if(fp==NULL){
				printf("FILE:%s Not Found\n",file_name);
			}
			else{
                                //write(sockfd,s3,sizeof(s3));
                                //read(sockfd, line, MAXLINE);
				bzero(filebuffer,0);
				int length=0;
				while((length = fread(filebuffer,sizeof(char),8000,fp))>0){
					printf("length=%d\n",length);
					if(write(sockfd,filebuffer,length)<0){
						printf("Send File:%s Failed.\n",file_name);
						break;
					}
					bzero(filebuffer,8000);
				}
			}
                        fclose(fp);
			printf("File:%s Transfer Successful!\n",file_name);			
		}
		else if(*line=='3'){
			//printf("intter 3\n");
			for(int i=1;line[i]!='\n';i++){
				file_name_up[i-1]=line[i];
			}
			FILE *fp1=fopen(file_name_up,"w");
			if(fp1==NULL){
				printf("file can not open\n");
				exit(1);
			}
			bzero(fileupbuffer,MAXLINE+1);
			int lengthup=0;
			lengthup = read(sockfd, fileupbuffer, MAXLINE);
				/*if(lengthup<=10){
					break;
				}*/
				//printf("intter\n");
				printf("%d\n",lengthup);
				if(fwrite(fileupbuffer,sizeof(char),lengthup,fp1)<lengthup){
					printf("file write faile\n");
					break;
				}
				fputs(fileupbuffer, stdout);
				bzero(fileupbuffer,MAXLINE+1);
				//fputs(fileupbuffer,stdout);
				//mesg[lengthup] = 0; 
					
			//}
			printf("receive file successful\n");
			fclose(fp1);
		}
                //printf("jump\n");
                else{
                        write(sockfd, line_n, sizeof(line_n));
                }
                
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
