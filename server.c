#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<fcntl.h>


int change(char cmd[128]){

    if(!strcmp(cmd,"ls")){
        return 1;
    }else if(!strcmp(cmd,"ps")){
        return 2;
    }else if(!strcmp(cmd,"g")){
        return 3;
    }else if(strstr(cmd,"cd")){
        return 4;
    }else if(strstr(cmd,"lcd")){
        return 5;
    }else if(strstr(cmd,"get")){
        return 6;
    }else if(strstr(cmd,"put")){
        return 7;
    }

}

char * getbehind(char cmd[128]){
    char *p;
    p = (char *)malloc(128);
    p = strtok(cmd," ");
    p = strtok(NULL," ");

    return p;
}

void putmessage(char cmd[128],int c_fd){
    char readbuf[8000];
    char *p = getbehind(cmd);
    read(c_fd,readbuf,8000);

    int fd = open(p,O_RDWR | O_CREAT,0666);
    write(fd,readbuf,strlen(readbuf));

    printf("receive success \n");
    close(fd);
    memset(readbuf,0,8000);
}

void choosecmd(char cmd[128],int c_fd){
    int ret = change(cmd);
    char *p = (char *)malloc(8000);

    FILE *fdb;
    char freadbuf[128];

    char *readbuf = (char*)malloc(128);

    int sfd;

    switch (ret){
        case 1:
            //ls 服务器的目录
            fdb = popen("ls","r");
            fread(freadbuf,sizeof(freadbuf),1,fdb);
            write(c_fd,freadbuf,sizeof(freadbuf));
            memset(freadbuf,0,sizeof(freadbuf));
            printf("ok\n");
            break;
        case 2:
            system("ps");
            break;
        case 3:
            read(c_fd,freadbuf,128);
            printf("%s\n",freadbuf);
            exit(1);
            break;
        case 4:
            p = getbehind(cmd);
            chdir(p);
            memset(p,0,sizeof(p));
            break;
        case 5:
            break;
        case 6:
            readbuf = getbehind(cmd);
            if(access(readbuf,F_OK) == -1){
                write(c_fd,"NO FILE",sizeof("NO FILE"));
            }else{
                sfd = open(readbuf,O_RDWR,0666);
                read(sfd,p,8000);
                write(c_fd,p,strlen(p));
                close(sfd);
                memset(p,0,8000);
            }
            break;
        case 7:

            putmessage(cmd,c_fd);
            break;
    }

}

int main(int argc,char ** argv){

    int c_fd;
    int s_fd;
    int nread;
    int c_len;

    char readbuf[128];
    char writebuf[128];

    struct sockaddr_in s_addr;
    struct sockaddr_in c_addr;


    if(argc != 3){

        perror("argc");
        exit(1);

    }

    s_fd = socket(AF_INET,SOCK_STREAM,0);

    if(s_fd == -1){
        perror("socket");
        exit(1);
    }

    memset(&s_addr,1,sizeof(struct sockaddr_in));

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1],&s_addr.sin_addr);

    bind(s_fd,(struct sockaddr*)&s_addr,sizeof(struct sockaddr_in));

    listen(s_fd,10);

    printf("waiting connect\n");

    c_len = sizeof(struct sockaddr);

    while(1){

        c_fd = accept(s_fd,(struct sockaddr*)&c_addr,&c_len);
        if(c_fd == -1){
            perror("accept");
            exit(1);
        }

        printf("connect success %s\n",inet_ntoa(c_addr.sin_addr));

        //从客户端读

        if(fork() == 0){
            while(1){

                nread = read(c_fd,readbuf,128);

                choosecmd(readbuf,c_fd);

                memset(readbuf,0,sizeof(readbuf));
            }
        }
        

    }
    

    return 0;



}