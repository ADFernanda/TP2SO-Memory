#ifndef SO605_GC
#define SO605_GC

#define MEMSIZE (long long int) 4096*1024*1024 // Processo tem 4096 MB de memória
#define REFERENCE_SIZE 4

// Nó da lista de memória alocada
typedef struct full_node{
	size_t size;
	int referenceCount;
	struct full_node *referencePointer;
} full_node_t;

// Ponteiro para o nó da lista
typedef full_node_t* mem_full_t;

full_node_t aloca(size_t size, long long int *memsizeFree);
void libera(mem_full_t ptr, long long int *memsizeFree);
full_node_t referencia(mem_full_t ptr, long long int *memsizeFree);

#endif