NAME = malloc_test
SRCS = test_basic.c test_edge_cases.c test_performance.c

CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -L.. -lft_malloc -lpthread

all: basic edge performance

# Test targets
basic: test_basic
	@echo "Running basic tests..."
	@env LD_LIBRARY_PATH=.. ./test_basic > log/basic.log 2>&1

edge: test_edge_cases
	@echo "Running edge case tests..."
	@env LD_LIBRARY_PATH=.. ./test_edge_cases > log/edge.log 2>&1

performance: test_performance
	@echo "Running performance tests..."
	@env LD_LIBRARY_PATH=.. ./test_performance > log/perf.log 2>&1

# Build test executables
test_basic: test_basic.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_edge_cases: test_edge_cases.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test_performance: test_performance.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f test_basic test_edge_cases test_performance

.PHONY: all basic edge performance clean
