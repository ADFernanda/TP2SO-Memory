// CHANGELOG
// ver 2.0
// Código desenvolvido por Radeori para realização da parte 3 do Trabalho Prático
// referente à disciplina apresentada abaixo
// ver. 1.0
// Esqueleto de um código de alocação de memória (novo maloc)
// Feito para a disciplina DCC065 - Sistemas Operacionais (UFMG)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "garbage.h"

// Caso você utilize a lista fora da memória que vamos alocar a mesma
// terá que ser acessada como uma var global. Global vars in C.

mem_full_t alocation;	// O vetor alocation mantem a informações de alocações.
						// É lido ao executarmos uma operação 'f'

void exitControlado(int exitMode){
	free(alocation);
	exit(exitMode);
}

full_node_t aloca(size_t size, long long int *memsizeFree){
	full_node_t node;
	char numberString[15];
	sprintf(numberString, "%lld", (*memsizeFree) - size - sizeof(full_node_t));
	if(numberString[0] != '-'){
		(*memsizeFree) -= size + sizeof(full_node_t);
		node.size = size + sizeof(full_node_t);
		node.referenceCount = 1;
		node.referencePointer = NULL;
		return node;
	}
	else{
		printf("Memória cheia demais\n");
		exitControlado(1);
	}
}

void libera(mem_full_t ptr, long long int *memsizeFree){
	do{
		if(ptr->referenceCount >= 1){
			(ptr->referenceCount)--;
			if(ptr->referenceCount == 0){
				(*memsizeFree) += ptr->size;
			}
		}
		else{
			printf("Memória alocada não referenciada\n");
			exitControlado(1);
		}
		ptr = ptr->referencePointer;
	} while(ptr != NULL);
}

full_node_t referencia(mem_full_t ptr, long long int *memsizeFree){
	full_node_t node;
	if(ptr->referenceCount >= 1){
		node = aloca(REFERENCE_SIZE, memsizeFree);
		node.referencePointer = ptr;
		(ptr->referenceCount)++;
		return node;
	}
	else{
		printf("Referenciando memória não alocada\n");
		exitControlado(1);
	}
}

int main(int argc, char **argv){
	int nops;							// ID máximo
	int opid;							// ID da operação atual
	int memsize;						// Tamanho da alocação
	long long int memsizeFree;	// Tamanho livre da memória
	char optype;						// Tipo da operação
	full_node_t addr;					// Endereço da alocação
	mem_full_t ptrD;					// Endereço da alocação destino

	memsizeFree = MEMSIZE;
	scanf("%d\n", &nops);
	alocation = (mem_full_t) calloc(nops, sizeof(full_node_t));

	while (scanf("%d", &opid) == 1){
		getchar();
		scanf("%d ", &memsize);

		scanf("%c", &optype);
		if(optype == 'a'){		// Aloca!
			addr = aloca((size_t)memsize, &memsizeFree);
			alocation[opid] = addr;
		}
		else if(optype == 'f'){	// Free!
			addr = alocation[opid];
			libera(&addr, &memsizeFree);
			alocation[opid] = addr;
		}
		else if(optype == 'r'){	// Referencia!
			ptrD = &(alocation[memsize]);
			addr = referencia(ptrD, &memsizeFree);
			alocation[opid] = addr;
		}
		else{
			printf("Erro na entrada\n");
			exitControlado(1);
		}
	}

	free(alocation);

	printf("%lld\n", MEMSIZE - memsizeFree);

	return 0;
}