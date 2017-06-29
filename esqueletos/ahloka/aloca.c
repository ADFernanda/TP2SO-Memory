// Esqueleto de um código de alocação de memória (novo maloc)
// Feito para a disciplina DCC065 - Sistemas Operacionais (UFMG)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "aloca.h"

// Strategia de alocação como variável global. Feio mas funciona dado o
// esqueleto do TP.
static char *STRATEGY = NULL;

// Caso você utilize a lista fora da memória que vamos alocar a mesma
// terá que ser acessada como uma var global. Global vars in C.
free_list_t *HEAP = NULL;

void InsereFimLista(free_node_t *newNode){ //insere no fim da lista e atualiza HEAP->lastAloca
  free_node_t *proximo = HEAP->head;

    while(proximo->next != NULL){
      proximo = proximo->next;
    }

    proximo->next = newNode;
    newNode->next = NULL;

    HEAP->lastAlloca = newNode;
}

/*
 * Sempre aloca até estourar o limite, não implementei libera
 */
void *alwaysGrow(size_t size) {
  assert(HEAP->lastAlloca != NULL);
  free_node_t *lastAlloca = HEAP->lastAlloca;
  //printf("Ultimo free %lu\n", lastAlloca->free);
  // Temos espaço para alocar + o espaço da lista?
  if (lastAlloca->free < sizeof(free_node_t) + size) {
    return NULL;
  }
  // Sim!
  // Novo nó logo após o último aloca.
  // Posicao da alocacao + tamanho alocado + tamanho do cabeçalho
  free_node_t *newNode = (void*)lastAlloca + lastAlloca->size + \
                         sizeof(free_node_t);
  newNode->next = NULL;
  newNode->size = size;
  newNode->free = lastAlloca->free - sizeof(free_node_t) - size;

  // Só crescemos, o espaço anterior não tem memória livre mais.
  lastAlloca->free = 0;
  lastAlloca->next = newNode;
  HEAP->lastAlloca = newNode; // Atualiza ponteiro para última operação

  // Retornamos o inicio do espaço que alocamos tirando o cabeçalho
  return (void*)newNode + sizeof(free_node_t);
}

void *ff(size_t size) {

  free_node_t *proximo = HEAP->head;
  free_node_t *lastAlloca = HEAP->lastAlloca;
  //printf("Ultimo free %lu\n", lastAlloca->free);

  while(proximo != NULL){//caminhar na lista e pegar primeiro espaço vazio grande o suficiente
    if(proximo->free >= size){//encontrado espaço em que será alocado

      // Temos espaço para alocar + o espaço da lista?
      if (proximo->free < sizeof(free_node_t) + size) {
        return NULL;
      }

      free_node_t *newNode = (void*)proximo + proximo->size + \
                         sizeof(free_node_t); //newNode no espaço imediatamente após o espaço alocado em proximo+cabeçaçho de newNode

      newNode->next = NULL;
      newNode->size = size;
      newNode->free = proximo->free - sizeof(free_node_t) - size;

      proximo->free = 0;

      InsereFimLista(newNode);
      proximo->free = 0;

      return (void*)newNode + sizeof(free_node_t);
    }
    proximo = proximo->next;
    
  }
  return NULL;//não foi encontrado espaço de memória grande o suficiente para alocação
}

void *bf(size_t size) {

  free_node_t *proximo = HEAP->head, *nodeBestFit = HEAP->head;
  int alocado = 0;
  //printf("ANTES: %lu\n", nodeBestFit->free );

  while(proximo != NULL){//caminhar na lista enquanto há algum espaço livre ou não qualquer
    //printf("proximo %lu\n", proximo->free);
    if( (proximo->free < nodeBestFit->free) && (proximo->free >= size) || (nodeBestFit->free == 0) ){//encontrado candidato a espaço em que será alocado
      nodeBestFit = proximo;
    }
    proximo = proximo->next;
  }
  //printf("nodeBestFit: %lu\n", nodeBestFit->free );

  if(nodeBestFit->free >= sizeof(free_node_t) + size){ //espaço free mais próximo de size e grande o bastante

    free_node_t *newNode = (void*)nodeBestFit + nodeBestFit->size + \
                         sizeof(free_node_t); //newNode no espaço imediatamente após o espaço alocado em nodeBestFit+cabeçaçho de newNode

    newNode->next = NULL;
    newNode->size = size;
    newNode->free = nodeBestFit->free - sizeof(free_node_t) - size;

    InsereFimLista(newNode);
    nodeBestFit->free = 0;

    return (void*)newNode + sizeof(free_node_t);
  }

  return NULL;
}

void *wf(size_t size) {
  free_node_t *proximo = HEAP->head, *nodeWorstFit = HEAP->head;
  int alocado = 0;

  while(proximo != NULL){//caminhar na lista enquanto há algum espaço livre ou não qualquer

    if( (proximo->free >= nodeWorstFit->free) && (proximo->free >= size) ){//encontrado candidato a espaço em que será alocado
      nodeWorstFit = proximo;
    }
    proximo = proximo->next;
  }

  //printf("NodeWorstFit: %lu\n", nodeWorstFit->free );

  if(nodeWorstFit->free > sizeof(free_node_t) + size){ //espaço free mais próximo de size e grande o bastante

    free_node_t *newNode = (void*)nodeWorstFit + nodeWorstFit->size + \
                         sizeof(free_node_t); //newNode no espaço imediatamente após o espaço alocado em nodeWorstFit+cabeçaçho de newNode

    newNode->next = NULL;
    newNode->size = size;
    newNode->free = nodeWorstFit->free - sizeof(free_node_t) - size;

    InsereFimLista(newNode);
    nodeWorstFit->free = 0;

    return (void*)newNode + sizeof(free_node_t);
  }

  return NULL;
}

void *nf(size_t size) {

  free_node_t *lastAlloca = HEAP->lastAlloca, *proximo = HEAP->lastAlloca;

  while(proximo != NULL){//caminhar na lista e pegar primeiro espaço vazio grande o suficiente
    if(proximo->free >= size){//encontrado espaço em que será alocado

      // Temos espaço para alocar + o espaço da lista?
      if (proximo->free < sizeof(free_node_t) + size) {
        return NULL;
      }

      free_node_t *newNode = (void*)proximo + proximo->size + \
                         sizeof(free_node_t); //newNode no espaço imediatamente após o espaço alocado em proximo+cabeçaçho de newNode

      newNode->next = NULL;
      newNode->size = size;
      newNode->free = proximo->free - sizeof(free_node_t) - size;    

      InsereFimLista(newNode);
      proximo->free = 0;

      return (void*)newNode + sizeof(free_node_t);
    }
    proximo = proximo->next;
    
  }
  return NULL;//não foi encontrado espaço de memória grande o suficiente para alocação
}


void ImprimeFragmentacao(){

  size_t totalFree=0;
  free_node_t *proximo= HEAP->head, *largestFreeNode = HEAP->head;

  while(proximo != NULL){
    totalFree += proximo->free;

    if(proximo->free > largestFreeNode->free){
      largestFreeNode = proximo;
    }
    proximo = proximo->next;

  }
  printf("Fragmentacao: %f\n",1-(double)largestFreeNode->free/totalFree);
}

void *aloca(size_t size) {
  if (strcmp(STRATEGY, "ag") == 0) {
    return alwaysGrow(size);
  }

  if (strcmp(STRATEGY, "ff") == 0) {
    return ff(size);
  }

  if (strcmp(STRATEGY, "bf") == 0) {
    return bf(size);
  }

  if (strcmp(STRATEGY, "wf") == 0) {
    return wf(size);
  }

  if (strcmp(STRATEGY, "nf") == 0) {
    return nf(size);
  }

  return NULL;
}

void libera(void *ptr) {
    free_node_t *proximo1 = HEAP->head;

  free_node_t *metaData = (void*)ptr - sizeof(free_node_t);
  //printf("metaData\t size:%lu\n",metaData->size );

  free_node_t *proximo = HEAP->head, *anterior= HEAP->head, *anteriorMetaData = NULL, *proximoMetaData = NULL;
  //printf("libera\t size:%lu\n",metaData->size );
  //procurando pelo ponteiro que aponta para metadata
  while(proximo != metaData){//caminhar na lista até encontrar metaData
    anterior = proximo;
    proximo = proximo->next;
    //printf("achei\t size:%lu\n",proximo->size );
  }

  if(proximo != metaData){
    printf("Erro ao liberar espaço de memoria.\n");
  }

  anteriorMetaData = anterior;
  proximoMetaData = metaData->next;
  anteriorMetaData->free += sizeof(free_node_t) + metaData->free + metaData->size;
  anteriorMetaData->next = metaData->next;
}

void run(void **variables, int nops) {
  // Vamos iniciar alocando todo o MEMSIZE. Vamos split e merges depois.
  // Vou iniciar o HEAP usando NULL, deixa o SO decidir. Podemos usar sbrk(0)
  // também para sugerir o local inicial.
  HEAP = mmap(NULL, MEMSIZE,
              PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS,
              -1, 0);
  assert(HEAP != NULL);
  assert(HEAP != MAP_FAILED);

  HEAP->head = (void*) HEAP + sizeof(free_list_t);
  HEAP->lastAlloca = HEAP->head;
  HEAP->head->size = 0;
  HEAP->head->free = MEMSIZE - sizeof(free_list_t) - sizeof(free_node_t);
  HEAP->head->next = NULL;
  int i=0;

  int opid;    // ID da operação
  int memsize; // Tamanho da alocação
  char optype; // Tipo da operação
  void *addr;

  while (scanf("%d", &opid) == 1) {
    getchar();
    scanf("%d", &memsize);
    getchar();
    scanf("%c", &optype);
    getchar();
    //printf("%d\n",opid );
    //printf("Alocando %d; %d; %c\n", opid, memsize, optype);
    if (optype == 'a') {         // Aloca!

      addr = aloca(memsize);
      if (addr == NULL) {
        printf("mem full\n");
        munmap(HEAP, MEMSIZE);
        exit(1);
      }
      variables[opid] = addr;
    } else if (optype == 'f') {  // Free!
      addr = variables[opid];
      //("\n\noperacao: %d\n",opid );
      libera(addr);
    } else {
      //printf("ENTRADA %c\n", optype);
      printf("Erro na entrada\n");
      munmap(HEAP, MEMSIZE);
      exit(1);
    }
  }
  ImprimeFragmentacao();
  munmap(HEAP, MEMSIZE);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage %s <algorithm>\n", argv[0]);
    exit(1);
  }
  STRATEGY = argv[1];

  int nops;
  scanf("%d\n", &nops);
  //printf("%d\n", nops);

  char *algorithms[] = {"ff", "bf", "wf", "nf", "ag"};
  int n_alg = 5;
  int valid = 0;
  for (int i = 0; i < n_alg; i++) {
    if (strcmp(STRATEGY, algorithms[i]) == 0) {
      valid = 1;
      break;
    }
  }
  if (valid == 0) {
    printf("Algoritmo inválido: Usage %s <algorithm>\n", argv[0]);
    printf("--onde o algoritmo vem das opções: {ff, bf, wf, nf, ag}\n");
    exit(1);
  }

  // O vetor variables mantem os endereços de ids alocados.
  // É lido ao executarmos uma operação 'f'
  void **variables = (void **) malloc(nops * sizeof(void**));
  assert(variables != NULL);

  for (int i = 0; i < nops; i++)
    variables[i] = NULL;

  run(variables, nops);

  free(variables);
}
