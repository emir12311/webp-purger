#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <webp/decode.h>
#include <png.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct
{
    unsigned int force : 1;
    unsigned int include_hidden : 1;
    unsigned int verbose : 1;
} Flags;

int convert(char* path, FILE* log_ptr, int verbose);

int walk(char* path, const Flags* flags, FILE* log_ptr);

int mkdir_p(const char* full_path);

int log_init(const char* home_dir, FILE** out_log_ptr);

int log_message(FILE* log_ptr, const char* reason, const char* message, const char* path, const int err, const int verbose);