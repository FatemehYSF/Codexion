NAME        = codexion

CC            = cc
CFLAGS        = -Wall -Wextra -Werror -pthread



SRCS        = main.c \
              args.c \
              init.c \
              coder.c \
              monitor.c \
              dongle.c \
              scheduler.c \
              scheduler_cmp.c \
              utils.c \
              log.c \
              time.c

OBJS        = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re