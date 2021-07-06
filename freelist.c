#include <unistd.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void* heap_start = NULL;
static void* alloc_last = NULL;

struct alloc_data {
	size_t size;
	bool used;
}

/* struct freelist { */
/*	void* next; */
/*	void* block; */
/* }; */

/*	/\* for (struct freelist* i = &f; i->next != NULL; i = i->next) { *\/ */
/*	/\*	if (((struct alloc_data*)i->block)->used == false) { *\/ */

/*	/\*	}  *\/ */
/*	/\* } *\/ */


/* struct freelist add_block(struct freelist f, void* block) */
/* { */
/*	struct freelist* i; */
/*	for (i = &f; i->next != NULL; i = i->next); */

/* } */

};
size_t align(size_t n)
{
	return (n + sizeof(intptr_t) - 1) & ~(sizeof(intptr_t) - 1);
}

void* find_first(size_t size)
{
	intptr_t top = (intptr_t)sbrk(0);
	for (uint8_t* p = heap_start; (intptr_t) p < top;) {
		struct alloc_data metadata = *(struct alloc_data*)p;
		if (metadata.size >= size && metadata.used == false) {
			return p + sizeof(struct alloc_data);
		}
		p += sizeof(struct alloc_data) + metadata.size;
	}
	return NULL;
}

void* find_next(size_t size)
{
	uint8_t* top = sbrk(0);
	uint8_t* p = alloc_last;
	do {
		struct alloc_data metadata = *(struct alloc_data*)p;
		if (metadata.size >= size && metadata.used == false) {
			return p + sizeof(struct alloc_data);
		}
		p += sizeof(struct alloc_data) + metadata.size;
		if (p == top) p = heap_start;
	} while (p != alloc_last);
}

void* find_best(size_t size)
{
	void* top = sbrk(0);
	printf("%p FOUND START vs HEAP START %p\n", (void*)top, heap_start);
	int dist = INT32_MAX;
	void* candidate = NULL;
	for (uint8_t* p = heap_start; p != top;) {
		struct alloc_data metadata = *(struct alloc_data*) p;
		printf("%p: %zu, %zu, %d, %d\n", p, metadata.size, metadata.size-size, dist, metadata.used);
		if (metadata.size-size >= 0 && metadata.size-size < dist && metadata.used == false) {
			dist = metadata.size-size;
			candidate = p;
		}
		p += sizeof(struct alloc_data) + metadata.size;
	}
	if (candidate == NULL) return NULL;
	return candidate+sizeof(struct alloc_data);
}

void* alloc(size_t size)
{
	void* start = sbrk(0);
	if (heap_start == NULL) heap_start = start;
	size_t alloc_size = sizeof(struct alloc_data) + align(size);
	uint8_t* ptr = find_first(size);
	if (ptr != NULL) return ptr;
	sbrk(alloc_size);
	ptr = start;
	if (ptr == (void*)-1) return NULL; // OOM
	*(struct alloc_data*)ptr = (struct alloc_data) {.size = align(size), .used = true};
	struct alloc_data metadata = *(struct alloc_data*) ptr;
	printf("%zu, %zu\n", metadata.size, metadata.size-size);

	ptr = ptr + sizeof(struct alloc_data);
	printf("%p START\n", sbrk(0));
	return ptr;
}

void unalloc(void* ptr)
{
	struct alloc_data* metadata = (struct alloc_data*)((intptr_t)ptr - sizeof(struct alloc_data));
	metadata->used = false;
}

int main()
{
	void* thing = alloc(23);
	printf("ALLOCATED PRE BLOCK\n");
	char* p = alloc(sizeof(char)*4);
	strcpy(p, "hi!");
	printf("ALLOC'D STR %p\n", p);
	unalloc(p);
	unalloc(thing);
	/* alloc(100); */
	/* alloc(50); */
	int* a = alloc(sizeof(int));
	printf("%p\n", a);
}
