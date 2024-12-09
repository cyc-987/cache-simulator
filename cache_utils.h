#ifndef __CACHE_UTILS_H__
#define __CACHE_UTILS_H__


#include "main.h"
#include "cache.h"

Pcache_line initaCacheLine();
void initCacheLines(Pcache_line* head, int size, int assoc);
int flushCacheLines(Pcache_line* head, int size, int assoc);

int ifHit(Pcache c, unsigned index, unsigned tag);
int ifNew(Pcache c, unsigned index, unsigned tag);

void updateLRU(Pcache c, unsigned index, unsigned tag);
void updateCacheNew(Pcache c, unsigned index, unsigned tag);
int updateCacheUsed(Pcache c, unsigned index, unsigned tag);

int ifDirty(Pcache c, unsigned index, unsigned tag);

void writeCacheLocal(Pcache c, unsigned index, unsigned tag);
void writeTrough(Pcache c, unsigned index, unsigned tag);


#endif