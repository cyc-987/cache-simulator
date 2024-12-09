#include "cache_utils.h"
#include "cache.h"
#include <stdlib.h>

Pcache_line initaCacheLine()
{
    Pcache_line p = (Pcache_line)malloc(sizeof(cache_line));
    p->tag = -1;
    p->dirty = 0;
    p->LRU_next = p->LRU_prev = NULL;

    return p;
}

void initCacheLines(Pcache_line* head, int size, int assoc)
{
    if(assoc == 1){ // direct mapped cache, simply finish the initialize
        for(int i=0; i<size; i++)
        {
            head[i] = initaCacheLine();
        }
    }else{// lazy initialize, initialize on use
        for(int i=0; i<size; i++)
        {
            head[i] = NULL;
        }
    }
}

int flushCacheLines(Pcache_line* head, int size, int assoc)
{
    int copyback = 0;
    if(assoc == 1){
        for(int i=0; i<size; i++)
        {
            if(head[i]->dirty){
                copyback++;
            }
        }
    }else{
        for(int i=0; i<size; i++)
        {
            Pcache_line temp = head[i];
            while(temp != NULL){
                if(temp->dirty){
                    copyback++;
                }
                temp = temp->LRU_next;
            }
        } 
    }
    return copyback;
}

int ifHit(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    if(c->associativity == 1){// direct mapped cache
        if(line->tag == tag){
            return 1;
        }else{
            return 0;
        }
    }else{
        if(line == NULL){return 0;}
        else{
            Pcache_line temp = line;
            while(temp != NULL){
                if(temp->tag == tag){return 1;}
                temp = temp->LRU_next;
            }
            return 0;
        }
    }
}

int ifNew(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    if(c->associativity == 1){//direct mapped cache
        if(line->tag == (unsigned)(-1)){
            return 1;
        }else{
            return 0;
        }
    }else{
        if(c->set_contents[index] > 0){// still remains
            return 1;
        }else{
            return 0;
        }
    }
}

void updateLRU(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    if(c->associativity == 1){// direct mapped cache
        //do nothing
    }else{
        Pcache_line temp = line;
        while(temp != NULL){
            if(temp->tag == tag){//move to the front of the list
                delete(c->LRU_head+index, c->LRU_tail+index, temp);
                insert(c->LRU_head+index, c->LRU_tail+index, temp);
                break;
            }
            temp = temp->LRU_next;
        }        
    }
}

void updateCacheNew(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    if(c->associativity == 1){// direct mapped cache
        line->tag = tag;
        line->dirty = 0;
    }else{
        Pcache_line temp = initaCacheLine();
        temp->tag = tag;
        temp->dirty = 0;
        insert(c->LRU_head+index, c->LRU_tail+index, temp);
        c->set_contents[index]--;
    }
}

int updateCacheUsed(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    //check if the data needs to be copied back
    int dirty = 0;
    int copyback = 0;
    

    if(c->associativity == 1){//direct mapped cache
        dirty = ifDirty(c, index, tag);
        line->tag = tag;
        line->dirty = 0;
    }else{
        Pcache_line temp = initaCacheLine();
        Pcache_line tobe_deleted = c->LRU_tail[index];
        dirty = c->LRU_tail[index]->dirty;
        temp->tag = tag;
        temp->dirty = 0;
        delete(c->LRU_head+index, c->LRU_tail+index, tobe_deleted);
        insert(c->LRU_head+index, c->LRU_tail+index, temp); 
        free(tobe_deleted);
    }

    if(dirty) copyback = 1;
    return copyback;
}

int ifDirty(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index]; 

    int dirty = 0;
    if(c->associativity == 1){//direct mapped cache
        dirty = line->dirty;
    }else{
        Pcache_line temp = line;
        while(temp != NULL){
            if(temp->tag == tag){
                dirty = temp->dirty;
                break;
            }
            temp = temp->LRU_next;
        } 
    }

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

    if(c->associativity == 1){
        line->dirty = 1;
    }else{
        Pcache_line temp = line;
        while(temp != NULL){
            if(temp->tag == tag){
                temp->dirty = 1;
                break;
            }
            temp = temp->LRU_next;
        }  
    }
}

void writeTrough(Pcache c, unsigned index, unsigned tag)
{
    Pcache_line* head = c->LRU_head;
    Pcache_line line = head[index];

    line->dirty = 0;
}