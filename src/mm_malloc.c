#include <unistd.h> // Para sbrk
#include <string.h> // Para memset
#include <assert.h>
#include "mm_malloc.h"

// Inicio de la lista enlazada del heap
void *base = NULL;

struct block_meta *get_block(void *ptr) {
    return (struct block_meta*)ptr - 1;
}

struct block_meta *find_free_block_best_fit(struct block_meta **last, size_t size) {
    struct block_meta *current = base;
    struct block_meta *best_fit = NULL;

    while (current) {
        if (current->free && current->size >= size) {
            if (!best_fit || current->size < best_fit->size) {
                best_fit = current;
            }
        }
        *last = current;
        current = current->next;
    }
    return best_fit;
}

struct block_meta *request_space(struct block_meta *last, size_t size) {
    struct block_meta *block;
    block = sbrk(0);
    void *request = sbrk(size + META_SIZE);
    assert((void*)block == request);
    
    if (request == (void*) -1) {
        return NULL;
    }

    if (last) {
        last->next = block;
    }
    block->size = size;
    block->next = NULL;
    block->free = 0;
    block->magic = 0x00;
    return block;
}

struct block_meta *find_prev(struct block_meta *block) {
    struct block_meta *current = base;
    struct block_meta *prev = NULL;

    while (current && current != block) {
        prev = current;
        current = current->next;
    }
    return prev;
}

void *my_malloc(size_t size) {
    // TODO: Implementar First-Fit o Best-Fit
    // 1. Verificar si hay un bloque libre del tamaño adecuado.
    // 2. Si no, pedir espacio al OS con sbrk().
    struct block_meta* block;
    
    if (!base) {
        block = request_space(NULL, size);

        if (!block) {
            return NULL;
        }
        base = block;
    } else {
        struct block_meta *last = base;
        block = find_free_block_best_fit(&last, size);

        if (!block) {
            block = request_space(last, size);

            if (!block) {
                return NULL;
            }
        } else {
            block->free = 0;
            block->magic = 0x10;
        }
    }
    return (block + 1); 
}

void my_free(void *ptr) {
    // TODO: Marcar el bloque como libre.
    // TODO: Fusionar bloques adyacentes (Coalescing).
    if (!ptr) {
        return;
    }
    struct block_meta *block = get_block(ptr);
    assert(block->free == 0);
    assert(block->magic == 0x00 || block->magic == 0x10);
    block->free = 1;
    block->magic = 0x1;

    struct block_meta *prev_block = find_prev(block);
    struct block_meta *next_block = block->next;

    if (next_block && next_block->free) {
        block->size += META_SIZE + next_block->size;
        block->next = next_block->next;
    }
    
    if (prev_block && prev_block->free) {
        prev_block->size += META_SIZE + block->size;
        prev_block->next = block->next;
    }
}

void *my_calloc(size_t nmemb, size_t size) {
    // TODO: Usar my_malloc y luego memset a 0.
    size_t total_size = nmemb * size;
    void *ptr = my_malloc(total_size);

    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void *my_realloc(void *ptr, size_t size) {
    // TODO: Redimensionar el bloque o moverlo a uno nuevo.

    (void)ptr;
    (void)size;
    return NULL;
}
