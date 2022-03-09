// GRR20190367 Vinicius Tikara Venturi Date

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

#define STACKSIZE 64*1024

ucontext_t main_context;
task_t *main_task;
task_t *current_task;
int global_id;

queue_t *task_queue;
task_t *dispatcher_task;

void task_setprio(task_t *task, int prio){
    // evita sair da range possível de prioridades
    if(prio > 20){
        prio = 20;
    }
    if(prio < -20){
        prio = -20;
    }
    if(!task){
        #ifdef PRIO_DEBUG
        printf ("# alterando a prioridade da tarefa de id %d para %d\n", current_task->id, prio) ;
        #endif
        current_task->static_priority = prio;
        current_task->dynamic_priority = prio;
    } else {
        #ifdef PRIO_DEBUG
        printf ("# alterando a prioridade da tarefa de id %d para %d\n", task->id, prio) ;
        #endif
        task->static_priority = prio;
        task->dynamic_priority = prio;
    }
}

int task_getprio(task_t *task){
    if(!task){
        return current_task->static_priority;
    } else {
        return task->static_priority;
    }
}

task_t *scheduler(){
    task_t *next, *choosen;
    int priority = 21;

    next = (task_t *)task_queue;
    // procura a tarefa mais prioritária
    for(int i = 0; i < queue_size(task_queue); i++){
        if(next->dynamic_priority <= priority && next != dispatcher_task){
            choosen = next;
            priority = choosen->dynamic_priority;
        }
        next = next->next;
    }

    next = (task_t *)task_queue;
    // aumenta a prioridade de todas outras tarefas
    for(int i = 0; i < queue_size(task_queue); i++){
        if(choosen != next && next != dispatcher_task){
            next->dynamic_priority += ALFA;
        }
        next = next->next;
    }

    choosen->dynamic_priority = choosen->static_priority;
    queue_remove(&task_queue, (queue_t *)choosen);
    return choosen;
}

void dispatcher(){
    // o dispatcher também se encontra na fila, logo
    // precisamos contabilizar aqui também
    while(queue_size(task_queue) > 1){
        task_t *next;
        next = scheduler();

        if(next){
            task_switch(next);

            switch (next->status)
            {
            case READY:
                queue_append(&task_queue, (queue_t *)next);
                break;
            case FINISHED:
                free(next->context.uc_stack.ss_sp);
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

    #ifdef INIT_DEBUG
    printf ("# criou tarefa principal\n") ;
    #endif

    main_task = malloc(sizeof(task_t));
    if(!main_task)
    {
        perror ("Erro na criação da tarefa principal: ") ;
        exit (1) ;
    }

    main_task->context = main_context;
    main_task->id = 0;
    main_task->prev = NULL;
    main_task->next = NULL;

    current_task = main_task;
    global_id = 1;

    dispatcher_task = malloc(sizeof(task_t));
    if(!dispatcher_task)
    {
        perror ("Erro na criação da tarefa dispatcher: ") ;
        exit (1) ;
    }

    task_create(dispatcher_task, dispatcher, NULL);
}

int task_create (task_t *task, void (*start_routine)(void *),  void *arg) {
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
    task->dynamic_priority = 0;
    task->static_priority = 0;

    #ifdef CREATE_DEBUG
    printf ("# tarefa de id %d criada\n", task->id) ;
    #endif

    queue_append(&task_queue, (queue_t *) task);

    return task->id;
}

int task_switch (task_t *task){
    #ifdef SWITCH_DEBUG
    printf ("# vou mudar da tarefa %d para tarefa %d\n", current_task->id, task->id) ;
    #endif
    task_t *aux = current_task;
    current_task = task;
    swapcontext (&(aux->context), &(task->context));
    return 0;
}

void task_exit (int exit_code){
    #ifdef EXIT_DEBUG
    printf ("# task de id %d vai sair\n", current_task->id) ;
    #endif
    if(current_task == dispatcher_task){
        task_switch(main_task);
    } else {
        current_task->status = FINISHED;
        task_yield();
    }
}

int task_id (){
    return current_task->id;
}

void task_yield (){
    #ifdef YIELD_DEBUG
    printf ("# task de id %d dando controle para o dispatcher\n", current_task->id) ;
    #endif
    
    task_switch(dispatcher_task);
}
