#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "store.h"

int store_init(){
	sprintf(activeFile,"%s/%s",yidbconfigObj->storePath,"activedb");
	active_fd_write = fopen(activeFile,"ab");
	if(active_fd_write==NULL){
		printf("file open error\n");exit(EXIT_FAILURE);
	}
	active_fd_read = fopen(activeFile,"rb");
	if(active_fd_read==NULL){
		printf("file open error\n");exit(EXIT_FAILURE);
	}
	//printf("%s\n", activeFile);
	//oldFileList
	getOldFileList();

 

}


int store_save(char *key,char *value,int exptime,int *fieldId,long *blockPos){
	yidbBlock *dbb;
	dbb = createYidbBlock(key,value,exptime);
	
	
	*fieldId = 0;
	*blockPos = ftell(active_fd_write);

	//printf("store save:%d-%d-%d-%s-%s,fieldId:0,pos:%d\n", dbb->keySize,dbb->valueSize,dbb->exptime,dbb->key,dbb->value,*blockPos);

	fwrite(&dbb->keySize,sizeof(int),1,active_fd_write);
	fwrite(&dbb->valueSize,sizeof(int),1,active_fd_write);
	fwrite(&dbb->exptime,sizeof(int),1,active_fd_write);
	fwrite(dbb->key,strlen(dbb->key),1,active_fd_write);
	fwrite(dbb->value,strlen(dbb->value),1,active_fd_write);
	fflush(active_fd_write);	

	//split file?
	unsigned long filesize;
	fseek(active_fd_read, 0L, SEEK_END);  
    filesize = ftell(active_fd_read); 
    //printf("%ld\n", filesize);
    if(filesize>YIDB_DB_FILE_SPLIT_SIZE){
    	splitFile();
    }

    //printf("destoryYidbBlock\n");

    destoryYidbBlock(dbb);

    //printf("destoryYidbBlock ok\n");

	return 0;
}

int store_get_value(int fieldId,long blockPos,char *value){
	yidbBlock *dbb;
	dbb = getYidbBlock(fieldId,blockPos);
	if(dbb==NULL){
		return -1;
	}
 	//exptime
 	if(dbb->exptime!=0){
	 	time_t ttime;
		time(&ttime);

		ttime = (int)ttime;
		if(ttime>dbb->exptime){
			destoryYidbBlock(dbb);
			return -1;
		}
	}
	//printf("store_get_value\n");
 	//cp value
 	memset(value,0,strlen(dbb->value));
 	strcpy(value,dbb->value);


 	destoryYidbBlock(dbb);
 

 	return 0;
}


int store_get_exptime(int fieldId,long blockPos){
	yidbBlock *dbb;
	int exptime;
	dbb = getYidbBlock(fieldId,blockPos);
	if(dbb==NULL){
		destoryYidbBlock(dbb);
		return -1;
	}
	exptime = dbb->exptime;
	destoryYidbBlock(dbb);
	return exptime;
}

static yidbBlock *getYidbBlock(int fieldId,long blockPos){
	yidbBlock *dbb;
	FILE *fp=NULL;
	int i;
	//read file
	char readFileName[50];
 
	getfieldIdFileName(fieldId,readFileName); 
 	
 	if(strcmp(readFileName,"activedb")==0){
 		fp = active_fd_read;
 	}else{
 		for(i=0;i<oldFileListPtr->count;i++){
	 		if(strcmp(oldFileListPtr->files[i]->fileName,readFileName)==0){
	 			fp = oldFileListPtr->files[i]->fp;break;
	 		}
 		}
 	}

 	//find read fp
 	if( fp == NULL ){
 		return NULL;
 	}
 
 	fseek(fp, blockPos, SEEK_SET);

 	
 	dbb = malloc(sizeof(struct yidbBlock));
 
 	fread(&dbb->keySize,sizeof(int),1,fp);
 	fread(&dbb->valueSize,sizeof(int),1,fp);
	fread(&dbb->exptime,sizeof(int),1,fp);
	//key value
	dbb->key = malloc(dbb->keySize);
	dbb->value = malloc(dbb->valueSize);

	memset(dbb->key,0,dbb->keySize);
	memset(dbb->value,0,dbb->valueSize);

	fread(dbb->key,dbb->keySize,1,fp);
	fread(dbb->value,dbb->valueSize,1,fp);

 	//printf("store_get fieldId:%d,pos:%d %d-%d-%d-%s-%s\n", fieldId,blockPos,dbb->keySize,dbb->valueSize,dbb->exptime,dbb->key,dbb->value);

  

 	if(dbb->keySize==0 || dbb->valueSize==0 || strlen(dbb->value)==0){
 		//not find
 		destoryYidbBlock(dbb);
 		return NULL;
 	}
 
 	return dbb;
}


static void splitFile(){
	fclose(active_fd_write);
	fclose(active_fd_read);
	//
	char newname[100];
	sprintf(newname,"%s/%s%d",yidbconfigObj->storePath,"yidb_",oldFileListPtr->nextField);
	rename( activeFile , newname );

	active_fd_write = fopen(activeFile,"ab");
	active_fd_read = fopen(activeFile,"rb");//

	//update index field
	setActiveFileIndexNewfieldId(oldFileListPtr->nextField);


	oldFileListPtr->nextField++;

	getOldFileList();
}

static int getfieldIdFileName(int fieldId,char *result){
	if(fieldId==0){
		strcpy(result,"activedb");
	}else{
		sprintf(result,"%s_%d",YIDB_DB_FILENAME_PRE,fieldId);
	}
	return 0;
}


static void getOldFileList(){
	DIR * dir;
    struct dirent * ptr;
  	char name[100];
  	int fieldId;
    int i;
    int maxFieldId = 0;
    oldFile *of;

	if(oldFileListPtr!=NULL){
		for(i=0;i<oldFileListPtr->count;i++){
			fclose(oldFileListPtr->files[i]->fp);
			free(oldFileListPtr->files[i]);
		}
		free(oldFileListPtr);
 	}
 	oldFileListPtr = malloc(sizeof(struct oldFileList));
	oldFileListPtr->nextField = 1;
	oldFileListPtr->count = 0;

	//read dir file
	
    dir = opendir(yidbconfigObj->storePath);
    while((ptr = readdir(dir)) != NULL)
    {
    	if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0){
    		continue;
    	}
    	if(strcmp(ptr->d_name,"yidb")>0){
    		sscanf(ptr->d_name,"%4s_%d",name,&fieldId);
    		//
    		of = malloc(sizeof(struct oldFile));
    		of->fieldID = fieldId;
    		sprintf(of->fileName,"%s_%d",name,fieldId);

    		//insert
    		oldFileListPtr->files[oldFileListPtr->count++] = of;

    		if(fieldId>maxFieldId){
    			maxFieldId = fieldId;
    		}
    		//printf("d_name : %s:%d\n", name,fieldId);
    	}
        
    }
    closedir(dir);

    oldFileListPtr->nextField = maxFieldId+1;


    //sort
    if(oldFileListPtr->count>0){
    	int i,j;
    	oldFile *tmpCell;
    	for(i=1;i<oldFileListPtr->count;i++){
    		tmpCell = oldFileListPtr->files[i];
    		for(j=i;j>0 && oldFileListPtr->files[j-1]->fieldID > tmpCell->fieldID;j--){
    			oldFileListPtr->files[j] = oldFileListPtr->files[j-1];
    		}
    		oldFileListPtr->files[j] = tmpCell;

    	}
    }

    //
    if(oldFileListPtr->count>0){ 
		int i;
		char filename[100];
		for(i=0;i<oldFileListPtr->count;i++){
			sprintf(filename,"%s/%s",yidbconfigObj->storePath,oldFileListPtr->files[i]->fileName);
			oldFileListPtr->files[i]->fp = fopen(filename,"rb");
		}
	}

}


static yidbBlock *createYidbBlock(char *key,char *value,int exptime){
	yidbBlock *dbb;
	dbb = malloc(sizeof(struct yidbBlock));
	if(dbb==NULL){
		free(dbb);
		printf("createYidbBlock malloc error\n");
		exit(EXIT_FAILURE);
	}

	dbb->key = malloc(strlen(key));
	dbb->value = malloc(strlen(value));
	strcpy(dbb->key,key);
	strcpy(dbb->value,value);
	dbb->keySize = strlen(key);
	dbb->valueSize = strlen(value);
	dbb->exptime = exptime;
	return dbb;
}

static void destoryYidbBlock(yidbBlock *dbb){
	
	free(dbb->key);
	free(dbb->value);
	free(dbb);
	
	dbb = NULL;
}