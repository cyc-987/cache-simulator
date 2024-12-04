#ifndef __CACHE_UTILS_H__
#define __CACHE_UTILS_H__


#include "main.h"
#include "cache.h"

void initCacheLines(Pcache_line* head, int size);
int flushCacheLines(Pcache_line* head, int size);

int ifHit(Pcache c, unsigned index, unsigned tag);
int ifNew(Pcache c, unsigned index, unsigned tag);

void updateCacheNew(Pcache c, unsigned index, unsigned tag);
int updateCacheUsed(Pcache c, unsigned index, unsigned tag);

int ifDirty(Pcache c, unsigned index, unsigned tag);

void writeCacheLocal(Pcache c, unsigned index, unsigned tag);
void writeBack(Pcache c, unsigned index, unsigned tag);


#endif