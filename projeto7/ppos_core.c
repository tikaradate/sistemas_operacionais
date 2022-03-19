#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

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
struct task_t *dispatcher_task;

// projeto 4
#define AGING -1

// projeto 5
struct itimerval timer;
struct sigaction action ;
unsigned short is_preemptable;
#define FALSE 0
#define TRUE 1

// projeto 6
int time_elapsed;
time_t time;

unsigned int systime(){
    return time_elapsed;
}

void task_setprio(task_t *task, int prio){
    if(prio > 20){
        prio = 20;
    }
    if(prio < -20){
        prio = -20;
    }
    if(!task){
        #ifdef PRIO_DEBUG
        printf ("# alterando a prioridade da tarefa %d para %d\n", current_task->id, prio) ;
        #endif
        current_task->static_priority = prio;
    } else {
        #ifdef PRIO_DEBUG
        printf ("# alterando a prioridade da tarefa %d para %d\n", task->id, prio) ;
        #endif
        task->static_priority = prio;
    }
}

int task_getprio(task_t *task){
    if(!task){
        return current_task->static_priority;
    } else {
        return task->static_priority;
    }
}

struct task_t *scheduler(){
    struct task_t *next, *choosen;
    next = (task_t*) task_queue;
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

    queue_remove(&task_queue, (queue_t *) choosen);
    choosen->quantum = 20;
    choosen->activations++;
    return choosen;
}

void print_elem (void *ptr)
{
   task_t *elem = ptr ;

   if (!elem)
      return ;

   elem->prev ? printf ("%d", elem->prev->id) : printf ("*") ;
   printf ("<%d>", elem->id) ;
   elem->next ? printf ("%d", elem->next->id) : printf ("*") ;
}

void dispatcher(){
    
    while(queue_size(task_queue) > 0){
        #ifdef DISP_DEBUG
        queue_print("fila de tarefas: ", task_queue, print_elem);
        #endif
        struct task_t *next;
        next = scheduler();
        #ifdef DISP_DEBUG
        printf("Próxima tarefa: %d\n", next->id );
        #endif

        dispatcher_task->activations++;

        if(next != NULL && next != dispatcher_task){
            
            task_switch(next);

            switch (next->status)
            {
            case READY:
                queue_append(&task_queue, (queue_t *)next);
                break;
            case FINISHED:
                next->end_time = time_elapsed;
                printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", 
                        next->id, next->end_time - next->start_time, next->processor_time, next->activations);
                free(next->context.uc_stack.ss_sp);
                break;
            default:
                break;
            }
        }
    }
    task_exit(0);
}

void tratador(int signum){
    // alarme tocou
    // if(signum == 14)
    time_elapsed++;
    current_task->processor_time++;
    if(current_task->preemptable){
        current_task->quantum--;
        if(current_task->quantum == 0){
            task_yield();
        }
    }
}

void ppos_init (){
    char *stack ;

    // botar numa função as coisas de timer 
    action.sa_handler = tratador ;
    sigemptyset (&action.sa_mask) ;
    action.sa_flags = 0 ;
    if (sigaction (SIGALRM, &action, 0) < 0)
    {
        perror ("Erro em sigaction: ") ;
        exit (1) ;
    }

    timer.it_value.tv_usec = 1000 ;      // primeiro disparo, em micro-segundos
    timer.it_value.tv_sec  = 0 ;      // primeiro disparo, em segundos
    timer.it_interval.tv_usec = 1000;   // disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec  = 0 ;   // disparos subsequentes, em segundos

    if (setitimer (ITIMER_REAL, &timer, 0) < 0)
    {
        perror ("Erro em setitimer: ") ;
        exit (1) ;
    }


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
    printf ("# task_create: criou tarefa principal\n") ;
    #endif

    main_task = malloc(sizeof(struct task_t));

    main_task->id = 0;
    main_task->context = main_context;
    main_task->status = READY;
    main_task->dynamic_priority = 0;
    main_task->static_priority = 0;
    main_task->quantum = 0;
    main_task->preemptable = TRUE;
    main_task->start_time = time_elapsed;
    main_task->processor_time = 0;
    main_task->activations = 0;

    queue_append(&task_queue, main_task);

    current_task = main_task;
    global_id = 1;
    time_elapsed = 0;

    
    dispatcher_task = malloc(sizeof(task_t));
    if(!dispatcher_task)
    {
        perror ("Erro na criação da tarefa dispatcher: ") ;
        exit (1) ;
    }
    task_create(dispatcher_task, dispatcher, NULL);
    dispatcher_task->preemptable = FALSE;

    
    task_yield();
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
    task->dynamic_priority = 0;
    task->static_priority = 0;
    task->quantum = 0;
    task->preemptable = TRUE;
    task->start_time = time_elapsed;
    task->processor_time = 0;
    task->activations = 0;
    
    #ifdef CREATE_DEBUG
    printf ("# task_create: criou tarefa %d\n", task->id) ;
    #endif

    queue_append(&task_queue, (queue_t *) task);

    return task->id;
}

int task_switch (task_t *task){
    // checar erros
    #ifdef DEBUG
    printf ("# vou tentar mudar da tarefa %d para tarefa %d\n", current_task->id, task->id) ;
    #endif
    struct task_t *aux = current_task;
    current_task = task;
    swapcontext (&(aux->context), &(task->context));
    #ifdef DEBUG
    printf ("# mudou para a tarefa de id %d\n", aux->id) ;
    #endif
    return 0;
}

void task_exit (int exit_code){
    #ifdef DEBUG
    printf ("# quero sair da task %d\n", current_task->id) ;
    #endif
    if(current_task == dispatcher_task){
        current_task->end_time = time_elapsed;
        printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", 
        current_task->id, current_task->end_time - current_task->start_time, current_task->processor_time, current_task->activations);
        free(current_task->context.uc_stack.ss_sp);
        task_switch(main_task);
    } else {
        current_task->status = FINISHED;
        current_task->end_time = time_elapsed;
        task_yield();
    }
    #ifdef DEBUG
    printf ("# consegui sair da task %d\n", current_task->id) ;
    #endif
}

int task_id (){
    return current_task->id;
}

void task_yield (){
    #ifdef YIELD_DEBUG
    printf ("# task %d dando controle para o dispatcher\n", current_task->id) ;
    #endif
    
    task_switch(dispatcher_task);
}
