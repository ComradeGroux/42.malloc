#pragma once

#include "libft.h"

#include <stdlib.h>
#include <sys/mman.h>

typedef enum e_heap_group
{
	TINY,
	SMALL,
	LARGE
} t_heap_group;

#define MIN_BLOCK_SIZE 16

#define TINY_MAX	128
#define SMALL_MAX	1024

#define TINY_ZONE_SIZE	4
#define SMALL_ZONE_SIZE 26

typedef struct s_heap
{
	struct s_heap	*prev;
	struct s_heap	*next;
	struct s_block	*blocks;
	t_heap_group	group;
	size_t			total_size;
	size_t			free_size;
	size_t			block_count;
} t_heap;

typedef struct s_block
{
	struct s_block	*prev;
	struct s_block	*next;
	size_t			size;
	size_t			in_use;
} t_block;

typedef struct s_malloc_state
{
	t_heap	*tiny;
	t_heap	*small;
	t_heap	*large;
	size_t	page_size;
} t_malloc_state;

void	free(void *ptr);
void	*malloc(size_t size);
void	*realloc(void *ptr, size_t size);

t_heap	*retrieve_heap(t_block *block);
void	coalescence(t_heap *heap, t_block *block);

void	show_alloc_mem(void);
