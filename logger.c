#include "shared.h"

int log_init(const char* home_dir, FILE** out_log_ptr)
{
    FILE* log_ptr;
    char log_folder[PATH_MAX];
    char log_file_path[PATH_MAX];

    snprintf(log_folder, sizeof log_folder, "%s/%s", home_dir, ".log/");
    if (mkdir_p(log_folder))
    {
        fprintf(stderr, "Couldnt create log folder.\n");
        return (1);
    }
    snprintf(log_file_path, sizeof log_file_path, "%s/%s", log_folder, "webp-purger.log");
    log_ptr = fopen(log_file_path, "a");
    if (log_ptr == NULL)
    {
        fprintf(stderr, "fopen on log file returned null. Bailing.\n");
        return (1);
    }
    *out_log_ptr = log_ptr;
    return (0); 
}

static int calculate_date_time(char* out_data_time, const size_t buf_size)
{
    time_t now;
    struct tm* tm;

    now = time(NULL);
    tm = localtime(&now);
    if (tm == NULL)
    {
        snprintf(out_data_time, buf_size, "---------- --:--:--");
        return (1);
    }
    strftime(out_data_time, buf_size, "%Y-%m-%d %H:%M:%S", tm);
    return (0);
}

int log_message(FILE* log_ptr, const char* reason, const char* message, const char* path, const int err, const int verbose)
{
    char buf[32];

    calculate_date_time(buf, sizeof buf);
    if (strcmp(reason, "") != 0)    
        fprintf(log_ptr, "[%s] %s: %s, %s\n", buf, path, message, reason);
    else
        fprintf(log_ptr, "[%s] %s: %s\n", buf, path, message);
    if (err == 0)
        fprintf(stdout, "%s: %s\n", path, message);
    else if (verbose)
        if (strcmp(reason, "") != 0)    
            fprintf(stderr, "%s: %s, %s\n", path, message, reason);
        else
            fprintf(stderr, "%s: %s\n", path, message);
    else
        fprintf(stderr, "%s: %s\n", path, message);
    return (0);
}