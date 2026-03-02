#include "malloc.h"

extern t_malloc_state	gMallocState;

void	*realloc(void *ptr, size_t size)
{
	(void)ptr;
	(void)size;
	return NULL;
}
