#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>
#include "dlmall.h"

#define TRUE 1
#define FALSE 0

#define HEAD (sizeof(struct head))
#define MIN(size) (((size) > (8)) ? (size) : (8))
#define LIMIT(size) (MIN(0) + HEAD + size)

#define MAGIC(memory) ((struct head *)memory - 1)
#define HIDE(block) (void *)((struct head *)block + 1)
#define ALIGN 8
#define ARENA (64 * 1024)

struct head *arena = NULL;
struct head *flist;

static void init() __attribute__((constructor));
void init() {
    arena = new();
    insert(arena);
} 

struct head *after(struct head *block)
{
    // return (struct head *)block + block->size + HEAD;
    // return (struct head *) HIDE(block) + block->size + HEAD;
    return (struct head *) (((char *)block) + block->size + HEAD);
}

struct head *before(struct head *block) 
{
    return (struct head*) (((char *)block) - block->bsize - HEAD);
}
// 
struct head *split(struct head *block, int size) 
{
    // if ( ! block->size > LIMIT(size)) return null;
    int rsize = block->size - size - HEAD; //maybe
    block->size = rsize;

    struct head *splt  = after(block);
    splt->bsize = block->size; 
    splt->bfree = block->free; 
    splt->size = size; // OR?
    splt->free = TRUE; //FALSE?

    struct head *aft = after(splt);
    aft->bsize = splt->size;
    
    return splt;
}

// hämta nytt block / arena
struct head *new() { 

    if(arena != NULL) {
        printf("one arena already allocated \n");
        return NULL;
    }

    struct head *new = (struct head *) mmap(NULL, ARENA, PROT_READ|PROT_WRITE, 
        MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    if(new == MAP_FAILED) {
        printf("mmap failed: error %d\n", errno);
        return NULL;
    }

    unsigned int size = ARENA - 2*HEAD; //VARFÖR TVÅ HEADS?
    new->bfree = FALSE;
    new->bsize = 0; // or 0?
    new->free = TRUE;
    new->size = size;

    struct head *sentinel = after(new);
    sentinel->bfree = TRUE;
    sentinel->bsize = size;
    sentinel->free = FALSE;
    sentinel->size = 0; // or 0 ?

    arena = (struct head*) new; // why the cast??
}

void detach (struct head *block) 
{
    if (block->next != NULL)
        block->next->prev = block->prev;
    if (block->prev != NULL)    
        block->prev->next = block->next;
    
    else  // WHATWHAT
        flist = flist->next;    
        
    // block->next = NULL; //ELLER ?!?!?
    // block->prev = NULL;
}

void insert (struct head *block) 
{
    block->next = flist;
    block->prev = NULL;
    
    if (flist != NULL) 
        flist->prev = block;
    flist = block;
}

int adjust(size_t request) 
{
    int size = MIN(request);
    size = size + (8-(size % ALIGN)); 
    return size;
}
     
struct head *find(size_t size) 
{
    if(flist == NULL)
        return NULL; 

    for(struct head *block=flist; block!=NULL; block=block->next) {
        // split and return split
      if(block->size >= LIMIT(size)) {
          detach(block);
          struct head * splt = split(block, size);
          insert(before(splt));
          after(splt)->bfree=FALSE;
          splt->free=FALSE;
          return splt;
      }    
      // else return block if big enough
      if(block->size >= size) {
        detach(block);
        after(block)->bfree = FALSE;
        block->free = FALSE;
        return block;
      }
    }
    return NULL;
}
// test!!!
void *dalloc(size_t request) 
{
    if(request <= 0) {
        return NULL; // ?
    }
    int size = adjust(request);
    // printf("size adjusted to: %d\n", size);
    struct head *taken = find(size);
    printf("%p\n", taken);
    if (taken == NULL)
        return NULL;
    else 
        return HIDE(taken);
}

// test!!!
void dfree(void *memory) {
    if(memory != NULL) {
        struct head *block = (struct head *) MAGIC(memory);
        
        struct head *aft = after(block);
        block->free = TRUE;
        aft->bfree= TRUE;
        insert(block);
    }
}

// int main(int argc, char const *argv[])
// {   
//     struct head *arena = new();
//     printf("ARENA: bfree %d | bsize %d | free %d | size %d \n",
//         arena->bfree, arena->bsize, arena->free, arena->size);

//     struct head *splt = split(arena, 20000);
//     if(splt)
//         printf("SPLIT: bfree %d | bsize %d | free %d | size %d \n",
//             splt->bfree, splt->bsize, splt->free, splt->size);

//     printf("hello");
//     return 0;
// }
