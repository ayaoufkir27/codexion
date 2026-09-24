NAME = codexion
CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
INCLUDES = includes/codexion.h

SRCS = srcs/dongles.c srcs/heap.c srcs/init.c srcs/main.c srcs/monitor.c srcs/parse.c srcs/routine.c srcs/utils.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
