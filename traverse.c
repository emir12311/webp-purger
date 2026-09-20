#include "shared.h" // has all the general libs

int walk(char* path)
{
    DIR* dir;
    struct dirent* entry;
    struct stat st;
    char* dot_ptr;
    char path_buf[4096];
    
    dir = opendir(path);
    if (dir == NULL)
    {
        fprintf(stderr, "opendir returned NULL, bailing\n");
        return (1);
    }
    while (1)
    {
        entry = readdir(dir);
        if (entry == NULL)
            break ;
        else if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue ;
        snprintf(path_buf, sizeof path_buf, "%s/%s", path, entry->d_name);
        if (lstat(path_buf, &st) != 0)
        {
            fprintf(stderr, "lstat on %s failed\n", path_buf);
            continue ;
        }
        if (S_ISDIR(st.st_mode))
            walk(path_buf);
        else if (S_ISREG(st.st_mode))
        {
            dot_ptr = strrchr(path_buf, '.');
            if (dot_ptr == NULL)
                continue ;
            if (strcasecmp(dot_ptr + 1, "webp") != 0)
                continue ;
            convert(path_buf);
        }
    }
    closedir(dir);
    return (0);
}
