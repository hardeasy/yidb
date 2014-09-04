#ifndef __YIDB_H
#define __YIDB_H

#define  TOSTR(x) #x

#define YIDB_MAX_KEYSIZE 100 /*max keysize 100 byte*/
#define YIDB_MAX_VALUESIZE 1024 * 10 /* max value size 10kb */

#define YIDB_INDEX_HASHTABLE_STEP 5000; /* index hash table step 5000*/

#define YIDB_DB_FILENAME_PRE "yidb"   /*dbfile pre name*/
#define YIDB_DB_FILE_STORE_PATH "data" /*dbfile store path*/
#define YIDB_DB_FILE_SPLIT_SIZE 1024 * 1024 * 20 /* each file size */


#define YIDB_DB_NET_PORT 2048  /*bind port*/

typedef struct yidbBlock{
	int keySize;
	int valueSize;
	int exptime;
	char *key;
	char *value;
}yidbBlock;


typedef struct yidbIndexHashTable{
	int size;
	struct yidbIndexBlock **list;
}yidbIndexHashTable;


typedef struct yidbIndexBlock{
	char *key;
	int fieldId;
	long blockPos;
	struct yidbIndexBlock *next;
}yidbIndexBlock;


typedef struct yidbconfig{
	char storePath[50];
	int port;
}yidbconfig;



yidbconfig *yidbconfigObj;


int execNetStr(char *str,char *result);
/*net_epoll*/
int net_serverStart();

/*store*/
int store_init();
int store_save(char *key,char *value,int exptime,int *fieldId,long *blockPos);
int store_get_value(int fieldId,long blockPos,char *value);
int store_get_exptime(int fieldId,long blockPos);

/*index*/
int index_init();
yidbIndexBlock *index_find(char *key);
void setActiveFileIndexNewfieldId(int fieldId);
int index_insert(char *key,int fieldId,long blockPos);

/*log*/



#endif
