#include <unistd.h>
#include <stddef.h>

size_t align(size_t n)
{
	return (n + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);
}

typedef struct linalloc_t {
	void* top;
	const void* bottom;
	const size_t size;
} linalloc_t;

linalloc_t linalloc_init(size_t size)
{
	void* start = sbrk(0);
	if (sbrk(size) == (void*)-1) {
		return (linalloc_t) {.top = 0, .bottom = 0, .size = 0};
	}
	return (linalloc_t) {.top = start, .bottom = start, .size = size};
}

void* linalloc(linalloc_t allocator, size_t size)
{
	void* start = allocator.top;
	size_t alloc_size = align(size);
	if (alloc_size > (intptr_t)(allocator.bottom + allocator.size)) {
		return NULL; // OOM
	}
	allocator.top += alloc_size;
	return allocator.top;
}

void linfree(linalloc_t allocator) {
	sbrk(-allocator.size);
}

int main()
{
	linalloc_t l = linalloc_init(1024);
	linalloc(l, 10);
	linfree(l);
}
