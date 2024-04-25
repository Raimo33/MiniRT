# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: craimond <bomboclat@bidol.juis>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/03/19 14:52:58 by egualand          #+#    #+#              #
#    Updated: 2024/04/25 17:18:37 by craimond         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

#TODO fixare errori quando si fa fclean 2 volte di fila

MLX_DIR = minilibx-linux
MLX_REPO = https://github.com/42Paris/minilibx-linux.git

LIBFT_DIR = libft
LIBFT_REPO = https://github.com/Raimo33/Libft.git

UTILS = $(addprefix utils/, mlx_utils.c protected_methods.c singleton.c conditionals.c ft_strtok.c ft_atof.c ft_atoui.c ft_freematrix.c get_next_line.c math_utils.c)
SRCS = $(addprefix srcs/, minirt.c init.c init2.c exit.c parse.c parse2.c parse3.c parse4.c setup_scene.c render.c render_light.c render_uv_maps.c render_utils.c intersections.c intersections2.c intersections3.c vector_operations.c $(UTILS))

OBJS = $(SRCS:.c=.o)

INCLUDES = .
HEADERS = $(addprefix srcs/headers/, minirt.h get_next_line.h scene.h shapes.h primitives.h)

RED = \033[0;31m
GREEN = \033[0;32m
NC = \033[0m

CC = cc
CFLAGS = -g -Wall -Wextra -Werror
RM = rm -rf

NAME = minirt

all: $(MLX_DIR) $(LIBFT_DIR) $(NAME)

$(MLX_DIR):
		@echo "cloning minilibx-linux"
		@git clone $(MLX_REPO) $(MLX_DIR) 2> /dev/null
		@echo "configuring minilibx-linux"
		@cd $(MLX_DIR) && ./configure 2> /dev/null > /dev/null
		@echo "$(GREEN)successfully configured minilibx-linux$(NC)"

$(LIBFT_DIR):
		@echo "cloning libft"
		@git clone $(LIBFT_REPO) $(LIBFT_DIR) 2> /dev/null
		@echo "configuring libft"
		@make bonus -C $(LIBFT_DIR) > /dev/null
		@echo "$(GREEN)successfully configured libft$(NC)"

$(NAME): $(MLX_DIR) $(LIBFT_DIR) $(OBJS)
		@$(CC) $(CFLAGS) $(OBJS) -L$(MLX_DIR) -L$(LIBFT_DIR) -lft -lmlx_Linux -pthread -I$(MLX_DIR) -I$(LIBFT_DIR) -lXext -lX11 -lm -lz -O3 -o $(NAME)
		@echo "$(GREEN)compiled $(NAME)$(NC)"

leaks: all
		@valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) scenes/italian_flag.rt 2> leaks.log

%.o: %.c $(HEADERS)
		@echo -n "compiling " && echo $< | rev | cut -d'/' -f1 | rev
		@$(CC) $(CFLAGS) -I $(INCLUDES) -c $< -o $@

clean:
		@$(RM) $(OBJS)
		@make -s clean -C $(LIBFT_DIR) > /dev/null 2>&1 || true
		@echo "$(RED)cleaned libft$(NC)"
		@echo "$(RED)cleaned $(NAME)$(NC)"

fclean: clean
		@echo "removing $(NAME)"
		@$(RM) $(NAME)
		@echo "removing minilibx_linux"
		@$(RM) $(MLX_DIR)
		@echo "removing libft"
		@$(RM) $(LIBFT_DIR)
		@echo "$(RED)removed everything$(NC)"

re: fclean all

.PHONY: all clean fclean re