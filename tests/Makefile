NAME = malloc_test

SRCS_DIR = srcs
CC = gcc
CFLAGS = -Wall -Wextra -Werror -I../includes -g3 -ggdb -O0
LDFLAGS = -L.. -lft_malloc -lpthread

all: libft_malloc basic edge performance thread absurd advanced gnl

libft_malloc:
	$(MAKE) -C .. # Build the malloc library in the parent directory

# Test targets
basic: test_basic
	@echo "Running basic tests..."
	@env LD_LIBRARY_PATH=.. ./test_basic

edge: test_edge_cases
	@echo "Running edge case tests..."
	@env LD_LIBRARY_PATH=.. ./test_edge_cases

performance: test_performance
	@echo "Running performance tests..."
	@env LD_LIBRARY_PATH=.. ./test_performance

thread: test_thread
	@echo "Running thread safety tests..."
	@env LD_LIBRARY_PATH=.. ./test_thread

absurd: test_absurd
	@echo "Running absurd test cases..."
	@env LD_LIBRARY_PATH=.. ./test_absurd

advanced: test_advanced
	@echo "Running advanced tests..."
	@env LD_LIBRARY_PATH=.. ./test_advanced

gnl: test_gnl
	@echo "Running GNL test..."
	@env LD_LIBRARY_PATH=.. ./test_gnl $(SRCS_DIR)/gnl/

# Build test executables
test_basic: $(SRCS_DIR)/basic.c $(SRCS_DIR)/printf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_edge_cases: $(SRCS_DIR)/edge_cases.c $(SRCS_DIR)/printf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_performance: $(SRCS_DIR)/performance.c $(SRCS_DIR)/printf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_thread: $(SRCS_DIR)/thread.c $(SRCS_DIR)/printf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_absurd: $(SRCS_DIR)/absurd.c $(SRCS_DIR)/printf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_advanced: $(SRCS_DIR)/advanced.c $(SRCS_DIR)/printf.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_gnl: $(SRCS_DIR)/gnl/gnl.c $(SRCS_DIR)/gnl/main.c $(SRCS_DIR)/printf.c
	$(CC) $(CFLAGS) -I$(SRCS_DIR)/gnl -o $@ $^ $(LDFLAGS)

clean:
	rm -f test_basic test_edge_cases test_performance test_thread test_absurd test_advanced test_gnl
	$(MAKE) -C .. clean # Clean the malloc library as well

.PHONY: all basic edge performance thread absurd advanced gnl clean libft_malloc
