#include "shared.h" // has all the general libs

int mkdir_p(char* full_path)
{
    char* slash_ptr;
    int i;
    int d;
    char prev_buf[PATH_MAX];
    char aft_buf[PATH_MAX];
    char path_buf[PATH_MAX];

    memset(&prev_buf, 0, sizeof prev_buf);
    memset(&aft_buf, 0, sizeof aft_buf);
    memset(&path_buf, 0, sizeof path_buf);
    slash_ptr = &full_path[0];
    if (slash_ptr[0] == '/')
    {
        if (mkdir("/", 0755) != 0)
        {
            if (errno != EEXIST)
            {
                fprintf(stderr, "Couldnt make directory. Check permissions");
                return (1);
            }
        }
        strcpy(prev_buf, "/");
    }
    slash_ptr++;
    i = 0;
    while (strchr(slash_ptr + i, '/') != NULL)
    {
        d = 0;
        for (; slash_ptr[i] != '/'; i++, d++)
            aft_buf[d] = slash_ptr[i];
        snprintf(path_buf, sizeof path_buf, "%s%s", prev_buf, aft_buf);
        fprintf(stderr, "mkdir: %s\n", path_buf);
        if (mkdir(path_buf, 0755) != 0)
        {
            if (errno != EEXIST)
            {
                fprintf(stderr, "Couldnt make directory. Check permissions\n");
                return (1);
            }
        }
        i++;
        path_buf[i] = '/';
        strcpy(prev_buf, path_buf);
        memset(&aft_buf, 0, sizeof aft_buf);
    }
    if (mkdir(full_path, 0755) != 0)
    {
        if (errno != EEXIST)
        {
            fprintf(stderr, "Couldnt make directory. Check permissions");
            return (1);
        }
    }
    return (0);
}

void clean_stdin(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int walk(char* path, Flags* flags)
{
    DIR* dir;
    struct dirent* entry;
    struct stat st;
    char* dot_ptr;
    char png_path_buf[4096];
    char path_buf[4096];
    char    buf[16];
    
    dir = opendir(path);
    if (dir == NULL)
    {
        fprintf(stderr, "%s:\nopendir returned NULL, bailing\n", path);
        return (1);
    }
    while (1)
    {
        entry = readdir(dir);
        if (entry == NULL)
            break ;
        else if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue ;
        if (lstat(path_buf, &st) != 0)
        {
            fprintf(stderr, "lstat on %s failed\n", path_buf);
            continue ;
        }
        if (S_ISDIR(st.st_mode))
        {    
            if ((entry->d_name[0] == '.' && flags->include_hidden) || entry->d_name[0] != '.')
            {
                if (walk(path_buf, flags) == -1)
                {
                    closedir(dir);
                    return (-1);
                }       
            } 
            else
                continue; 
        }
        else if (S_ISREG(st.st_mode))
        {
            if ((entry->d_name[0] == '.' && flags->include_hidden) || entry->d_name[0] != '.')
            {
                strcpy(png_path_buf, entry->d_name);
                dot_ptr = strrchr(entry->d_name, '.');
                if (dot_ptr == NULL)
                    continue ;
                if (strcasecmp(dot_ptr + 1, "webp") != 0)
                    continue ;
                if (convert(path_buf))
                {
                    fprintf(stderr, "Failed to convert %s\n", path_buf);
                    if (flags->force)
                        continue;
                    printf("Continue? [y/N] ");
                    fflush(stdout);
                    if (fgets(buf, sizeof buf, stdin) == NULL)
                    {
                        fprintf(stderr, "%s:\nfgets returned null.\n", path_buf);
                        closedir(dir);
                        return (-1);
                    }
                    if (strchr(buf, '\n') == NULL)
                        clean_stdin();
                    if (buf[0] == 'y' || buf[0] == 'Y')
                        continue ;
                    else
                    {
                        closedir(dir);
                        return (-1);
                    }
                }
                
            }
            else
                continue;
        }
    }
    closedir(dir);
    return (0);
}
