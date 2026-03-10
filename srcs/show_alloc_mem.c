#include "intern_malloc.h"
#include "libft.h"

extern t_malloc_state	gMallocState;

static int	print_mem(t_heap *head)
{
	size_t	total = 0;
	while (head)
	{
		t_block	*block = head->blocks;
		while (block)
		{
			if (block->in_use != 0)
			{
				void	*start = (char *)block + sizeof(t_block);
				void	*end = start + block->size;
				ft_printf("%p - %p : %i bytes\n", start, end, block->size);
				total += block->size;
			}
			block = block->next;
		}
		head = head->prev;
	}
	return total;
}

static t_heap	*get_last(t_heap *head)
{
	while (head)
	{
		if (head->next == NULL)
			return head;
		head = head->next;
	}
	return head;
}

void	show_alloc_mem(void)
{
	unsigned long long	total = 0;
	t_heap	*head;

	head = get_last(gMallocState.tiny);
	ft_printf("TINY  : %p\n", gMallocState.tiny);
	total += print_mem(head);

	head = get_last(gMallocState.small);
	ft_printf("SMALL : %p\n", gMallocState.small);
	total += print_mem(head);

	head = get_last(gMallocState.large);
	ft_printf("LARGE : %p\n", gMallocState.large);
	total += print_mem(head);

	ft_printf("Total : %i bytes\n", total);
}
