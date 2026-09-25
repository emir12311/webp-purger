#include "shared.h" // has all the general libs

typedef struct
{
	uint8_t*	data;
	size_t		data_size;
	int			width;
	int			height;
	uint8_t*	rgba_ptr;
} buffer;

static int	get_raw_bytes(char* path, buffer* buf, FILE* log_ptr, int verbose)
{
	FILE*	f;
	size_t	got;
	long	size;

	f = fopen(path, "rb");
	if (f == NULL)
	{
		log_message(log_ptr, strerror(errno), "fopen failed", path, 1, verbose);
		return (1);
	}
	if (fseek(f, 0, SEEK_END) != 0)
	{
		log_message(log_ptr, strerror(errno), "fseek failed", path, 1, verbose);
		fclose(f);
		return (1);
	}
	size = ftell(f);
	if (size < 0)
	{
		log_message(log_ptr, strerror(errno), "ftell failed", path, 1, verbose);
		fclose(f);
		return (1);
	}
	buf->data_size = (size_t)size;
	rewind(f);
	buf->data = malloc(buf->data_size);
	if (buf->data == NULL)
	{
		log_message(log_ptr, "", "malloc failed", path, 1, verbose);
		fclose(f);
		return (1);
	}
	got = fread(buf->data, 1, buf->data_size, f);
	if (got != buf->data_size)
	{
		log_message(log_ptr, "short read", "read failed", path, 1, verbose);
		fclose(f);
		return (1);
	}
	fclose(f);
	return (0);
}

static int	check_webp_header(buffer* buf, FILE* log_ptr, const char* path, int verbose)
{
	if (buf->data_size < 12)
	{
		log_message(log_ptr, "size < 12", "invalid webp", path, 1, verbose);
		return (1);
	}
	else if (memcmp(buf->data, "RIFF", 4) != 0)
	{
		log_message(log_ptr, "missing RIFF header", "invalid webp", path, 1, verbose);
		return (1);
	}
	else if (memcmp(buf->data + 8, "WEBP", 4) != 0)
	{
		log_message(log_ptr, "missing WEBP signature", "invalid webp", path, 1, verbose);
		return (1);
	}
	return (0);
}

static int	decode_webp(buffer* buf, FILE* log_ptr, const char* path, int verbose)
{
	buf->rgba_ptr = WebPDecodeRGBA(buf->data, buf->data_size, &buf->width, &buf->height);
	if (buf->rgba_ptr == NULL)
	{
		log_message(log_ptr, "WebPDecodeRGBA returned NULL", "decode failed", path, 1, verbose);
		return (1);
	}
	return (0);
}

static int	raw_to_png(const char* path, buffer* buf, FILE* log_ptr, int verbose)
{
	png_image	png;
	char*		new_path;
	char*		dot_ptr;

	memset(&png, 0, sizeof png);
	png.version = PNG_IMAGE_VERSION;
	png.width = (png_uint_32)buf->width;
	png.height = (png_uint_32)buf->height;
	png.format = PNG_FORMAT_RGBA;
	new_path = malloc(strlen(path) + 1);
	if (new_path == NULL)
	{
		log_message(log_ptr, "", "malloc failed for output path", path, 1, verbose);
		WebPFree(buf->rgba_ptr);
		return (1);
	}
	strcpy(new_path, path);
	dot_ptr = strrchr(new_path, '.');
	strcpy(dot_ptr, ".png");
	if (!png_image_write_to_file(&png, new_path, 0, buf->rgba_ptr, buf->width * 4, NULL))
	{
		log_message(log_ptr, "png_image_write_to_file returned 0", "png write failed", path, 1, verbose);
		WebPFree(buf->rgba_ptr);
		free(new_path);
		return (1);
	}
	free(new_path);
	WebPFree(buf->rgba_ptr);
	return (0);
}

int	convert(char *path, FILE* log_ptr, int verbose)
{
	buffer	buf;

	memset(&buf, 0, sizeof buf);
	if (get_raw_bytes(path, &buf, log_ptr, verbose)
		|| check_webp_header(&buf, log_ptr, path, verbose)
		|| decode_webp(&buf, log_ptr, path, verbose)
		|| raw_to_png(path, &buf, log_ptr, verbose))
	{
		free(buf.data);
		return (1);
	}
	free(buf.data);
	return (0);
}
