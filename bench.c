// #include "dlmall.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dlmall.h"

#define MAX 4000
#define MIN 8
#define BUFFER 100
#define SIZES 2000

int request() 
{    
    double k = log(((double)MAX) / MIN);

    double r = ((double)(rand() % (int)(k * 10000))) / 10000;

    int size = (int) ((double)MAX / exp(r)); 

    return size;
}

int freq[SIZES];

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

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        printf("usage: bench <rounds> <loop> (<file>)\n");
        exit(1);
    }
    
    int rounds = atoi(argv[1]);
    int loop = atoi(argv[2]);
    char *name = argv[3];

    // void *init =;
    // void *current;

    void *buffer[BUFFER];
    for (int i = 0; i < BUFFER; i++)
    {
        buffer[i] = NULL;
    }

    printf("arena %p.\n", arena);
    printf("flist %p.\n", flist);

    for (int j = 0; j < rounds; j++)
    {
        for (int i = 0; i < loop; i++)
        {
            int index = rand() % BUFFER;
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
                memory = (int *) dalloc(size);

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
    }

    FILE *file = fopen("sizefile", "w");

    sizes(freq, SIZES);
    int length = length_of_free();
    printf("flist number of blocks: %d\n", length);

    qsort(freq, length, sizeof(int), cmp);

    for (int i = 0; i < length; i++)
    {
        fprintf(file, "%d\n", freq[i]);
    }

    fclose(file);

    return 0;
}
