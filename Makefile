SRC = src/main.c src/convert.c src/traverse.c src/logger.c

NAME = webp-purger

TARGET = dist/$(NAME)

CC = clang

CFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -Wformat=2 -Wcast-qual -Wmissing-prototypes -Wmissing-declarations -Wstrict-prototypes

OBJ = $(SRC:src/%.c=build/%.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p dist
	$(CC) $(CFLAGS) $(OBJ) -lwebp -lpng -o $@

build/%.o: src/%.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)
	rm -rf build

fclean: clean
	rm -f $(TARGET)
	rm -rf dist

re: fclean
	$(MAKE) $(TARGET)

.PHONY: all clean fclean re