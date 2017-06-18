
// Esqueleto de um código de alocação de memória (novo maloc)
// Feito para a disciplina DCC065 - Sistemas Operacionais (UFMG)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "aloca.h"

#define ZERO 0
// Strategia de alocação como variável global. Feio mas funciona dado o
// esqueleto do TP.
static char *STRATEGY = NULL;
void *init, *memory;
free_node_t *inicioLista;
//void *memory = mmap(init, MEMSIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

// Caso você utilize a lista fora da memória que vamos alocar a mesma
// terá que ser acessada como uma var global. Global vars in C.

void Remove(free_node_t *nodo, free_node_t *anterior){
  //remove da lista de free o nodo
    anterior->next = nodo->next;
    free(nodo);
}

void *aloca(size_t size) {
  free_node_t *espacoAlocado, *anteriorAoAlocado;
 
  if (strcmp(STRATEGY, "bf") == 0) {
    
    if(inicioLista->next == NULL){ //nunca houve free()      
      inicioLista->size -= size;
      inicioLista->endereco = memory + (MEMSIZE - inicioLista->size);
      espacoAlocado = inicioLista;

    }else{ //procura por espaço para alocação adequado para best fit

      free_node_t *proximo = inicioLista->next;
      free_node_t *anterior = inicioLista;

      while(proximo->next != NULL){
        anterior = proximo;
        proximo = proximo->next;
        if(proximo->size >= size){
          if(proximo->size == size){
            anteriorAoAlocado = anterior;
            espacoAlocado = proximo;            
            return (void *)(espacoAlocado->endereco);
          }else if(proximo->size < espacoAlocado->size && proximo->size>size)
            anteriorAoAlocado = anterior;
            espacoAlocado = proximo;
          }
        }
        Remove(espacoAlocado, anteriorAoAlocado);    
      }
    }
    
    return (void *)(espacoAlocado->endereco);;
}
  
void libera(void *ptr, size_t size) {
  //e ae? o que faço com esse void*?
  free_node_t *proximo = inicioLista;
  while(proximo->next != NULL){
    proximo = proximo->next;
  }
  free_node_t *novoNodo = (free_node_t*) malloc((sizeof(free_node_t)));
  proximo->next = novoNodo;
  novoNodo->size = size;
  novoNodo->endereco = ptr;
  novoNodo->next = NULL;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage %s <algorithm>\n", argv[0]);
    exit(1);
  }
  STRATEGY = argv[1];
  int nops;
  printf("\nDigite o número de operações\n");
  scanf("%d", &nops);
  
  char *algorithms[] = {"ff", "bf", "wf", "nf"};
  int n_alg = 4;
  int valid = 0;
  for (int i = 0; i < n_alg; i++) {
    if (strcmp(STRATEGY, algorithms[i]) == 0) {
      valid = 1;
      break;
    }
  }

  if (valid == 0) {
    printf("Algoritmo inválido: Usage %s <algorithm>\n", argv[0]);
    printf("--onde o algoritmo vem das opções: {ff, bf, wf, nf}");
    exit(1);
  }

  init = sbrk(0);

  memory = mmap(init, MEMSIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  inicioLista = (free_node_t*) malloc(sizeof(free_node_t));
  inicioLista->size = MEMSIZE;
  inicioLista->posicao = 0;
  inicioLista->next = NULL;
  // O vetor variables mantem os endereços de ids alocados.
  // É lido ao executarmos uma operação 'f'
  void **variables = (void **) malloc(nops * sizeof(void**));
  for (int i = 0; i < nops; i++)
    variables[i] = NULL;

  int opid;    // ID da operação
  int memsize; // Tamanho da alocação
  char optype; // Tipo da operação
  void *addr;

  printf("Digite o id da operação\n");
  while (scanf("%d", &opid) == 1) {
    getchar();
    printf("Digite o tamanho a ser alocado\n");
    scanf("%d", &memsize);
    getchar();
    printf("Digite o tipo de operacao:\n");    
    scanf("%c", &optype); 
    if (optype == 'a') {  
      // Aloca!
      addr = aloca(memsize);
      variables[opid] = addr;
    } else if (optype == 'f') {  // Free!
      addr = variables[opid];
      libera(addr);
    } else {
      printf("Erro na entrada");
      exit(1);
    }
    printf("Digite o id da operação\n");
  }

  for (int i = 0; i < nops; i++)
    if (variables[i] != NULL)
      free(variables[i]);
  free(variables);

  return 0;
}
