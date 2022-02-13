#include "queue.h"

#include <stdio.h>

int queue_size (queue_t *queue){
    int qt = 0;
    queue_t *primeiro = queue;
    queue_t *proximo = primeiro;

    if(!primeiro){
        return 0;
    } 

    do{
        proximo = proximo->next;
        qt++;
    } while(proximo != primeiro);
    return qt;
}


void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    queue_t *primeiro = queue;
    queue_t *proximo = primeiro;

    printf("%s [", name);

    for(int i = 0; i < queue_size(queue); i++){
        // um dos dois lol
        // print_elem(&(proximo));
        print_elem((void *) proximo);
        if(i < queue_size(queue) - 1)
            printf(" ");
        proximo = proximo->next;
    }
    printf("]\n");
}


int queue_append (queue_t **queue, queue_t *elem){
    if(!queue)
        return -1;
    if(!elem)
        return -2;
    if(elem->next || elem->prev){
        return -3;
    }

    if(!(*queue)){
        *queue = elem;
        elem->next = elem;
        elem->prev = elem; 
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
        return -1;
    }
    if(!elem){
        return -2;
    }
    if(queue_size(*queue) == 0){
        return -3;
    }

    queue_t *primeiro = *queue;
    queue_t *proximo = primeiro;

    for(int i = 0; i < queue_size(*queue); i++){
        if(proximo == elem){
            if(elem == primeiro){
                *queue = elem->next;
            }
            if(queue_size(*queue) == 1){
                *queue = NULL;
            } else {
                elem->prev->next = elem->next;
                elem->next->prev = elem->prev;
            }
            elem->next = NULL;
            elem->prev = NULL;
            
            return 0;
        }
        proximo = proximo->next;
    }

    // do{
    //     if(proximo == elem){
            
    //         elem->prev->next = elem->next;
    //         elem->next->prev = elem->prev;
    //         if(elem == primeiro){
    //             *queue = elem->next;
    //         }
    //         // erro ao deletar o ultimo da fila....
    //         printf("size: %d\n", queue_size(*queue));
    //         elem->next = NULL;
    //         elem->prev = NULL;
    //         return 0;
    //     }
    //     proximo = proximo->next;
    // } while(proximo != primeiro);

    return -4;
}