#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


main(int argc,char *argv[]){
	int sockfd;
	int len;
	struct sockaddr_in address;

	char sendStr[1000];
	time_t ttime;
	time(&ttime);

	ttime = (int)ttime;
	ttime = ttime+100;

	sprintf(sendStr,"set key1 wwdsa %d",ttime);

	char buffer[1000];
	memset(buffer,0,sizeof(buffer));
	int result;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	address.sin_port = htons(2048);

	len = sizeof(address);

	result = connect(sockfd,(struct sockaddr *)&address,len);
	if(result==-1){
		perror("client1 error");
		exit(1);
	}

	printf("send:%s\n",sendStr);

	write(sockfd,sendStr,sizeof(sendStr));
	len=read(sockfd,buffer,sizeof(buffer));  
    printf("receive from server:%s\n",buffer);  
    if(len<0)  
    {  
          perror("receive from server failed");  
          exit(EXIT_FAILURE);  
    } 

    memset(sendStr,0,sizeof(sendStr));

    sprintf(sendStr,"get key1");

    clock_t start,finish;
    start = clock();

    int i;
    for(i=0;i<500000;i++){

	    write(sockfd,sendStr,sizeof(sendStr));
		len=read(sockfd,buffer,sizeof(buffer));  
	    //printf("receive2 from server:%s\n",buffer);  
	    if(len<0)  
	    {  
	        perror("receive from server failed");  
	        exit(EXIT_FAILURE);  
	    } 

	}

	finish = clock();
	printf( "find %f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC ); 
    
	close(sockfd);
	exit(0);
}