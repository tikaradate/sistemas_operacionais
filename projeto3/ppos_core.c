#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

#define STACKSIZE 64*1024

// projeto 2
ucontext_t main_context;
struct task_t *main_task;
struct task_t *current_task;
int global_id;
int number_of_tasks;

// projeto 3
struct queue_t *task_queue;
struct task_t dispatcher_task;


struct task_t *scheduler(){
    struct task_t *next;
    next = task_queue;
    queue_remove(&task_queue, (queue_t *) next);
    return next;
}

void dispatcher(){
    while(queue_size(task_queue) > 1){
        struct task_t *next;
        next = scheduler();

        if(next){
            task_switch(next);

            switch (next->status)
            {
            case READY:
                queue_append(&task_queue, (queue_t *)next);
                break;
            case FINISHED:
                
                break;
            default:
                break;
            }
        }
    }
    task_exit(0);
}

void ppos_init (){
    char *stack ;

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
    getcontext (&main_context) ;

    stack = malloc (STACKSIZE) ;
    if (stack)
    {
        main_context.uc_stack.ss_sp = stack ;
        main_context.uc_stack.ss_size = STACKSIZE ;
        main_context.uc_stack.ss_flags = 0 ;
        main_context.uc_link = 0 ;
    }
    else
    {
        perror ("Erro na criação da pilha: ") ;
        exit (1) ;
    }

    #ifdef DEBUG
    printf ("# # # task_create: criou tarefa principal\n") ;
    #endif

    main_task = malloc(sizeof(struct task_t));

    main_task->context = main_context;
    main_task->id = 0;
    main_task->prev = NULL;
    main_task->next = NULL;

    current_task = main_task;
    global_id = 1;


    task_create(&dispatcher_task, dispatcher, NULL);
}

int task_create (task_t *task, void (*start_routine)(void *),  void *arg) {
    // acho que tem que ser um ponteiro para existir depois da função acabar
    ucontext_t task_context;
    char *stack ;

    getcontext (&task_context) ;

    stack = malloc (STACKSIZE) ;
    if (stack)
    {
        task_context.uc_stack.ss_sp = stack ;
        task_context.uc_stack.ss_size = STACKSIZE ;
        task_context.uc_stack.ss_flags = 0 ;
        task_context.uc_link = 0 ;
    }
    else
    {
        perror ("Erro na criação da pilha: ") ;
        exit (1) ;
    }

    makecontext (&task_context, (void *)start_routine, 1, arg) ;

    task->id = global_id++;
    task->context = task_context;
    task->status = READY;

    #ifdef DEBUG
    printf ("# # # task_create: criou tarefa %d\n", task->id) ;
    #endif

    queue_append(&task_queue, (queue_t *) task);

    return task->id;
}

int task_switch (task_t *task){
    // checar erros
    #ifdef DEBUG
    printf ("# # # vou tentar mudar da tarefa %d para tarefa %d\n", current_task->id, task->id) ;
    #endif
    struct task_t *aux = current_task;
    current_task = task;
    swapcontext (&(aux->context), &(task->context));
    #ifdef DEBUG
    printf ("# # # mudou para a tarefa de id %d\n", aux->id) ;
    #endif
    return 0;
}

void task_exit (int exit_code){
    #ifdef DEBUG
    printf ("# # # quero sair da task %d\n", current_task->id) ;
    #endif
    if(current_task == &dispatcher_task){
        task_switch(main_task);
    } else {
        current_task->status = FINISHED;
        task_yield();
    }
    #ifdef DEBUG
    printf ("# # # consegui sair da task %d\n", current_task->id) ;
    #endif
}

int task_id (){
    return current_task->id;
}


void task_yield (){
    #ifdef DEBUG
    printf ("# # # task %d dando controle para o dispatcher\n", current_task->id) ;
    #endif
    
    task_switch(&dispatcher_task);
}
