#include "shared.h"  // has all the general libs

void print_usage(void)
{
    fprintf
    (stderr,
    "Usage:\n"
    "webp-purger <path to directory> [OPTIONS]\n"
    "Options:\n"
    "--force              Automatically continues on failures.\n"
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
    char *dir;

    memset(&flags, 0, sizeof flags);
    if (parse_args(argc, argv, &flags, &dir))
        return (1);
    
    if (lstat(dir, &st) != 0)
    {
        fprintf(stderr, "The passed path is unrecognized.\n");
        return(1);
    }
    else if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "The passed path is not a directory.\n");
        return(1);
    }
    status = walk(dir, &flags);
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