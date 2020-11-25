#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>
#include "dlmall.h"

#define TRUE 1
#define FALSE 0

// #define TAKEN (sizeof(struct taken))
#define HEAD (sizeof(struct taken))
#define MIN(size) (((size) > (16)) ? (size) : (16))
#define LIMIT(size) (MIN(0) + HEAD + size)

#define MAGIC(memory) ((struct taken *)memory - 1)
#define HIDE(block) (void *)((struct taken *)block + 1)
#define ALIGN 8
#define ARENA (64 * 1024)

struct head *arena = NULL;
struct head *flist = NULL;

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
    // else flist = flist->next;    
    if(block == flist)
        flist = flist->next;
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

// dalloc -> find a block in flist or NULL if none are found: 
// a block of at least MIN(size) aligned, detach and return it
// if at least LIMIT(size) aligned => split and return the right (higher adress) part
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
          after(splt)->bfree=FALSE; // is this ok in c??
          splt->free=FALSE;
          return splt;
    }    
      // else return block if big enough
    else if(block->size >= size) {
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
    printf("taken   %p  \n ", taken);
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
        block = merge(block);   
        block->free = TRUE;
        after(block)->bfree= TRUE; // dangerous if aft is merged??
        insert(block);
    }
}
#define MERGE_LEFT 1

struct head *merge(struct head *block ) 
{
    struct head *aft = after(block);
    if (block->bfree) {
        struct head *bfr = before(block);
        detach(bfr);
        bfr->size += block->size + HEAD;
        aft->bsize = bfr->size;
        aft->bfree = bfr->free; // necessary only for non detach implementation prolly
        // block = merge(bfr);
        block = bfr;
    }
    if (aft->free) {
        detach(aft);
        struct head *aftaft = after(aft);
        block->size += aft->size + HEAD;
        aftaft->bsize = block->size;
        aftaft->bfree = block->free;
        // block = merge(aftaft);   //warning not working
    }
    
    return block;
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
