#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "yidb.h"

#define BUFFER_SIZE (YIDB_MAX_KEYSIZE + YIDB_MAX_VALUESIZE + 100)
#define MAX_EVENTS 1000

/*
struct p_params{
    int sockfd;
    char buf[BUFFER_SIZE];
};


void *pthread_handleRequest(void *arg)  
{  
    struct p_params *pprams;
    pprams = (struct p_params *)arg;
    char result[200];
    memset(result,0,sizeof(result));
    execNetStr(pprams->buf,result);
    write(pprams->sockfd,result,sizeof(result));
    free(pprams);
    pthread_exit(0);
} 


void handleRequest(int sockfd,char *buf){
    
    struct p_params *pprams;
    pprams = malloc(sizeof(struct p_params));
    pprams->sockfd = sockfd;
    strcpy(pprams->buf,buf);
    pthread_t thread;
    pthread_create(&thread,NULL,pthread_handleRequest,(void *)pprams);
}
*/


int net_serverStart(){
	int server_sockfd,client_sockfd;
	int len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;

	int sin_size,sockfd;
	char buf[BUFFER_SIZE];
	memset(buf,0,BUFFER_SIZE);   
	memset(&server_address,0,sizeof(server_address));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(yidbconfigObj->port);

	//create socket server
	if( (server_sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1 ){
		perror("socket");
		exit(1);
	}
	//bind
	if( bind(server_sockfd,(struct sockaddr*)&server_address,sizeof(server_address)) == -1 ){
		perror("bind error");
 		exit(EXIT_FAILURE); 
	}
	//listen
	listen(server_sockfd,1000);
	sin_size=sizeof(struct sockaddr_in);
	//create epoll
	int epoll_fd;
	epoll_fd = epoll_create(MAX_EVENTS);
	if(epoll_fd==-1)  
    {  
        perror("epoll_create failed");  
        exit(EXIT_FAILURE);  
    }

    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    
    ev.events=EPOLLIN|EPOLLET;
    ev.data.fd=server_sockfd;
    //register server event
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_sockfd,&ev)==-1){
    	perror("epll_ctl:server_sockfd register failed");  
        exit(EXIT_FAILURE);  
    }

    int nfds;//event nums
    while(1){

    	//wait event
    	nfds = epoll_wait(epoll_fd,events,MAX_EVENTS,-1);
    	if(nfds==-1)  
        {   printf("failed\n");
            perror("start epoll_wait failed");  
            exit(EXIT_FAILURE);  
        } 
         
        int i;  
        for(i=0;i<nfds;i++)  
        {
        	if(events[i].data.fd==server_sockfd){
                //printf("new socket\n");
        		//client come
        		if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&client_address,&sin_size))<0)  
                {     
                    perror("accept client_sockfd failed");     
                    exit(EXIT_FAILURE);  
                }  

                //register client event
                ev.events=EPOLLIN|EPOLLET;  
                ev.data.fd=client_sockfd;
                if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_sockfd,&ev)==-1)  
                {  
                    perror("epoll_ctl:client_sockfd register failed\n");  
                    exit(EXIT_FAILURE);  
                } 
                //printf("accept client %s\n",inet_ntoa(client_address.sin_addr));
        	}else if(events[i].events&EPOLLIN){
                 
        		//event raise
                sockfd = events[i].data.fd;
        		len = read(sockfd, buf, BUFFER_SIZE);  
                //printf("len:%d %s\n",len,buf);
                if(len<=0)  
                {   
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,events[i].data.fd,&ev);  
                    close(sockfd);
                    continue;
                } 
                
               
                //exec
                char result[200];
                memset(result,0,sizeof(result));
                execNetStr(buf,result);
                
                //printf("result:%s\n",result); 
                write(sockfd,result,sizeof(result)); 
                
                //handleRequest(sockfd,buf);

        	}
        }

    }

    close(server_sockfd);
    close(epoll_fd);

    return 0;
}

