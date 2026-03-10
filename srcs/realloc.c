#include "intern_malloc.h"

extern t_malloc_state	gMallocState;

void	*realloc(void *ptr, size_t size)
{
	if (gMallocState.page_size == 0)
		gMallocState.page_size = sysconf(_SC_PAGESIZE);

	if (ptr == NULL)
		return malloc(size);
	if (size == 0)
	{
		free(ptr);
		return NULL;
	}

	t_block	*block = (t_block*)((char *)ptr - sizeof(t_block));
	if (block->size == size)
		return ptr;

	if (size < block->size)
	{
		size_t res = size % MIN_BLOCK_SIZE;
		if (res != 0)
			size += MIN_BLOCK_SIZE - res;

		t_block	*next	= block->next;
		t_block *new	= (t_block *)((char *)ptr + size);

		new->prev = block;
		new->next = next;
		new->in_use = 0;
		new->size = block->size - size - sizeof(t_block);
		if (next != NULL)
			next->prev = new;
		block->size = size;
		block->next = new;

		t_heap	*heap = retrieve_heap(block);
		heap->block_count += 1;
		heap->free_size += new->size;
		coalescence(heap, new);

		return ptr;
	}
	else
	{
		void	*new_ptr = malloc(size);
		ft_memcpy(new_ptr, ptr, block->size);
		free(ptr);
		return new_ptr;
	}
}
