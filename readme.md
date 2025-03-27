# malloc - Custom Memory Allocator

A robust, thread-safe implementation of dynamic memory allocation functions for UNIX systems, implementing the standard malloc interface with additional features for memory management and debugging.

## Project Overview

This project implements the following memory allocation functions:
- **malloc**: Allocate memory of specified size
- **free**: Deallocate previously allocated memory
- **realloc**: Resize allocated memory
- **calloc**: Allocate and zero-initialize memory
- **show_alloc_mem**: Basic allocation visualization
- **show_alloc_mem_ex**: Detailed memory state with hex dumps

### API Compatibility with System malloc
This custom malloc implementation is fully compatible with the standard C library malloc. It follows the same function signatures and behavior as defined in the C standard:

```c
void	*malloc(size_t size);
void	free(void *ptr);
void	*realloc(void *ptr, size_t size);
void	*calloc(size_t nmemb, size_t size);
void	show_alloc_mem(void);
void	show_alloc_mem_ex(void);
```

Programs using standard memory allocation functions can use this implementation without any code modifications:
- Function parameters and return values match the standard malloc implementation
- Error handling follows the same conventions (returning NULL on failure)
- Memory alignment meets standard requirements
- Thread safety is maintained as in the system implementation
- All edge cases (NULL pointers, zero sizes) are handled correctly
- Simply use one of the linking methods described in the "Using the Library" section to replace the system malloc with this implementation in your programs.

Key features include:
- Zone-based allocation strategy for efficient memory management
- Thread-safe implementation using POSIX mutexes
- Memory visualization tools
- Memory defragmentation
- Minimal system calls for performance optimization

## Implementation Details

### Memory Organization

Memory is organized into three zone types:
- **TINY**: For allocations from 1 to 128 bytes
- **SMALL**: For allocations from 129 to 1024 bytes
- **LARGE**: For allocations larger than 1024 bytes

Zones are pre-allocated to minimize system calls, with each zone containing at least 100 allocations.

### Allocation Strategy

1. Memory is mapped using `mmap` at program initialization
2. Zones are divided into blocks with metadata headers
3. First-fit algorithm is used to find available blocks
4. Blocks are split when significantly larger than requested size
5. Adjacent free blocks are merged during defragmentation

### Thread Safety

Thread safety is ensured using a global mutex (`g_malloc_mutex`) that protects all critical sections in the allocation and freeing operations.

## How the Code Works

### Memory Block Structure

Each allocated block contains:
- Metadata header (size, status, magic number)
- User data area
- Proper alignment (16-byte)

### Allocation Process

1. Calculate required size including metadata and alignment
2. Find appropriate zone type (TINY, SMALL, LARGE)
3. Search for a free block of sufficient size
4. Split block if necessary
5. Mark block as allocated and return pointer to user data area

### Free and Defragmentation

1. Validate the pointer to ensure it's a proper allocation
2. Mark the block as free
3. Merge with adjacent free blocks when possible
4. Perform zone defragmentation when fragmentation exceeds threshold
5. Unmap LARGE zones when they become empty

## How to Build and Use

### Building the Library

```bash
# Clone the repository
git clone git@github.com:c-bertran/malloc.git
cd malloc

# Build the shared library
make
```

This creates a shared library file (libft_malloc_<hosttype>.so) and a symbolic link (libft_malloc.so).

## Using the Library

### Method 1: LD_PRELOAD
```bash
# Run a program with your malloc implementation
env LD_PRELOAD=./libft_malloc.so <program>
```

### Method 2: Linking at Compile Time
```bash
# Compile your program with the custom malloc
gcc -o myprogram myprogram.c -L/path/to/malloc -lft_malloc
```

## Debug Mode
To enable debug output:
```bash
# Build with debug output enabled
make CFLAGS="-Wall -Wextra -Werror -fPIC -DDEBUG_MALLOC=1"
```

## Testing

### Available Test Suites
- basic: Simple malloc/free, calloc, realloc tests
- edge_cases: NULL pointers, zero-size, very large allocations
- performance: Many allocations, fragmentation handling
- thread: Thread-safety tests with concurrent allocations
- absurd: Extreme test cases to stress the implementation
- advanced: Real-world allocation patterns, stability tests
- gnl: Get Next Line test for real program allocation patterns

### Running Tests
```bash
# Build library and run all tests
make test

# Run specific test suite
cd tests
make basic
make edge
make performance
make thread
make absurd
make advanced
make gnl
```
### Test Coverage
The tests verify:

- Memory integrity
- Thread safety
- Performance under load
- Edge case handling
- Memory leak detection
- Stability in long-running scenarios
- Memory Visualization
