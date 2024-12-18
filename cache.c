/*
 * cache.c
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cache.h"
#include "main.h"
#include "cache_utils.h"

/* cache configuration parameters */
static int cache_split = 0;
static int cache_usize = DEFAULT_CACHE_SIZE;
static int cache_isize = DEFAULT_CACHE_SIZE; 
static int cache_dsize = DEFAULT_CACHE_SIZE;
static int cache_block_size = DEFAULT_CACHE_BLOCK_SIZE;
static int words_per_block = DEFAULT_CACHE_BLOCK_SIZE / WORD_SIZE;
static int cache_assoc = DEFAULT_CACHE_ASSOC;
static int cache_writeback = DEFAULT_CACHE_WRITEBACK;
static int cache_writealloc = DEFAULT_CACHE_WRITEALLOC;

/* cache model data structures */
static Pcache icache;
static Pcache dcache;
static cache c1;
static cache c2;
static cache_stat cache_stat_inst;
static cache_stat cache_stat_data;

/************************************************************/
void set_cache_param(int param, int value)
{

  switch (param) {
  case CACHE_PARAM_BLOCK_SIZE:
    cache_block_size = value;
    words_per_block = value / WORD_SIZE;
    break;
  case CACHE_PARAM_USIZE:
    cache_split = FALSE;
    cache_usize = value;
    break;
  case CACHE_PARAM_ISIZE:
    cache_split = TRUE;
    cache_isize = value;
    break;
  case CACHE_PARAM_DSIZE:
    cache_split = TRUE;
    cache_dsize = value;
    break;
  case CACHE_PARAM_ASSOC:
    cache_assoc = value;
    break;
  case CACHE_PARAM_WRITEBACK:
    cache_writeback = TRUE;
    break;
  case CACHE_PARAM_WRITETHROUGH:
    cache_writeback = FALSE;
    break;
  case CACHE_PARAM_WRITEALLOC:
    cache_writealloc = TRUE;
    break;
  case CACHE_PARAM_NOWRITEALLOC:
    cache_writealloc = FALSE;
    break;
  default:
    printf("error set_cache_param: bad parameter value\n");
    exit(-1);
  }

}
/************************************************************/

/************************************************************/
void init_cache()
{

  /* initialize the cache, and cache statistics data structures */

  //unifined cache
  if(cache_split){
    c1.size = cache_dsize;

    c2.size = cache_isize;
    c2.associativity = cache_assoc; 
    c2.n_sets = c2.size / (cache_block_size * c2.associativity);
    c2.index_mask_offset = LOG2(cache_block_size);
    c2.index_mask = (c2.n_sets - 1) << c2.index_mask_offset;

    c2.LRU_head = (Pcache_line *)malloc(c2.n_sets * sizeof(Pcache_line));
    c2.LRU_tail = (Pcache_line *)malloc(c2.n_sets * sizeof(Pcache_line));

    c2.contents = c2.n_sets;
    c2.set_contents = (int*)malloc(c2.n_sets * sizeof(int));
    for(int i=0;i<c2.n_sets;i++){c2.set_contents[i] = c2.associativity;}

    initCacheLines(c2.LRU_head, c2.n_sets, c2.associativity);
    initCacheLines(c2.LRU_tail, c2.n_sets, c2.associativity);
  }else{
    c1.size = cache_usize;
  }
  c1.associativity = cache_assoc; 
  c1.n_sets = c1.size / (cache_block_size * c1.associativity);
  c1.index_mask_offset = LOG2(cache_block_size);
  c1.index_mask = (c1.n_sets - 1) << c1.index_mask_offset;

  c1.LRU_head = (Pcache_line *)malloc(c1.n_sets * sizeof(Pcache_line));
  c1.LRU_tail = (Pcache_line *)malloc(c1.n_sets * sizeof(Pcache_line));

  c1.contents = c1.n_sets;
  c1.set_contents = (int*)malloc(c1.n_sets * sizeof(int));
  for(int i=0;i<c1.n_sets;i++){c1.set_contents[i] = c1.associativity;}

  initCacheLines(c1.LRU_head, c1.n_sets, c1.associativity);
  initCacheLines(c1.LRU_tail, c1.n_sets, c1.associativity);

}
/************************************************************/

/************************************************************/
void perform_access(unsigned addr, unsigned access_type)
{

  /* handle an access to the cache */

  // get index and tag from addr
  unsigned index = (addr & c1.index_mask) >> c1.index_mask_offset;
  unsigned tag = addr >> (LOG2(cache_block_size) + LOG2(c1.n_sets));

  Pcache Pcache_selected;
  Pcache_stat Pcache_stat_selected;

  if(access_type == TRACE_DATA_LOAD || access_type == TRACE_INST_LOAD)
  {// load
  
    //select right cache and status    
    if(access_type == TRACE_DATA_LOAD){
      Pcache_selected = &c1;
      Pcache_stat_selected = &cache_stat_data;
    }else{
      Pcache_selected = &c2;
      Pcache_stat_selected = &cache_stat_inst;
    }
    if(cache_split == FALSE){
      Pcache_selected = &c1; //unified cache is default to c1
    }

    index = (addr & Pcache_selected->index_mask) >> Pcache_selected->index_mask_offset;
    tag = addr >> (LOG2(cache_block_size) + LOG2(Pcache_selected->n_sets));

    int hit = ifHit(Pcache_selected, index, tag);//check if hit

    if(hit){//hit
      updateLRU(Pcache_selected, index, tag);//move the hitted one to the front of the list
    }else{//not hit

      //check if new
      int new = ifNew(Pcache_selected, index, tag);

      Pcache_stat_selected->demand_fetches += words_per_block; 
      Pcache_stat_selected->misses++;

      if(new){//new
        updateCacheNew(Pcache_selected, index, tag);
      }else{//the block has been used
        int copyback = updateCacheUsed(Pcache_selected, index, tag);
        if(copyback){Pcache_stat_selected->copies_back += words_per_block;}
        Pcache_stat_selected->replacements++;
      }
    }

    Pcache_stat_selected->accesses++;

  }else if(access_type == TRACE_DATA_STORE)
  {// store
    Pcache_selected = &c1;
    Pcache_stat_selected = &cache_stat_data;

    index = (addr & Pcache_selected->index_mask) >> Pcache_selected->index_mask_offset;
    tag = addr >> (LOG2(cache_block_size) + LOG2(Pcache_selected->n_sets));

    int hit = ifHit(Pcache_selected, index, tag);//check if hit

    if(hit){//hit
      if(cache_writeback){
        //set dirty bit
        writeCacheLocal(Pcache_selected, index, tag);
        updateLRU(Pcache_selected, index, tag);
      }else{
        updateLRU(Pcache_selected, index, tag);
        Pcache_stat_selected->copies_back += 1;
      }
    }else{//miss
      Pcache_stat_selected->misses++;
      if(cache_writealloc){//write allocate
        //fetch
        //check if new
        int new = ifNew(Pcache_selected, index, tag);
        Pcache_stat_selected->demand_fetches += words_per_block; 
        if(new){//new
          updateCacheNew(Pcache_selected, index, tag);
        }else{//the block has been used
          int copyback = updateCacheUsed(Pcache_selected, index, tag);
          if(copyback){Pcache_stat_selected->copies_back += words_per_block;}
          Pcache_stat_selected->replacements++;
        }
        if(cache_writeback){//treated as hit
          writeCacheLocal(Pcache_selected, index, tag);
          updateLRU(Pcache_selected, index, tag);
        }else{
          updateLRU(Pcache_selected, index, tag);
          Pcache_stat_selected->copies_back += 1;
        }

      }else{//not write allocate, direct write to mem
        Pcache_stat_selected->copies_back += 1;
      }
    }
    Pcache_stat_selected->accesses++;

  }else{
    printf("error perform_access: bad access type\n");
    exit(-1);
  }

}
/************************************************************/

/************************************************************/
void flush()
{

  /* flush the cache */
  int copyback = flushCacheLines(c1.LRU_head, c1.n_sets, c1.associativity);
  if(cache_split){copyback += flushCacheLines(c2.LRU_head, c2.n_sets, c2.associativity);}
  cache_stat_data.copies_back += copyback*words_per_block;

}
/************************************************************/

/************************************************************/
void delete(Pcache_line* head, Pcache_line* tail, Pcache_line item)
{
  if (item->LRU_prev) {
    item->LRU_prev->LRU_next = item->LRU_next;
  } else {
    /* item at head */
    *head = item->LRU_next;
  }

  if (item->LRU_next) {
    item->LRU_next->LRU_prev = item->LRU_prev;
  } else {
    /* item at tail */
    *tail = item->LRU_prev;
  }
}
/************************************************************/

/************************************************************/
/* inserts at the head of the list */
void insert(Pcache_line* head, Pcache_line* tail, Pcache_line item)
{
  item->LRU_next = *head;
  item->LRU_prev = (Pcache_line)NULL;

  if (item->LRU_next)
    item->LRU_next->LRU_prev = item;
  else
    *tail = item;

  *head = item;
}
/************************************************************/

/************************************************************/
void dump_settings()
{
  printf("*** CACHE SETTINGS ***\n");
  if (cache_split) {
    printf("  Split I- D-cache\n");
    printf("  I-cache size: \t%d\n", cache_isize);
    printf("  D-cache size: \t%d\n", cache_dsize);
  } else {
    printf("  Unified I- D-cache\n");
    printf("  Size: \t%d\n", cache_usize);
  }
  printf("  Associativity: \t%d\n", cache_assoc);
  printf("  Block size: \t%d\n", cache_block_size);
  printf("  Write policy: \t%s\n", 
	 cache_writeback ? "WRITE BACK" : "WRITE THROUGH");
  printf("  Allocation policy: \t%s\n",
	 cache_writealloc ? "WRITE ALLOCATE" : "WRITE NO ALLOCATE");
}
/************************************************************/

/************************************************************/
void print_stats()
{
  printf("\n*** CACHE STATISTICS ***\n");

  printf(" INSTRUCTIONS\n");
  printf("  accesses:  %d\n", cache_stat_inst.accesses);
  printf("  misses:    %d\n", cache_stat_inst.misses);
  if (!cache_stat_inst.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses,
	 1.0 - (float)cache_stat_inst.misses / (float)cache_stat_inst.accesses);
  printf("  replace:   %d\n", cache_stat_inst.replacements);

  printf(" DATA\n");
  printf("  accesses:  %d\n", cache_stat_data.accesses);
  printf("  misses:    %d\n", cache_stat_data.misses);
  if (!cache_stat_data.accesses)
    printf("  miss rate: 0 (0)\n"); 
  else
    printf("  miss rate: %2.4f (hit rate %2.4f)\n", 
	 (float)cache_stat_data.misses / (float)cache_stat_data.accesses,
	 1.0 - (float)cache_stat_data.misses / (float)cache_stat_data.accesses);
  printf("  replace:   %d\n", cache_stat_data.replacements);

  printf(" TRAFFIC (in words)\n");
  printf("  demand fetch:  %d\n", cache_stat_inst.demand_fetches + 
	 cache_stat_data.demand_fetches);
  printf("  copies back:   %d\n", cache_stat_inst.copies_back +
	 cache_stat_data.copies_back);
}
/************************************************************/
