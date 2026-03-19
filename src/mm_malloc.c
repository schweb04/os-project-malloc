#include <unistd.h> // Para sbrk
#include <string.h> // Para memset
#include <assert.h> // para debugging básico
#include "mm_malloc.h"

// Inicio de la lista enlazada del heap
void *base = NULL;

struct block_meta *get_block(void *ptr) {
    return (struct block_meta*)ptr - 1; // La aritmética de punteros no está definida para void*, 
                                        // por lo que ptr se castea a struct block_meta*, de forma que al restar 1, 
                                        // se obtiene la dirección del bloque de metadatos asociado al bloque de datos apuntado por ptr
}

struct block_meta *find_free_block_best_fit(struct block_meta **last, size_t size) {
    struct block_meta *current = base;
    struct block_meta *best_fit = NULL;

    while (current) {
        if (current->free && current->size >= size) {
            if (!best_fit || current->size < best_fit->size) best_fit = current;
        }
        *last = current;// Cuando termine el recorrido, last apuntará al último bloque de la lista, 
                        // el cual es necesario para añadir correctamente más bloques a la lista en caso de que se necesite pedir más espacio
        current = current->next;
    }
    return best_fit;
}

struct block_meta *request_space(struct block_meta *last, size_t size) {
    struct block_meta *block;
    block = sbrk(0);
    void *request = sbrk(size + META_SIZE);// Se pide espacio al OS para el bloque de datos y su bloque de metadatos asociado
    assert((void*)block == request);
    
    if (request == (void*) -1) return NULL;
    if (last) last->next = block;
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
    if (size == 0) return NULL;
    struct block_meta* block;
    
    if (!base) {// Si es la primera vez que se llama a my_malloc (base nula), no hay bloques en la lista, por lo que se pide espacio
        block = request_space(NULL, size);

        if (!block) return NULL;
        base = block;
    } else {
        struct block_meta *last = base;
        block = find_free_block_best_fit(&last, size);

        if (!block) {
            block = request_space(last, size);

            if (!block) return NULL;
        } else {
            block->free = 0;
            block->magic = 0x10;
        }
    }
    return (block + 1);// Se devuelve un puntero al bloque de datos, que es la dirección del bloque de metadatos + 1 (como se vio en get_block) 
}

void my_free(void *ptr) {
    // TODO: Marcar el bloque como libre.
    // TODO: Fusionar bloques adyacentes (Coalescing).
    if (!ptr) return;
    struct block_meta *block = get_block(ptr);
    assert(block->free == 0);
    assert(block->magic == 0x00 || block->magic == 0x10);
    block->free = 1;
    block->magic = 0x1;

    struct block_meta *prev_block = find_prev(block);
    struct block_meta *next_block = block->next;

    // Mi implementación está pensada para que un bloque libre absorba al siguiente bloque libre, 
    // por lo que primero verifico el siguiente bloque, y luego el bloque anterior. 
    // De esta forma, si el bloque actual absorbe al siguiente bloque, el bloque actual quedará con el tamaño total de ambos bloques, 
    // y el bloque anterior podrá absorber al bloque actual sin problemas.
    if (next_block && next_block->free) {//Si el siguiente bloque existe y está libre, el bloque actual absorbe el siguiente y al bloque de metadatos asociado al siguiente bloque
        block->size += META_SIZE + next_block->size;
        block->next = next_block->next;
    }
    if (prev_block && prev_block->free) {// Si el bloque anterior existe y está libre, el bloque anterior absorbe el bloque actual y al bloque de metadatos asociado al bloque actual
        prev_block->size += META_SIZE + block->size;
        prev_block->next = block->next;
    }
}

void *my_calloc(size_t nmemb, size_t size) {
    // TODO: Usar my_malloc y luego memset a 0.
    if (nmemb == 0 || size == 0) return NULL;
    size_t total_size = nmemb * size;
    void *ptr = my_malloc(total_size);

    if (ptr) memset(ptr, 0, total_size);
    return ptr;
}

void *my_realloc(void *ptr, size_t size) {
    // TODO: Redimensionar el bloque o moverlo a uno nuevo.
    if (!ptr) return my_malloc(size);// Si ptr es NULL, realloc se comporta como malloc
    if (size == 0) {
        my_free(ptr);// Si size es 0, realloc se comporta como free
        return NULL;
    }
    
    struct block_meta *block = get_block(ptr);

    if (size < block->size) {
        struct block_meta *new_block = (struct block_meta*)((char*)(ptr) + size);// La aritmética de punteros no está definida para void*, 
                                                                                 // por lo que ptr se castea a char*, de forma que al sumar size, 
                                                                                 // se obtiene la dirección del nuevo bloque de datos que quedará después de redimensionar el bloque actual
        new_block->size = block->size - size - META_SIZE;// El nuevo bloque tendrá el tamaño restante, tomando también en cuenta el tamaño del bloque de metadatos asociado al nuevo bloque
        new_block->free = 1;
        new_block->magic = 0x1;
        new_block->next = block->next;
        block->size = size;
        block->next = new_block;
        return ptr;
        
    } else if (size == block->size) return ptr; 
    else {
        void *new_ptr = my_malloc(size);

        if (!new_ptr) return NULL;
        memcpy(new_ptr, ptr, block->size);
        my_free(ptr);
        return new_ptr;
    }
}
