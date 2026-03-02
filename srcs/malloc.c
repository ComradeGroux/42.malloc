#include "malloc.h"

t_malloc_state	gMallocState = { NULL, NULL, NULL, 0};

static t_heap	*create_heap(t_heap_group group, size_t size_required)
{
	void	*ptr;
	size_t	size = gMallocState.page_size;

	t_heap	**head;
	switch (group)
	{
		case TINY:
			head = &gMallocState.tiny;
			size *= TINY_ZONE_SIZE;
			break;

		case SMALL:
			head = &gMallocState.small;
			size *= SMALL_ZONE_SIZE;
			break;

		default:
		{
			head = &gMallocState.large;
			size_required += sizeof(t_heap) + sizeof(t_block);
			size_t	res = size_required % gMallocState.page_size;
			if (res != 0)
				size = size_required + (gMallocState.page_size - res);
			else
				size = size_required;
			break;
		}
	}

	ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED)
		return NULL;

	t_heap	*new = (t_heap *)ptr;
	new->prev		= NULL;
	new->next		= NULL;
	new->blocks		= NULL;
	new->group		= group;
	new->total_size	= size;
	new->free_size 	= size - sizeof(t_heap) - sizeof(t_block);
	new->block_count= 1;

	if (*head == NULL)
		*head = new;
	else
	{
		t_heap *current = *head;
		while (current->next)
			current = current->next;
		current->next = new;
		new->prev = current;
	}

	t_block	*block = (t_block *)((char *)ptr + sizeof(t_heap));
	block->prev		= NULL;
	block->next		= NULL;
	block->size		= new->free_size;
	block->in_use	= 0;

	new->blocks = block;

	return new;
}

static void	split_block(t_heap *heap, t_block *block, size_t size)
{
	t_block	*new = (t_block *)((char *)block + sizeof(t_block) + size);
	new->prev	= block;
	new->next	= block->next;
	new->size	= block->size - sizeof(t_block) - size;
	new->in_use	= 0;

	block->next		= new;
	block->size		= size;
	block->in_use	= 1;

	if (new->next != NULL)
		new->next->prev = new;

	heap->block_count += 1;
	heap->free_size -= (size + sizeof(t_block));
}

static void	alloc_block(t_heap *heap, t_block *block, size_t size)
{
	if (block->size >= size + sizeof(t_block) + MIN_BLOCK_SIZE)
		split_block(heap, block, size);
	else
	{
		block->in_use = 1;
		heap->free_size -= block->size;
	}
}

void	*malloc(size_t size)
{
	if (gMallocState.page_size == 0)
		gMallocState.page_size = sysconf(_SC_PAGESIZE);

	size_t	res = size % MIN_BLOCK_SIZE;
	if (res != 0)
		size += MIN_BLOCK_SIZE - res;

	t_heap	*heap_head	= NULL;
	t_block	*block_head	= NULL;
	if (size <= TINY_MAX)
		heap_head = gMallocState.tiny;
	else if (size <= SMALL_MAX)
		heap_head = gMallocState.small;
	else
	{
		heap_head = create_heap(LARGE, size);
		if (heap_head == NULL)
			return NULL;
		block_head = heap_head->blocks;
		alloc_block(heap_head, block_head, size);
		return (char *)block_head + sizeof(t_block);
	}

	while (heap_head)
	{
		if (heap_head->free_size >= size)
		{
			block_head = heap_head->blocks;
			while (block_head)
			{
				if (block_head->in_use == 0 && block_head->size >= size)
				{
					alloc_block(heap_head, block_head, size);
					break;
				}
				block_head = block_head->next;
			}
			if (block_head != NULL)
				break;
		}
		heap_head = heap_head->next;
	}
	if (heap_head == NULL)
	{
		if (size <= TINY_MAX)
			heap_head = create_heap(TINY, size);
		else if (size <= SMALL_MAX)
			heap_head = create_heap(SMALL, size);

		if (heap_head == NULL)
			return NULL;

		block_head = heap_head->blocks;
		alloc_block(heap_head, block_head, size);
	}

	return (char *)block_head + sizeof(t_block);
}
