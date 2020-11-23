#ifndef __DLMALL_H
#define __DLMALL_H
#include <stdint.h>

extern struct head *arena;
extern struct head *flist;

struct head
{
    uint16_t bfree;    // 2 bytes , the status of block before
    uint16_t bsize;    // 2 b yt e s , th e s i z e of b l o c k b e f o r e
    uint16_t free;     // 2 b yt e s , th e s t a t u s of th e b l o c k
    uint16_t size;     // 2 b yt e s , th e s i z e (max 2^16 i . e . 64 Ki b yt e )
    struct head *next; // 8 b yt e s p o int e r
    struct head *prev; //  8 b yt e s p o int e r
};

void *dalloc(size_t request); 
void dfree();

struct head *after(struct head *block);
struct head *before(struct head *block);
struct head *split(struct head *block, int size);
struct head *new();
void detach (struct head *block);
void insert(struct head *block);
struct head *find(size_t size);
int adjust(size_t size);
#endif 
