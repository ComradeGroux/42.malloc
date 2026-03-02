#include "malloc.h"

extern t_malloc_state	gMallocState;

static int	print_mem(t_heap *head)
{
	size_t	total = 0;
	while (head)
	{
		ft_printf("%p - %p : %i bytes\n", head, (char *)head + head->total_size, head->total_size);
		total += head->total_size;
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
