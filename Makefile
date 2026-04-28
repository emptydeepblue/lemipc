# Colors
GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
RESET = \033[0m

# Project name
NAME = lemipc

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

# Directories
SRC_DIR = src
INC_DIR = inc
OBJ_DIR = obj
LIBFT_DIR = libft

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Libft
LIBFT = $(LIBFT_DIR)/libft.a

# Include paths
INC = -I$(INC_DIR) -I$(LIBFT_DIR)

# Rules
all: $(LIBFT) $(NAME)
	@echo "$(GREEN)✓ $(NAME) created$(RESET)"

$(NAME): $(OBJ)
	@$(CC) $(CFLAGS) -o $@ $^ $(LIBFT)
	@echo "$(GREEN)✓ Linking complete$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@$(CC) $(CFLAGS) $(INC) -c $< -o $@
	@echo "$(YELLOW)Compiling: $<$(RESET)"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(LIBFT):
	@$(MAKE) -C $(LIBFT_DIR)
	@$(MAKE) bonus -C $(LIBFT_DIR)

clean:
	@$(MAKE) -C $(LIBFT_DIR) clean
	@rm -rf $(OBJ_DIR)
	@echo "$(RED)✗ Object files removed$(RESET)"

fclean: clean
	@$(MAKE) -C $(LIBFT_DIR) fclean
	@rm -f $(NAME)
	@echo "$(RED)✗ $(NAME) removed$(RESET)"

re: fclean all

test: all
	@./lemipc 1 & \
	./lemipc 1 &  \
	./lemipc 2 & \
	./lemipc 2 & \
	./lemipc 1 & \
	./lemipc 3 & \
	./lemipc 3 & \
	./lemipc 3 & \
	./lemipc 4 & \
	./lemipc 4 & \
	./lemipc 2 & \
	./lemipc 4 & \
	./lemipc 2 & \
	./lemipc 1 & \
	./lemipc 2 & \
	./lemipc 2 & \
	./lemipc 1 & \
	./lemipc 3 & \
	./lemipc 3 & \
	./lemipc 3 & \
	./lemipc 4 & \
	./lemipc 4 & \
	./lemipc 1 & \
	./lemipc 4 & \
	./lemipc 3 & \
	./lemipc 3 & \
	./lemipc 3 & \
	./lemipc 4 & \
	./lemipc 4 & \
	./lemipc 2 & \
	./lemipc 4 & \
	./lemipc 2 & wait

vtest: all
	@valgrind --leak-check=full --track-origins=yes --error-exitcode=1 \
		./lemipc 1 & ./lemipc 1 & ./lemipc 2 & ./lemipc 2 & ./lemipc 1 & wait

htest: all
	@valgrind --tool=helgrind --error-exitcode=1 \
		./lemipc 1 & ./lemipc 1 & ./lemipc 2 & ./lemipc 2 & wait

.PHONY: all clean fclean re test vtest htest
