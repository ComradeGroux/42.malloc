#include "malloc.h"

extern t_malloc_state	gMallocState;

void	show_alloc_mem(void)
{
	ft_printf("TINY : %p\n", gMallocState.tiny);

	ft_printf("SMALL : %p\n", gMallocState.small);

	ft_printf("LARGE : %p\n", gMallocState.large);


	ft_printf("Total : %i bytes\n");
}
