#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct
{
    unsigned int noconfirm : 1;
} Flags;

int convert(char* path);

int walk(char* path, Flags* flags);