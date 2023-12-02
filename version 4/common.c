#include<stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "common.h"

void error(char * msg)
{
    printf("%s\n",msg);
}

int filesize(char *filename)
{
    int o_bytes;
    struct stat st;
    int n;
    n=stat(filename, &st);
    o_bytes= st.st_size;
    return o_bytes;
}