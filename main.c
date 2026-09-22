#include "shared.h"  // has all the general libs

int main(int argc, char* argv[])
{
    struct stat st;
    int status;

    if (argc != 2)
    {
        fprintf(stderr, "Usage:\nwebp-purger <path to directory>\n");
        return (1);
    }
    else if (lstat(argv[1], &st) != 0)
    {
        fprintf(stderr, "The passed path is unrecognized.\n");
        return(1);
    }
    else if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "The passed path is not a directory.\n");
        return(1);
    }
    status = walk(argv[1]);
    if (status == 1)
    {
        fprintf(stderr, "The path walk failed.\n");
        return(1);
    }
    else if (status == -1)
    {
        printf("Aborting..\n");
        return(1);
    }
    return (0);
}