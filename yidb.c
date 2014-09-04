#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "yidb.h"

static clock_t start,finish;

setConfig(){
	
	//config
	yidbconfigObj = malloc(sizeof(struct yidbconfig));
	strcpy(yidbconfigObj->storePath,"data");
	yidbconfigObj->port = YIDB_DB_NET_PORT;
}

int set(char *key,char *value,int exptime){
	//store save
	int fieldId = 0;
	long blockPos;

	//printf("store_save \n");

	store_save(key,value,exptime,&fieldId,&blockPos);

	//printf("store return fieldId:%d,blockPos:%d\n", fieldId,blockPos);
	//index
	index_insert(key,fieldId,blockPos);

	return 0;
}


int get(char *key,char *result){
	yidbIndexBlock *dbib;
	dbib = index_find(key);
	if(dbib==NULL){
		return -1;
	} 
	return store_get_value(dbib->fieldId,dbib->blockPos,result);
 
}

int exptime2(char *key){
	yidbIndexBlock *dbib;
	dbib = index_find(key);
	if(dbib==NULL){
		return -1;
	}
	return store_get_exptime(dbib->fieldId,dbib->blockPos);
}

int delete(char *key){
	yidbIndexBlock *dbib;
	dbib = index_find(key);
	if(dbib==NULL){
		return -1;
	}
	set(key,"0",2);
	return 0;
}


int execNetStr(char *str,char *result){
	//command
	int exptime;
	char command[10],key[YIDB_MAX_KEYSIZE],value[YIDB_MAX_VALUESIZE];
	sscanf(str,"%s %s %s %d",command,key,value,&exptime);
	//printf("%s %s %s %d\n", command,key,value,exptime);
	if(strcmp(command,"set")==0 && strlen(key)>0 && strlen(value)>0 ){
		set(key,value,exptime);
		strcpy(result,"200");
		return 0;
	}else if(strcmp(command,"get")==0){
		memset(value,0,sizeof(value));
		if(get(key,value)==0){
			strcpy(result,value);
			return 0;		
		}else{
			return -1;
		}
		
	}else if(strcmp(command,"exptime")==0){
		int etime;
		etime = exptime2(key);
		if(etime>=0){
			sprintf(result,"%d",etime);return 0;	
		}else{
			return -1;
		}

	}else if(strcmp(command,"delete")==0){
		if(delete(key)==0){
			strcpy(result,"200");return 0;
		}else{
			return -1;
		}

	}else{
		strcpy(result,"command error");
		return -1;
	}
	return 0;

}
 


void init_daemon(void)
{
    pid_t daemon; 
    int i,fd; 
     
    daemon = fork(); 
    if(daemon < 0){ 
        //printf("Fork Error!"); 
        exit(1); 
    } 
    else if (daemon > 0 ) { 
        //printf("Father process exited!"); 
        exit(0); 
    } 
    setsid(); 
    umask(0); 
    /*for (i = 0;i < getdtablesize();i++){ 
        close(i);
    }*/
}



int main(int argc,char *argv[]){
	
	int result;

	setConfig();

    opterr = 0;

    while( (result = getopt(argc, argv, "p:d:")) != -1 )
    {
          switch(result)
          {
              case 'p':
                  yidbconfigObj->port = atoi(optarg);
                  break;
              case 'd':
                  memset(yidbconfigObj->storePath,0,sizeof(yidbconfigObj->storePath));
                  strcpy(yidbconfigObj->storePath,optarg);
                  break;
              default:
                  printf("params error\n");
                  exit(EXIT_FAILURE);
                  break;
          }
    }
    //printf("datadir:%s,port:%d\n", yidbconfigObj->storePath,yidbconfigObj->port);

	store_init();
	index_init();

	init_daemon();
	

 	net_serverStart();


}