#include <webp/decode.h>
#include <png.h>

#include "shared.h" // has all the general libs

typedef struct
{
	uint8_t*	data;
	size_t	data_size;
	int width;
	int height;
	uint8_t* rgba_ptr;
} buffer;

int	get_raw_bytes(char* path, buffer* buf)
{
	FILE*	f;
	size_t	got;

	f = fopen(path, "rb");
	if (f == NULL)
	{
		fprintf(stderr, "fopen returned null.\n");
		return (1);
	}
	fseek(f, 0, SEEK_END);
	buf->data_size = ftell(f);
	rewind(f);
	buf->data = malloc(buf->data_size);
	if (buf->data == NULL)
	{
		fprintf(stderr, "buf.data malloc returned null\n");
		fclose(f);
		return (1);
	}
	got = fread(buf->data, 1, buf->data_size, f);
	if (got != buf->data_size)
	{
		fprintf(stderr, "short read. bailing.\n");
		fclose(f);
		return (1);
	}
	fclose(f);
	return (0);
}

int decode_webp(buffer* buf)
{
	buf->rgba_ptr = WebPDecodeRGBA(buf->data, buf->data_size, &buf->width, &buf->height);
	if (buf->rgba_ptr == NULL)
	{
		fprintf(stderr, "WebPDecodeRGBA returned null\n");
		return (1);
	}
	return (0);
}

int	raw_to_png(char* path, buffer* buf)
{
	png_image	png;
	char*	new_path;
	char*	dot_ptr;

	memset(&png, 0, sizeof png);
	png.version = PNG_IMAGE_VERSION;
	png.width = buf->width;
	png.height = buf->height;
	png.format = PNG_FORMAT_RGBA;
	new_path = malloc(strlen(path) + 1);
	if (new_path == NULL)
	{
		fprintf(stderr, "new_path malloc returned null\n");
		WebPFree(buf->rgba_ptr);
		return (1);
	}
	strcpy(new_path, path);
	dot_ptr = strrchr(new_path, '.');
	if (dot_ptr == NULL)
	{
		fprintf(stderr, "strrchr returned null.\n");
		WebPFree(buf->rgba_ptr);
		free(new_path);
		return (1);
	}
	if (strcasecmp(dot_ptr + 1, "webp") != 0)
	{
		fprintf(stderr, "strcasecmp returned non-zero. check file name.\n");
		WebPFree(buf->rgba_ptr);
		free(new_path);
		return (1);
	}
	strcpy(dot_ptr, ".png");
	if(!png_image_write_to_file(&png, new_path, 0, buf->rgba_ptr, buf->width * 4, NULL))
	{
		fprintf(stderr, "png_image_write_to_file returned zero. bailing.\n");
		WebPFree(buf->rgba_ptr);
		free(new_path);
		return (1);
	}
	WebPFree(buf->rgba_ptr);
	return (0);
}

int convert(char *path)
{
	buffer	buf;

	memset(&buf, 0, sizeof buf);
	if (!get_raw_bytes(path, &buf))
	{
		if (!decode_webp(&buf))
		{
			if (!raw_to_png(path, &buf))
			{
				free(buf.data);
				return (0);
			}
		}
	}
	free(buf.data);
	return (1);
}
