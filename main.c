#include "shared.h"  // has all the general libs

static void	print_usage(void)
{
	fprintf
	(stderr,
	"Usage:\n"
	"webp-purger <path to directory> [OPTIONS]\n"
	"Options:\n"
	"--help                                     Print this text.\n"
	"--verbose                             Shows specific errors\n"
	"--force                Automatically continues on failures.\n"
	"--include-hidden     Includes hidden directories and files.\n"
	);
}

static int	parse_args(int argc, char *argv[], Flags *flags, char** out_dir)
{
	int	i;

	*out_dir = NULL;
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
			else if (strcmp(argv[i], "--verbose") == 0)
			{
				if (flags->verbose)
				{
					fprintf(stderr, "Duplicate option: %s\n", argv[i]);
					print_usage();
					return (1);
				}
				flags->verbose = 1;
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
			if (*out_dir != NULL)
			{
				fprintf(stderr, "Too many arguments\n");
				print_usage();
				return (1);
			}
			*out_dir = argv[i];
		}
	}
	if (*out_dir == NULL)
	{
		print_usage();
		return (1);
	}
	return (0);
}

int	get_home_dir(char** out_home_dir)
{
	char*	tmp;

	tmp = getenv("HOME");
	if (tmp == NULL)
	{
		fprintf(stderr, "Couldnt get users home directory. Bailing.");
		return (1);
	}
	*out_home_dir = tmp;
	return (0);
}

int main(int argc, char* argv[])
{
	struct stat	st;
	Flags		flags;
	FILE*		log_ptr;
	int			status;
	char*		home_dir;
	char*		dir;
	char*		real_dir;

	memset(&flags, 0, sizeof flags);
	if (get_home_dir(&home_dir))
		return (1);
	if (log_init(home_dir, &log_ptr))
		return (1);
	if (parse_args(argc, argv, &flags, &dir))
	{
		fclose(log_ptr);
		return (1);
	}
	real_dir = malloc(PATH_MAX);
	if (real_dir == NULL)
	{
		log_message(log_ptr, strerror(errno), "real_dir malloc failed", "System", 1, flags.verbose);
		fclose(log_ptr);
		return (1);
	}
	if (lstat(dir, &st) != 0)
	{
		fprintf(stderr, "The passed path is unrecognized.\n");
		free(real_dir);
		fclose(log_ptr);
		return (1);
	}
	if (realpath(dir, real_dir) == NULL)
	{
		log_message(log_ptr, strerror(errno), "realpath failed", dir, 1, flags.verbose);
		free(real_dir);
		fclose(log_ptr);
		return (1);
	}
	else if (!S_ISDIR(st.st_mode))
	{
		fprintf(stderr, "The passed path is not a directory.\n");
		free(real_dir);
		fclose(log_ptr);
		return (1);
	}
	status = walk(real_dir, &flags, log_ptr);
	if (status == 1)
	{
		log_message(log_ptr, "", "walk failed", real_dir, 1, flags.verbose);
		free(real_dir);
		fclose(log_ptr);
		return (1);
	}
	else if (status == -1)
	{
		printf("Aborting..\n");
		free(real_dir);
		fclose(log_ptr);
		return (1);
	}
	log_message(log_ptr, "", "Successfully finished.", real_dir, 0, flags.verbose);
	printf("The trashed files are in %s/.webptrashed/\n", home_dir);
	printf("The log is in %s/.log/webp-purger.log\n", home_dir);
	free(real_dir);
	fclose(log_ptr);
	return (0);
}
