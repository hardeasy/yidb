#ifndef __YIDB_INDEX_H
#define __YIDB_INDEX_H

#include "yidb.h"

static yidbIndexHashTable *yidbIndexhashTableObj; 


static yidbIndexHashTable *index_initHashtable(int size);
static void index_readFileInitIndex();
static void index_readOneFileInitIndex(char *filename);
static int index_hash(char *key,int tablesize);
static yidbIndexBlock *createYidbIndexBlock(char *key,int fieldId,long blockPos);
static void destoryYidbIndexBlock(yidbIndexBlock *dbib);
#endif