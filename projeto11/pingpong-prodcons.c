#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ppos.h"
#include "queue.h"

#define N 5

typedef struct item_t{
    struct item_t *prev, *next;
    int id;
} item_t ;

task_t prod1, prod2, prod3, cons1, cons2;

semaphore_t s_item;
semaphore_t s_buffer;
semaphore_t s_vaga;

queue_t *item_queue;

void produtor(void * arg){
    
    
    for(;;){
        task_sleep (1000);
        struct item_t *item = malloc(sizeof(item_t));
        item->id = rand() % 100;
        item->prev = NULL;
        item->next = NULL;

        sem_down (&s_vaga);
        sem_down (&s_buffer);
        queue_append(&item_queue, (queue_t *)item);
        sem_up (&s_buffer);

        sem_up (&s_item);
        printf("%s produziu item %d\n", (char *) arg, item->id);
   }
}

void consumidor(void * arg){
    struct item_t *item;
    for(;;){
        sem_down (&s_item);

        sem_down (&s_buffer);
        item = (item_t *) item_queue;
        queue_remove(&item_queue, item_queue);
        sem_up (&s_buffer);

        sem_up (&s_vaga);

        printf("%s consumiu item %d\n", (char *) arg, item->id);
        free(item);
        task_sleep (1000);
   }
}

int main(){
    sem_create(&s_item, 0);
    sem_create(&s_vaga, N);
    sem_create(&s_buffer, 1);

    ppos_init();
    //cria tasks
    task_create (&prod1, produtor, "p1") ;
    task_create (&prod2, produtor, "p2") ;
    task_create (&prod3, produtor, "p3") ;
    task_create (&cons1, consumidor, "                          c1");
    task_create (&cons2, consumidor, "                          c2");
    
    task_join(&prod1);
    task_exit(0);
    return 0;
}