# Check for HOSTTYPE environment variable
ifeq ($(HOSTTYPE),)
HOSTTYPE := $(shell uname -m)_$(shell uname -s)
endif

# Library names
NAME = libft_malloc_$(HOSTTYPE).so
LINK_NAME = libft_malloc.so

# Directories
SRCS_DIR = srcs
INCS_DIR = includes
OBJS_DIR = objs

# Source files
SRCS = $(SRCS_DIR)/calloc.c \
	$(SRCS_DIR)/free.c \
	$(SRCS_DIR)/log.c \
	$(SRCS_DIR)/malloc.c \
	$(SRCS_DIR)/realloc.c \
	$(SRCS_DIR)/show.c \
	$(SRCS_DIR)/internal/block.c \
	$(SRCS_DIR)/internal/defrag.c \
	$(SRCS_DIR)/internal/system.c \
	$(SRCS_DIR)/internal/validation.c \
	$(SRCS_DIR)/internal/zone.c

# Object files
OBJS = $(SRCS:$(SRCS_DIR)/%.c=$(OBJS_DIR)/%.o)

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -fPIC -g3 -ggdb -O0
INCLUDES = -I$(INCS_DIR)
LDFLAGS = -shared

# Command variables
RM = rm -f
MKDIR = mkdir -p

# Default target
all: $(NAME)

# Create the symbolic link
$(LINK_NAME): $(NAME)
	@echo "Creating symbolic link: $(LINK_NAME) -> $(NAME)"
	@ln -sf $(NAME) $(LINK_NAME)

# Build the shared library
$(NAME): $(OBJS)
	@echo "Building library: $(NAME)"
	@$(CC) $(LDFLAGS) -o $@ $^
	@echo "Creating symbolic link"
	@ln -sf $(NAME) $(LINK_NAME)

# Compile source files
$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@echo "Compiling: $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Create objects directory
$(OBJS_DIR):
	@echo "Creating directory: $(OBJS_DIR)"
	@$(MKDIR) $(OBJS_DIR)

# Clean object files
clean:
	@echo "Cleaning object files"
	@$(RM) -r $(OBJS_DIR)

# Clean everything
fclean: clean
	@echo "Cleaning libraries"
	@$(RM) $(NAME) $(LINK_NAME)

# Test suite
test:
	@echo "Building with DEBUG_MALLOC=1 for testing"
	@$(MAKE) CFLAGS="$(CFLAGS) -D DEBUG_MALLOC=1" all
	@echo "Running test suite"
	@cd tests && $(MAKE) && $(MAKE) clean && cd ..
	@$(MAKE) fclean

# Rebuild
re: fclean all

.PHONY: all clean fclean re test
