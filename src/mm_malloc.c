#include <unistd.h> // Para sbrk
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

void *my_malloc(size_t size) {
    // TODO: Implementar First-Fit o Best-Fit
    // 1. Verificar si hay un bloque libre del tamaño adecuado.
    // 2. Si no, pedir espacio al OS con sbrk().
    return NULL; 
}

void my_free(void *ptr) {
    // TODO: Marcar el bloque como libre.
    // TODO: Fusionar bloques adyacentes (Coalescing).

    if (ptr == NULL) {
        return;
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
    return NULL;
}
