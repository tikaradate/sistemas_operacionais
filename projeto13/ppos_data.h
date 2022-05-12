// GRR20190367 Vinicius Tikara Venturi Date

// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.4 -- Janeiro de 2022

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

#define READY 1
#define FINISHED 2
#define SUSPENDED 3

#define FALSE 0
#define TRUE 1

// constante de prioridade dinâmica
#define ALFA -1

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  short static_priority;
  short dynamic_priority;
  short quantum;
  short preemptable ;			// pode ser preemptada?
  int start_time;
  int end_time;
  int activations;
  int processor_time;
  struct task_t *wait_queue;
  int wakeup_time;
  
   // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  struct queue_t *queue;
  int counter;
  int alive;
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  int msg_size;
  int max;
  int number_of_messages;
  void *queue;
  int start, end;
  semaphore_t s_buffer;
  semaphore_t s_positions;
  semaphore_t s_items;
} mqueue_t ;

#endif

