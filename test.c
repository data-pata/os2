#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "dlmall.h"

int main(int argc, char const *argv[])
{
    printf("running test test.c \n");
    struct head *alloced = (struct head *) dalloc(2000);
    dfree(alloced);
    
}   
