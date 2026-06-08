#include "intern_malloc.h"

extern t_malloc_state	gMallocState;

static	void	free_page(t_heap *heap)
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

	heap->prev = NULL;
	heap->next = NULL;
	munmap(heap, heap->total_size);
}

void	free(void *ptr)
{
	if (ptr == NULL)
		return;

	t_block	*block = (t_block *)((char *)ptr - sizeof(t_block));
	t_heap	*heap = retrieve_heap(block);
	if (heap == NULL || block->in_use == 0)
		return;

	block->in_use = 0;
	heap->free_size += block->size;
	coalescence(heap, block);

	if (heap->block_count == 1 && heap->free_size == heap->total_size - sizeof(t_heap) - sizeof(t_block))
	{
		if (heap->group == LARGE)
		{
			free_page(heap);
			return;
		}

		t_heap**	head = NULL;
		if (heap->group == TINY)
			head = &gMallocState.tiny;
		else
			head = &gMallocState.small;

		t_heap*	cursor = *head;
		int		empty = 0;
		while (cursor)
		{
			if (cursor != heap && cursor->block_count == 1 && cursor->blocks->in_use == 0)
				empty++;
			cursor = cursor->next;
		}

		if (empty != 0)
			free_page(heap);
	}
}
