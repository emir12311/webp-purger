SRC = main.c convert.c traverse.c logger.c

NAME = webp-purger

CC = clang

CFLAGS = -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror -Wformat=2 -Wcast-qual -Wmissing-prototypes -Wmissing-declarations -Wstrict-prototypes

OBJ = main.o convert.o traverse.o logger.o

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -lwebp -lpng -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean
	$(MAKE) $(NAME)

.PHONY: all clean fclean re