#include "intern_malloc.h"

extern t_malloc_state gMallocState;

static t_heap	*check_part_of(t_heap *heap, t_block *block)
{
	while (heap)
	{
		char	*end = (char *)heap + heap->total_size;
		if ((void *)block > (void *)heap && (void *)block < (void *)end)
			return heap;
		heap = heap->next;
	}
	return NULL;
}

t_heap	*retrieve_heap(t_block *block)
{
	t_heap	*heap_head = NULL;

	heap_head = check_part_of(gMallocState.tiny, block);
	if (heap_head != NULL)
		return heap_head;

	heap_head = check_part_of(gMallocState.small, block);
	if (heap_head != NULL)
		return heap_head;

	heap_head = check_part_of(gMallocState.large, block);
	if (heap_head != NULL)
		return heap_head;

	return NULL;
}

void	coalescence(t_heap *heap, t_block *block)
{
	if (block->next != NULL)
	{
		if (block->next->in_use == 0)
		{
			t_block	*next = block->next;
			if (next->next != NULL)
				next->next->prev = block;

			block->next = next->next;
			block->size += next->size + sizeof(t_block);

			heap->block_count -= 1;
			heap->free_size += sizeof(t_block);
		}
	}
	if (block->prev != NULL)
	{
		if (block->prev->in_use == 0)
		{
			t_block	*prev = block->prev;
			prev->next = block->next;
			if (prev->next != NULL)
				prev->next->prev = prev;
			prev->size += block->size + sizeof(t_block);

			heap->block_count -= 1;
			heap->free_size += sizeof(t_block);
		}
	}
}
