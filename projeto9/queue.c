// GRR20190367 Vinicius Tikara Venturi Date

#include "queue.h"

#include <stdio.h>

int queue_size (queue_t *queue){
    // fila vazia, logo tamanho 0
    if(!queue){
        return 0;
    } 

    queue_t *primeiro = queue;
    queue_t *proximo = primeiro;
    int qt = 0;

    // como já foi checado se estava vazia
    // temos pelos menos um elemento a ser contado
    do{
        proximo = proximo->next;
        qt++;
    } while(proximo != primeiro);

    return qt;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    queue_t *proximo = queue;

    printf("%s [", name);

    for(int i = 0; i < queue_size(queue); i++){
        print_elem((void *) proximo);

        // evita o espaço extra após o último elemento da fila
        if(i < queue_size(queue) - 1)
            printf(" ");
        
        proximo = proximo->next;
    }
    printf("]\n");
}

int queue_append (queue_t **queue, queue_t *elem){
    if(!queue){
        fprintf(stderr, "ERRO: Fila não existe\n");
        return -1;
    }
    if(!elem){
        fprintf(stderr, "ERRO: Elemento a ser inserido não existe\n");
        return -2;
    }
    // se o elemento tem próximo ou prévio, quer fizer que
    // aponta para algum outro elemento de outra fila
    if(elem->next || elem->prev){
        fprintf(stderr, "ERRO: Elemento a ser inserido pertence a outra fila\n");
        return -3;
    }

    // se a fila estiver vazia
    // todos ponteiros apontam para o elemento
    if(!(*queue)){
        *queue = elem;
        elem->next = elem;
        elem->prev = elem; 
    // se não, arrumamos de acordo com os elementos
    // já presentes
    } else {
        (*queue)->prev->next = elem;
        elem->next = *queue;
        elem->prev = (*queue)->prev;
        (*queue)->prev = elem;
    }
    return 0;
    
}

int queue_remove (queue_t **queue, queue_t *elem){
    if(!queue){
        fprintf(stderr, "ERRO: Fila não existe\n");
        return -1;
    }
    if(!elem){
        fprintf(stderr, "ERRO: Elemento a ser deletado não existe\n");
        return -2;
    }
    if(queue_size(*queue) == 0){
        fprintf(stderr, "ERRO: Fila vazia\n");
        return -3;
    }

    queue_t *primeiro = *queue;
    queue_t *proximo = primeiro;

    for(int i = 0; i < queue_size(*queue); i++){
        // se o elemento foi achado arrumamos seus ponteiros
        if(proximo == elem){

            // se o elemento a ser deletado for o primeiro,
            // temos que guardar como cabeça da fila o próximo elemento
            if(elem == primeiro){
                *queue = elem->next;
            }
            // se houver apenas um elemento, a fila fica vazia
            if(queue_size(*queue) == 1){
                *queue = NULL;
            // se não arrumamos os ponteiros
            } else {
                elem->prev->next = elem->next;
                elem->next->prev = elem->prev;
            }

            elem->next = NULL;
            elem->prev = NULL;
            
            // então retornamos o valor de sucesso
            return 0;
        }
        proximo = proximo->next;
    }

    // se saiu do laço não foi achado o elemento ao percorrer a fila
    fprintf(stderr, "ERRO: Elemento a ser deletado não pertence a fila\n");
    return -4;
}