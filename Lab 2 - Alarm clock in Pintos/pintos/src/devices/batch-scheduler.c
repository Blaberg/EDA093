/* Tests cetegorical mutual exclusion with different numbers of threads.
 * Automatic checks only catch severe problems like crashes.
 */
#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "lib/random.h" //generate random numbers
#include "timer.h"
#include "threads/synch.h"

#define BUS_CAPACITY 3
#define SENDER 0
#define RECEIVER 1
#define NORMAL 0
#define HIGH 1

int space;
struct condition waiting[2][2];
struct lock mutex;
struct condition send;
struct condition recv;
int waitingTasks[2][2];
int currentDirection = SENDER;

/*
 *	initialize task with direction and priority
 *	call o
 * */
typedef struct {
	int direction;
	int priority;
} task_t;

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive);

void senderTask(void *);
void receiverTask(void *);
void senderPriorityTask(void *);
void receiverPriorityTask(void *);


void oneTask(task_t task);/*Task requires to use the bus and executes methods below*/
	void getSlot(task_t task); /* task tries to use slot on the bus */
	void transferData(task_t task); /* task processes data on the bus either sending or receiving based on the direction*/
	void leaveSlot(task_t task); /* task release the slot */



/* initializes semaphores */
void init_bus(void){

    random_init((unsigned int)123456789);
    int n,m;
    for( n = 0; n<2; n++){
        for( m = 0; m<2; m++){
            cond_init(&waiting[n][m]);
            &waitingTasks[n][m] = 0;
        }
    }
    space = BUS_CAPACITY;
    lock_init(&mutex);
    cond_init(&send);
    cond_init(&recv);
}

/*
 *  Creates a memory bus sub-system  with num_tasks_send + num_priority_send
 *  sending data to the accelerator and num_task_receive + num_priority_receive tasks
 *  reading data/results from the accelerator.
 *
 *  Every task is represented by its own thread. 
 *  Task requires and gets slot on bus system (1)
 *  process data and the bus (2)
 *  Leave the bus (3).
 */

void batchScheduler(unsigned int num_tasks_send, unsigned int num_task_receive,
        unsigned int num_priority_send, unsigned int num_priority_receive)
{
    unsigned int i;
    /* create sender threads */
    for(i = 0; i < num_tasks_send; i++)
        thread_create("sender_task", 1, senderTask, NULL);

    /* create receiver threads */
    for(i = 0; i < num_task_receive; i++)
        thread_create("receiver_task", 1, receiverTask, NULL);

    /* create high priority sender threads */
    for(i = 0; i < num_priority_send; i++)
       thread_create("prio_sender_task", 1, senderPriorityTask, NULL);

    /* create high priority receiver threads */
    for(i = 0; i < num_priority_receive; i++)
       thread_create("prio_receiver_task", 1, receiverPriorityTask, NULL);
}

/* Normal task,  sending data to the accelerator */
void senderTask(void *aux UNUSED){
        task_t task = {SENDER, NORMAL};
        oneTask(task);
}

/* High priority task, sending data to the accelerator */
void senderPriorityTask(void *aux UNUSED){
        task_t task = {SENDER, HIGH};
        oneTask(task);
}

/* Normal task, reading data from the accelerator */
void receiverTask(void *aux UNUSED){
        task_t task = {RECEIVER, NORMAL};
        oneTask(task);
}

/* High priority task, reading data from the accelerator */
void receiverPriorityTask(void *aux UNUSED){
        task_t task = {RECEIVER, HIGH};
        oneTask(task);
}

/* abstract task execution*/
void oneTask(task_t task) {
  getSlot(task);
  transferData(task);
  leaveSlot(task);
}


/* task tries to get slot on the bus subsystem */
void getSlot(task_t task)
{
    lock_acquire(&mutex);
    &waitingTasks[task.direction][task.priority] += 1;
    while(space < 1 || task.direction !=currentDirection || (task.priority == NORMAL && (waitingTasks[0][HIGH] || waitingTasks[1][HIGH]))) {
        cond_wait(&waiting[task.direction][task.priority], &mutex);
    }
    &waitingTasks[task.direction][task.priority] -= 1;
    space--;
    currentDirection=task.direction;
    lock_release(&mutex);

}

/* task processes data on the bus send/receive */
void transferData(task_t task)
{
    timer_msleep(random_ulong() % 1000);
}

/* task releases the slot */
void leaveSlot(task_t task)
{
   lock_acquire(&mutex);
   space++;
   if(&waitingTasks[currentDirection][HIGH]){
       cond_signal(&waiting[currentDirection][HIGH],&mutex);
   }else if(&waitingTasks[1-currentDirection][HIGH]){
        cond_signal(&waiting[1-currentDirection][HIGH],&mutex);
    }else if(&waitingTasks[currentDirection][NORMAL]){
        cond_signal(&waiting[currentDirection][NORMAL],&mutex);
    }else if(&waitingTasks[1-currentDirection][NORMAL]){
        cond_signal(&waiting[1-currentDirection][NORMAL],&mutex);
    }else{
    cond_broadcast(&waiting[currentDirection][NORMAL], &mutex);
}
lock_release(&mutex);

}
