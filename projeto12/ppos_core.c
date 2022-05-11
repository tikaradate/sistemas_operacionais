// GRR20190367 Vinicius Tikara Venturi Date

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "ppos.h"
#include "queue.h"

#define STACKSIZE 64*1024

ucontext_t main_context;
struct task_t *main_task;
struct task_t *current_task;
int global_id;
struct queue_t *task_queue;
struct task_t *dispatcher_task;
struct itimerval timer;
struct sigaction action ;
int time_elapsed;
time_t time;
struct queue_t *sleep_queue;
int number_of_tasks;

unsigned int systime(){
    return time_elapsed;
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

void task_sleep (int t){
    current_task->status = SUSPENDED;
    current_task->wakeup_time = t + systime();
    task_suspend((task_t **) &sleep_queue);
}

void track_time(){
    if(queue_size(sleep_queue) > 0){
        task_t *next = (task_t*) sleep_queue;
        for(int i = 0; i < queue_size(sleep_queue); i++){
            if(next->wakeup_time - time_elapsed <= 0){
                next = next->next;
                task_resume(next->prev, (task_t **) &sleep_queue);
                
            } else { 
                next = next->next;
            }
        } 
    }
}

int task_join (task_t *task){
    if(!task){
        return -1;
    }
    if(task->status == FINISHED){
        return task->id;
    }
    
    task_suspend(&task->wait_queue);

    return task->id;
}

void task_suspend (task_t **queue) {
    current_task->status = SUSPENDED;
    queue_append((queue_t **) queue, (queue_t *)current_task);
    task_yield();
}

void task_resume (task_t * task, task_t **queue) {
    queue_remove((queue_t **) queue,(queue_t *) task);
    task->status = READY;
    queue_append(&task_queue,(queue_t *)  task);
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
        if(next->dynamic_priority < priority && next != dispatcher_task){
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

void dispatcher(){
    
    while(number_of_tasks > 0){
        
        struct task_t *next = NULL;
        if(queue_size(task_queue) > 0){
            next = scheduler();
        }

        dispatcher_task->activations++;

        // if(queue_size(task_queue) == 0)
        //     sleep(0.100);

        track_time();
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
            case SUSPENDED:
                continue;
                break;
            default:
                break;
            }
        }
    }
    task_exit(0);
}

void tratador(int signum){
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

    queue_append(&task_queue, (queue_t *) main_task);

    current_task = main_task;
    global_id = 1;
    time_elapsed = 0;
    number_of_tasks = 0;
    
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
    
    number_of_tasks++;

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
    number_of_tasks--;
    if(current_task == dispatcher_task){
        current_task->end_time = time_elapsed;
        printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", 
        current_task->id, current_task->end_time - current_task->start_time, current_task->processor_time, current_task->activations);
        free(current_task->context.uc_stack.ss_sp);
    } else {
        current_task->status = FINISHED;
        current_task->end_time = time_elapsed;
        for(int i = 0; i < queue_size((queue_t *)current_task->wait_queue); i++){
            task_resume(current_task->wait_queue, &current_task->wait_queue);
        }
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

int lock = 0;

void enter_cs (int *lock)
{
  // atomic OR (Intel macro for GCC)
  while (__sync_fetch_and_or (lock, 1)) ;   // busy waiting
}
 
void leave_cs (int *lock)
{
  (*lock) = 0 ;
}
 
int sem_create (semaphore_t *s, int value){
    if(!s) return -1;

    s->counter = value;
    s->queue = NULL;
    s->alive = TRUE;
    return 0;
}

int sem_down (semaphore_t *s){
    if(!s || !s->alive){
        return -1;
    }
    enter_cs(&lock);
    s->counter--;
    leave_cs(&lock);

    if(s->counter < 0)
        task_suspend((task_t **)&s->queue);

    return 0; 
}

int sem_up (semaphore_t *s){
    if(!s || !s->alive){
        return -1;
    }

    enter_cs(&lock);
    s->counter++;
    leave_cs(&lock);

    if(s->counter <= 0)
        task_resume((task_t *) s->queue, (task_t **)&s->queue);

    return 0;
}

int sem_destroy (semaphore_t *s){
    while(s->queue){
        task_resume((task_t *)s->queue, (task_t **)&s->queue);
    }
    s->alive = FALSE;
    return 0;
}

int mqueue_create (mqueue_t *queue, int max_msgs, int msg_size){
    queue->max = max_msgs;
    queue->msg_size = msg_size;
    queue->queue = calloc(max_msgs, msg_size);
    queue->start = 0;
    queue->end = 0;
    queue->number_of_messages = 0;
    sem_create(&queue->s_buffer, 1);
    sem_create(&queue->s_positions, max_msgs);
    sem_create(&queue->s_items, 0);
    return 0;
}

int mqueue_send (mqueue_t *queue, void *msg){
    sem_down (&queue->s_positions);
    sem_down (&queue->s_buffer);
    if(!queue->queue) return -1;

    int offset = queue->end*queue->msg_size;
    memcpy(queue->queue + offset, msg, queue->msg_size);
    queue->end = (queue->end + 1) % queue->max;
    queue->number_of_messages++;
    
    sem_up(&queue->s_buffer);
    sem_up(&queue->s_items);

    return 0;
}

int mqueue_recv (mqueue_t *queue, void *msg){
    sem_down (&queue->s_items);
    sem_down (&queue->s_buffer);

    if(!queue->queue) return -1;

    int offset = queue->start*queue->msg_size;
    memcpy(msg, queue->queue + offset, queue->msg_size);
    queue->start = (queue->start + 1) % queue->max;
    queue->number_of_messages--;

    sem_up(&queue->s_buffer);
    sem_up(&queue->s_positions);

    return 0;
}

int mqueue_destroy (mqueue_t *queue){
    free(queue->queue);
    queue->queue = NULL;
    sem_destroy(&queue->s_buffer);
    sem_destroy(&queue->s_positions);
    sem_destroy(&queue->s_items);
    return 0;
}

int mqueue_msgs (mqueue_t *queue){
    return queue->number_of_messages;
}