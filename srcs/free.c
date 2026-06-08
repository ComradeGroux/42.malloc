#include "intern_malloc.h"

extern t_malloc_state	gMallocState;

void	free(void *ptr)
{
	if (ptr == NULL)
		return;

	t_block	*block = (t_block *)((char *)ptr - sizeof(t_block));
	t_heap	*heap = retrieve_heap(block);
	if (heap == NULL)
		return;

	block->in_use = 0;
	heap->free_size += block->size;
	coalescence(heap, block);

	if (heap->block_count == 1 && heap->blocks->in_use == 0)
	{
		t_heap**	head = NULL;
		if (heap->group == TINY)
			head = &gMallocState.tiny;
		else if (heap->group == SMALL)
			head = &gMallocState.small;
		else
			head = &gMallocState.large;

		t_heap*	cursor = *head;
		int		empty = 0;
		while (cursor)
		{
			if (cursor != heap && cursor->block_count == 1 && cursor->blocks->in_use == 0)
				empty++;
			cursor = cursor->next;
		}

		if (empty != 0)
		{
			if (heap->prev != NULL)
				heap->prev->next = heap->next;
			if (heap->next != NULL)
				heap->next->prev = heap->prev;

			if (heap->group == TINY && gMallocState.tiny == heap)
				gMallocState.tiny = heap->next;
			else if (heap->group == SMALL && gMallocState.small == heap)
				gMallocState.small = heap->next;
			else if (heap->group == LARGE && gMallocState.large == heap)
				gMallocState.large = heap->next;

			munmap(heap, heap->total_size);
		}
	}
}
