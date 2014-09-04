#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "index.h"


int index_init(){
	//init hashtable;
	int hashTableSize = YIDB_INDEX_HASHTABLE_STEP;
	yidbIndexhashTableObj = index_initHashtable(hashTableSize);

	//
	index_readFileInitIndex();

	return 0;
}

yidbIndexBlock *index_find(char *key){
	yidbIndexBlock  *tmpCell,*pos;

	//printf("key %s\n", key);
	//printf("%d %d\n", yidbIndexhashTableObj->size,index_hash(key,yidbIndexhashTableObj->size));
	 
	tmpCell = yidbIndexhashTableObj->list[index_hash(key,yidbIndexhashTableObj->size)];
	pos = tmpCell->next;
	while(pos!=NULL && strcmp(key,pos->key)!=0){
		pos = pos->next;
	}
	return pos;
}

int index_insert(char *key,int fieldId,long blockPos){
	yidbIndexBlock  *fib,*tmpCell,*dbib;
	dbib = createYidbIndexBlock(key,fieldId,blockPos);
	fib = index_find(dbib->key);
 
	if(fib==NULL){
		tmpCell = yidbIndexhashTableObj->list[index_hash(dbib->key,yidbIndexhashTableObj->size)];
		dbib->next = tmpCell->next;
		tmpCell->next = dbib;
	}else{
		//update
		fib->fieldId = dbib->fieldId;
		fib->blockPos = dbib->blockPos;
		
		destoryYidbIndexBlock(dbib);
	}

	return 0;
}

void setActiveFileIndexNewfieldId(int fieldId){
	int i;
	yidbIndexBlock *tmpCell,*pos;
	for(i=0;i<yidbIndexhashTableObj->size;i++){
		tmpCell = yidbIndexhashTableObj->list[i];
		pos = tmpCell->next;
		while(pos!=NULL){
			pos->fieldId = fieldId;
			pos = pos->next;
		}
	}
}



static yidbIndexHashTable *index_initHashtable(int size){
	int i;
	yidbIndexHashTable *iht;
 
	iht = malloc(sizeof(struct yidbIndexHashTable));
	iht->size = size;
	
	iht->list = malloc( sizeof(struct yidbIndexBlock *) * iht->size );
	for(i=0;i<size;i++){
		iht->list[i] = (yidbIndexBlock *) malloc(sizeof(struct yidbIndexBlock));
		iht->list[i]->next = NULL;
	}
	
	return iht;
}


static void index_readFileInitIndex(){
	//read dir file
	DIR * dir;
    struct dirent * ptr;
    dir = opendir(yidbconfigObj->storePath);
    while((ptr = readdir(dir)) != NULL)
    {
    	if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0){
    		continue;
    	}
    	if(strcmp(ptr->d_name,"yidb")>0){
    		index_readOneFileInitIndex(ptr->d_name);
    		//printf("d_name : %s:%d\n", name,fieldId);
    	}
        
    }
    closedir(dir);

    //active file 
	index_readOneFileInitIndex("activedb");
}

static void index_readOneFileInitIndex(char *filename){
	char file[100];
	char name[100];
	int fieldId;
	if(strcmp(filename,"activedb")==0){
		fieldId = 0;
	}else{
		sscanf(filename,"%4s_%d",name,&fieldId);
	}
	sprintf(file,"%s/%s",yidbconfigObj->storePath,filename);
	//printf("%d-%s\n", fieldId,file);return;
	FILE *fp;
	yidbBlock *dbb;
	
	int keySize,valueSize,exptime;
	char key[YIDB_MAX_KEYSIZE];
	char value[YIDB_MAX_VALUESIZE];
	long pos = 0;
	if( (fp = fopen(file,"rb")) != NULL){
		while(!feof(fp)){
			keySize = 0;
			pos = ftell(fp);
 			
 			fread(&keySize,sizeof(int),1,fp);
 			fread(&valueSize,sizeof(int),1,fp);
 			fread(&exptime,sizeof(int),1,fp);
			//key
			fread(key,keySize,1,fp);
 			//value guo
 			fread(value,valueSize,1,fp);
 			
			if(keySize>0 && valueSize>0 && strlen(key)!=0){
				//printf("%s,%d,%ld\n",key,fieldId,pos);
				index_insert(key,fieldId,pos);
			}

		}
		fclose(fp);
	}
}



static int index_hash(char *key,int tablesize){
	unsigned hash_val = 0;
	char *k;
	k = key;
	while(*k!='\0'){
		hash_val = (hash_val<<5) + *k++;
	}
	return hash_val%tablesize;
}


static yidbIndexBlock *createYidbIndexBlock(char *key,int fieldId,long blockPos){
	//printf("%s,%d,%d\n", key,fieldId,blockPos);
	yidbIndexBlock *dbib;
	dbib = malloc(sizeof(struct yidbIndexBlock));
	if(dbib==NULL){
		free(dbib);
		printf("createYidbIndexBlock malloc error\n");
		exit(EXIT_FAILURE);
	}
	
	dbib->key = malloc(strlen(key));
	memset(dbib->key,0,strlen(dbib->key));
	strcpy(dbib->key,key);
	dbib->fieldId = fieldId;
	dbib->blockPos = blockPos;

	return dbib;
}


static void destoryYidbIndexBlock(yidbIndexBlock *dbib){
	free(dbib->key);
	free(dbib);
}