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
    if(!strcmp(cmd,"lls")){
        return 1;
    }else if(!strcmp(cmd,"ls")){
        return 2;
    }else if(!strcmp(cmd,"g")){
        return 3;
    }else if(strstr(cmd,"cd") != NULL){
        return 4;
    }else if(strstr(cmd,"lcd") != NULL){
        return 5;
    }else if(strstr(cmd,"get") != NULL){
        return 6;
    }else if(strstr(cmd,"put") != NULL){
        return 7;
    }

}

char* getbehind(char cmd[128]){
    char* p;
    p = (char*)malloc(128);
    p = strtok(cmd," ");
    p = strtok(NULL," ");

    return p;
}

void getmessage(char cmd[128],int c_fd){
    char readbuf[8000];
    char* p = getbehind(cmd);
    read(c_fd,readbuf,8000);

    int fd = open(p,O_RDWR | O_CREAT,0666);
    write(fd,readbuf,strlen(readbuf));
    printf("receive success!\n");
    close(fd);
    memset(p,0,8000);

}

void putmessage(char cmd[128],int c_fd){
    char* readbuf = (char*)malloc(128);
    readbuf = getbehind(cmd);
    int cfd;
    char *p = (char*)malloc(8000);

    if(access(readbuf,F_OK) == -1){
        printf("no file\n");
    }else{
        cfd = open(readbuf,O_RDWR,0666);
        read(cfd,p,sizeof(p));
        close(cfd);
        memset(p,0,8000);
    }


}

void choosecmd(char cmd[128],int c_fd){
    int ret = change(cmd);
    char *p = (char*)malloc(8000);
    switch(ret){
        case 1:
            system("ls");
            break;
        case 2:
            read(c_fd,p,1024);
            printf("%s\n",p);
            memset(p,0,1024);
            break;
        case 3:
            printf("unconnect\n");
            write(c_fd,"away host",128);
            close(c_fd);
            exit(-1);
            break;
        case 4:
            break;
        case 5:
            p = getbehind(cmd);
            chdir(p);
            memset(p,0,8000);
            break;
        case 6:

            getmessage(cmd,c_fd);
            break;
        case 7:
            putmessage(cmd,c_fd);
            break;
    }

}

int main(int argc,char ** argv){


    int c_fd;
    struct sockaddr_in c_addr;
    int clent;
    char writebuf[128];
    char readbuf[1024];

    //控制台传参不正确，退出
    if(argc != 3){
        perror("argc");
        exit(1);
    }
    
    //1.创建客户端套接字
    c_fd = socket(AF_INET,SOCK_STREAM,0);
    if(c_fd == -1){
        perror("socket");
        exit(1);
    }

    //2.创建sockaddr_in结构体，并connect服务器
    
    //2.1初始化结构体
    memset(&c_addr,0,sizeof(struct sockaddr_in));
    //2.2结构体赋值
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(atoi(argv[2])); //字符串转换为32位网络字节序
    inet_aton(argv[1],&c_addr.sin_addr);

    //2.3连接
    clent = sizeof(struct sockaddr_in);
    if(connect(c_fd,(struct sockaddr*)&c_addr,clent)){
        perror("connect");
        exit(-1);
    }

    printf("connect success\n");

    //
    while(1){

        //从标准输入中读
        gets(writebuf);

        printf("cmd:%s\n",writebuf);

        //将指令传给套接字
        write(c_fd,writebuf,strlen(writebuf));

        choosecmd(writebuf,c_fd);

        printf("-----------------cmd-----------\n-");

        memset(writebuf,0,strlen(writebuf));

    }
    
    return 0;
}
