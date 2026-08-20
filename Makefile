MAKEFLAGS += --no-print-directory

# Defining Colors
GREEN := \033[0;32m
RED := \033[0;31m
CYAN := \033[36m
RESET := \033[0m

CC := gcc
CFLAGS := -Wall -Werror -Wextra

NAME := ft_ping

SRCS := src/main.c \
		src/init.c \
		src/send.c \
		src/receive.c \
		src/ping.c \
		src/signals.c \
		src/utils.c

OBJS := $(SRCS:.c=.o)

RM := rm -rf

all: $(NAME)

$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) -o $@ $(OBJS)
	@echo "$(GREEN)>>>>>> Built $(NAME) <<<<<<$(RESET)"

clean:
	@$(RM) $(OBJS)
	@echo "$(CYAN)>>>>>> Cleaning <<<<<<$(RESET)"

fclean: clean
	@$(RM) $(NAME)
	@echo "$(RED)>>>>>> Everything Cleaned Successfully <<<<<<$(RESET)"

re: fclean all

.PHONY: all clean fclean re
