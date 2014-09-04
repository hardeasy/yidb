#ifndef __YIDB_STORE_H
#define __YIDB_STORE_H

#include "yidb.h"

FILE *active_fd_write;
FILE *active_fd_read;

char activeFile[100];/*active file full path*/

//int lastFieldId;


typedef struct oldFile{
	int fieldID;
	char fileName[100];
	FILE *fp;
}oldFile;

typedef struct oldFileList{
	int nextField;
	int count;
	struct oldFile *files[1000];
}oldFileList;


static struct oldFileList *oldFileListPtr;   /*except active file*/

static yidbBlock *getYidbBlock(int fieldId,long blockPos);
static int getfieldIdFileName(int filedId,char *result);
static void splitFile();
static void getOldFileList();
static yidbBlock *createYidbBlock(char *key,char *value,int exptime);
static void destoryYidbBlock(yidbBlock *dbb);
#endif