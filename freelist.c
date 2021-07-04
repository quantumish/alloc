#include <unistd.h>
#include <stddef.h>
#include <stdbool.h>

static void* heap_start = NULL;

struct alloc_data {
	size_t size;
	bool used;
};

size_t align(size_t n)
{
	return (n + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);
}

void* find_first(size_t size)
{
	intptr_t top = (intptr_t)sbrk(0);
	for (void* p = heap_start; (intptr_t) p < top;) {
		struct alloc_data metadata = *(struct alloc_data*)p;
		if (metadata.size >= size && metadata.used == false) {
			return p + sizeof(struct alloc_data);
		}
	}
	return NULL;
}

void* alloc(size_t size)
{
	void* start = sbrk(0);
	if (heap_start == NULL) heap_start = start;
	size_t alloc_size = sizeof(size_t) + sizeof(bool) + align(size);
	void* ptr = find_first(size);
	if (ptr != NULL) {
		return ptr;
	} else ptr = sbrk(alloc_size);
	if (ptr == (void*)-1) return NULL; // OOM
	*(struct alloc_data*)ptr = (struct alloc_data) {.size = size, .used = true};
	ptr = ptr + sizeof(struct alloc_data);
	return ptr;
}

void unalloc(void* ptr)
{
	struct alloc_data* metadata = (struct alloc_data*)((intptr_t)ptr - sizeof(struct alloc_data));
	metadata->used = false;
}
