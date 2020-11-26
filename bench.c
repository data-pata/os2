#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bench.h"
#include "dlmall.h"
#include <sys/time.h>
#include <time.h>

#define MAX 4000    // data size request interval     
#define MIN 8       
#define BUFFER 100  // implicitly max alloced blocks at the same time 
#define SIZES 2000  // implicitly maximum freelist length
#define LOOP 300    // num of allocs+frees

int freq[SIZES];

int loop, min, max; char *name;
int test(int argc, char * argv[])
{
    if (argc < 1)
    {
        fprintf(stderr, "give filename");
        exit(1);
    }
    else if (argc < 4)
    {
        // printf("defaulting arguments: bench <loop [300]> (<file>)\n");
        loop = LOOP;
        min = MIN;
        max = MAX;
        sprintf(name, "%d-%d-default.dat", loop, max);
        fprintf(stderr,
                "#defaulting arguments: [filename %s] [loop: %d] [minsize: %d] [maxsize: %d] \n",
                name, loop, min, max);
    }
    else
    {
        name = argv[1];
        loop = atoi(argv[2]);
        min = atoi(argv[3]);
        max = atoi(argv[4]);
        fprintf(stderr,
                "#arguments: [filename %s] [loop: %d] [minsize: %d] [maxsize: %d] \n",
                name, loop, min, max);
    }

    // void *init =;
    // void *current;

    void *buffer[BUFFER];
    for (int i = 0; i < BUFFER; i++)
    {
        buffer[i] = NULL;
    }

    printf("arena %p.\n", arena);
    printf("flist %p.\n", flist);

    for (int i = 0; i < loop; i++)
    {
        int index = (int)rand() % ((int)BUFFER);
        // printf("index: %d\n", index);
        if (buffer[index] != NULL)
        {
            dfree(buffer[index]);
            buffer[index] = NULL;
        }
        else
        {
            size_t size = (size_t)request();
            // printf("size from request: %ld\n", size);
            int *memory;
            memory = (int *)dalloc(size);

            if (memory == NULL)
            {
                memory = dalloc(0);
                fprintf(stderr, "memory myllocation failed, last address %p\n", memory);
                return (1);
            }
            buffer[index] = memory;
            /* writing to the memory so we know it exists */
            *memory = 123;
        }
    }
    // current = flist;
    // int allocated = (int)((current - init) / (1024));
    // printf("The final top of the heap is %p.\n", flist);
    // printf("   increased by %d Ki byte\n", are);
    // printf("   length of free list is %d\n", length_of_free());

    FILE *file = fopen(name, "w");

    sizes(freq, SIZES);
    int length = length_of_free();
    fprintf(stderr, "flist number of blocks: %d\n", length);

    qsort(freq, length, sizeof(int), cmp);

    for (int i = 0; i < length; i++)
    {
        fprintf(file, "%d\n", freq[i]);
    }

    fclose(file);

    return 0;
}
int testTime()
{
    int TUSEN = 1000;
    void *buffer[TUSEN];

    for (int i = 0; i < TUSEN; i++)
    {
        buffer[i] = NULL;
    }

    clock_t start_clock = clock(), diff;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (size_t i = 0; i < TUSEN; i++)
    {
        buffer[i] = dalloc(16);
    }
    for (size_t i = 0; i < 3000; i++)
    {
        for (size_t i = 0; i < 1000; i++)
        {
            char *memory = (char *)buffer[i];
            *memory = '1';
        }
    }
    diff = clock() - start_clock;
    long msec = diff / (CLOCKS_PER_SEC / (double)1000.0);
    gettimeofday(&end, NULL);
    long seconds = (end.tv_sec - start.tv_sec);
    long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
    printf("Time taken %ld seconds %ld milliseconds\n", msec / 1000, msec % 1000000);
    printf("Time elapsed is %ld seconds and %ld micros\n", seconds, micros);
    return 0;
}

int main(int argc, char *argv[])
{   
    // test(argc, argv);
    testTime();
}

int request()
{
    double k = log(((double)max) / min);
    double r = ((double)(rand() % (int)(k * 10000))) / 10000;
    int size = (int)((double)max / exp(r));
    return size;
}
int length_of_free()
{
    int i = 0;
    struct head *next = flist;
    while (next != NULL)
    {
        i++;
        next = next->next;
    }
    return i;
}
void sizes(int *buffer, int max)
{
    struct head *next = flist;
    int i = 0;

    while ((next != NULL) & (i < max))
    {
        buffer[i] = next->size;
        i++;
        next = next->next;
    }
}
int cmp(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;
}