#include "cache_utils.h"
#include <stdlib.h>

void initCacheLines(Pcache_line* head, int size)
{
    for(int i=0; i<size; i++)
    {
        head[i] = (Pcache_line)malloc(sizeof(cache_line));
        head[i]->tag = -1;
        head[i]->dirty = 0;
        head[i]->LRU_next = NULL;
        head[i]->LRU_prev = NULL;
    }
}

int flushCacheLines(Pcache_line* head, int size)
{
    int copyback = 0;
    for(int i=0; i<size; i++)
    {
        if(head[i]->dirty){
            copyback++;
        }
    }
    return copyback;
}

int ifHit(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    if(line->tag == tag){
        return 1;
    }else{
        return 0;
    }
}

int ifNew(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    if(line->tag == (unsigned)(-1)){
        return 1;
    }else{
        return 0;
    }
}

void updateCacheNew(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    line->tag = tag;
    line->dirty = 0;
}

int updateCacheUsed(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    int dirty = ifDirty(c, index, tag);
    int copyback = 0;
    if(dirty) copyback = 1;

    line->tag = tag;
    line->dirty = 0;

    return copyback;
}

int ifDirty(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index]; 

    int dirty = line->dirty;

    if(dirty){
        return 1;
    }else{
        return 0;
    }
}

void writeCacheLocal(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index]; 

    line->dirty = 1;
}

void writeBack(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    line->dirty = 0;
}