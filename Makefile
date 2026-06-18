# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: noemi <noemi@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/13 19:28:59 by noemi             #+#    #+#              #
#    Updated: 2026/06/13 19:29:00 by noemi            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= codexion

CC		= cc
CFLAGS	= -Wall -Wextra -Werror -pthread -I include

SRC_DIR	= src

SRCS	= $(SRC_DIR)/main.c \
		  $(SRC_DIR)/init.c \
		  $(SRC_DIR)/parsing_args.c \
		  $(SRC_DIR)/heap.c \
		  $(SRC_DIR)/heap_utils.c \
		  $(SRC_DIR)/utils.c \
		  $(SRC_DIR)/coder.c \
		  $(SRC_DIR)/coder_utils.c \
		  $(SRC_DIR)/monitor.c

OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
