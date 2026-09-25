#include "shared.h" // has all the general libs

int	mkdir_p(const char* full_path)
{
	const char*	slash_ptr;
	int			i;
	int			d;
	int			n;
	char		prev_buf[PATH_MAX];
	char		aft_buf[PATH_MAX];
	char		path_buf[PATH_MAX];

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
		n = snprintf(path_buf, sizeof path_buf, "%s%s", prev_buf, aft_buf);
		if (n < 0 || (size_t)n >= sizeof path_buf)
		{
			fprintf(stderr, "Couldnt make directory, path too long.");
			return (1);
		}
		if (mkdir(path_buf, 0755) != 0)
		{
			if (errno != EEXIST)
			{
				if (n < 0 || (size_t)n >= sizeof path_buf)
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

static int	move_to_trash(const char* file_path, const char* dir_path, FILE* log_ptr, const int verbose)
{
	char	trash_path[PATH_MAX];
	char	trash_dir_path[PATH_MAX];
	char*	home_dir;
	char	buf[65536];
	int		src_fd;
	int		dst_fd;
	ssize_t	nread;
	ssize_t	nwritten;
	size_t	offset;

	src_fd = -1;
	dst_fd = -1;
	nread = 1;
	if (get_home_dir(&home_dir))
	{
		log_message(log_ptr, "", "get_home_dir failed.", "System", 1, verbose);
		return (1);
	}
	snprintf(trash_dir_path, sizeof trash_dir_path, "%s/.webptrashed%s", home_dir, dir_path);
	if (mkdir_p(trash_dir_path))
	{
		log_message(log_ptr, "", "mkdir on path failed.", trash_dir_path, 1, verbose);
		return (1);
	}
	snprintf(trash_path, sizeof trash_path, "%s/.webptrashed%s", home_dir, file_path);
	if (rename(file_path, trash_path))
	{
		if (errno == EXDEV)
		{
			src_fd = open(file_path, O_RDONLY);
			if (src_fd == -1)
				return (1);
			dst_fd = open(trash_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (dst_fd == -1)
				goto cleanup;
			while (nread != 0)
			{
				nread = read(src_fd, buf, sizeof buf);
				if (nread == -1)
					goto cleanup;
				offset = 0;
				while (offset < (size_t)nread)
				{
					nwritten = write(dst_fd, buf + offset, (size_t)nread - offset);
					if (nwritten == -1)
						goto cleanup;
					offset += (size_t)nwritten;
				}
			}
			if (fsync(dst_fd) == -1)
				goto cleanup;
			if (close(dst_fd) == -1)
			{
				dst_fd = -1;
				goto cleanup;
			}
			dst_fd = -1;
			if (close(src_fd) == -1)
			{
				src_fd = -1;
				return (1);
			}
			src_fd = -1;
			if (unlink(file_path) == -1)
			{
				log_message(log_ptr, "", "unlink on path failed", file_path, 1, verbose);
				return (1);
			}
		}
		else
		{
			log_message(log_ptr, "", "rename on file failed", trash_path, 1, verbose);
			return (1);
		}
	}
	log_message(log_ptr, "", "Successfully moved file to trash.", trash_path, 0, verbose);
	return (0);
cleanup:
	if (dst_fd != -1)
		close(dst_fd);
	if (src_fd != -1)
		close(src_fd);
	return (1);
}

static void	clean_stdin(void)
{
	int	c;
	while ((c = getchar()) != '\n' && c != EOF)
		;
}

int walk(const char* path, const Flags* flags, FILE* log_ptr)
{
	struct dirent*	entry;
	struct stat		st;
	DIR*			dir;
	char*			dot_ptr;
	char			path_buf[4096];
	char			buf[16];
	
	dir = opendir(path);
	if (dir == NULL)
	{
		log_message(log_ptr, "opendir returned null.", "Couldnt open directory", path, 1, flags->verbose);
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
			log_message(log_ptr, "", "lstat on file failed", path_buf, 1, flags->verbose);
			continue ;
		}
		if (S_ISDIR(st.st_mode))
		{
			if (strcmp(entry->d_name, ".webptrashed") != 0 && ((entry->d_name[0] == '.' && flags->include_hidden) || entry->d_name[0] != '.'))
			{
				if (walk(path_buf, flags, log_ptr) == -1)
				{
					closedir(dir);
					return (-1);
				}
			}
			else
				continue ;
		}
		else if (S_ISREG(st.st_mode))
		{
			if ((entry->d_name[0] == '.' && flags->include_hidden) || entry->d_name[0] != '.')
			{
				dot_ptr = strrchr(entry->d_name, '.');
				if (dot_ptr == NULL)
					continue ;
				if (strcasecmp(dot_ptr + 1, "webp") != 0)
					continue ;
				log_message(log_ptr, "", "Converting..", path_buf, 0, flags->verbose);
				if (convert(path_buf, log_ptr, flags->verbose))
				{
					log_message(log_ptr, "", "Failed to convert file", path_buf, 1, flags->verbose);
					if (flags->force)
						continue ;
					printf("Continue? [y/N] ");
					fflush(stdout);
					if (fgets(buf, sizeof buf, stdin) == NULL)
					{
						log_message(log_ptr, "", "fgets returned null", path_buf, 1, flags->verbose);
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
				if (move_to_trash(path_buf, path, log_ptr, flags->verbose))
					log_message(log_ptr, "", "Failed to move the file to trash.", path_buf, 1, flags->verbose);
				else
					log_message(log_ptr, "", "Successfully finished conversion.", path_buf, 0, flags->verbose);
			}
			else
				continue ;
		}
	}
	closedir(dir);
	return (0);
}
