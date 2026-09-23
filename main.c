#include "shared.h"  // has all the general libs
#include <stdio.h>

void print_usage(void)
{
    fprintf
    (stderr,
    "Usage:\n"
    "webp-purger <path to directory> [OPTIONS]\n"
    "Options:\n"
    "--help                                     Print this text.\n"
    "--force                Automatically continues on failures.\n"
    "--include-hidden     Includes hidden directories and files.\n"
    );
}

int parse_args(int argc, char **argv, Flags *flags, char **dir_out)
{
    int i;
    
    *dir_out = NULL;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "--force") == 0)
            {
                if (flags->force)
                {
                    fprintf(stderr, "Duplicate option: %s\n", argv[i]);
                    print_usage();
                    return (1);
                }
                flags->force = 1;
            }    
            else if (strcmp(argv[i], "--include-hidden") == 0)
            {
                if (flags->include_hidden)
                {
                    fprintf(stderr, "Duplicate option: %s\n", argv[i]);
                    print_usage();
                    return (1);
                }
                flags->include_hidden = 1;
            }
            else if (strcmp(argv[i], "--help") == 0)
            {
                print_usage();
                exit(0);
            }    
            else
            {
                fprintf(stderr, "Unknown option: %s\n", argv[i]);
                print_usage();
                return (1);
            }
        }
        else
        {
            if (*dir_out != NULL)
            {
                fprintf(stderr, "Too many arguments\n");
                print_usage();
                return (1);
            }
            *dir_out = argv[i];
        }
    }
    if (*dir_out == NULL)
    {
        print_usage();
        return (1);
    }
    return (0);
}

int main(int argc, char* argv[])
{
    struct stat st;
    Flags flags;
    int status;
    int error;
    char *dir;
    char *real_dir;

    memset(&flags, 0, sizeof flags);
    if (parse_args(argc, argv, &flags, &dir))
        return (1);
    real_dir = malloc(PATH_MAX); 
    if (real_dir == NULL)
    {
        fprintf(stderr, "real_dir malloc failed\n");
        return(1);
    }
    if (lstat(dir, &st) != 0)
    {
        fprintf(stderr, "The passed path is unrecognized.\n");
        return(1);
    }
    real_dir = realpath(dir, NULL);
    if (real_dir == NULL)
    {
        error = errno;
        fprintf(stderr, "The passed directory is faulty. Check it. Error: %s\n", strerror(error));
        return(1);
    }
    else if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "The passed path is not a directory.\n");
        return(1);
    }
    status = walk(real_dir, &flags);
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