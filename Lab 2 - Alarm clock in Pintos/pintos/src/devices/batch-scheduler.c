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

#define BUS_CAPACITY 3
#define SENDER 0
#define RECEIVER 1
#define NORMAL 0
#define HIGH 1


struct condition direction[4];
int priority[2];
struct lock mutex;
int currentDir;
int space;

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

    lock_init(&mutex);
    int i;
    for(i = 0; i < 4; i++){
        cond_init(&direction[i]);
    }
    currentDir = SENDER;
    space = 0;
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
void getSlot(task_t task) {
    lock_acquire(&mutex);
    if(space == 3 || (task.direction != currentDir) && space > 0) { // if full or wrong direction we must wait
        if(task.priority){ //high priority = 1
            priority[task.direction] += 1;                      // increment priority list for sender/reciever
            cond_wait(&direction[2 + task.direction],&mutex);   // add high prio sender or reciever to waiting
            priority[task.direction] -=  1;                     // decrement priority list for sender/reciever
        } else { // low priority = 0
			cond_wait(&direction[task.direction],&mutex); // add low prio sender or reciever to waiting list
		}
    }
    currentDir = task.direction;
    space++;
    lock_release(&mutex);
}

/* task processes data on the bus send/receive */
void transferData(task_t task) 
{
    timer_msleep(random_ulong() % 100);
}

/* task releases the slot */
void leaveSlot(task_t task)  {
    lock_acquire(&mutex);
    space--;

    if(priority[task.direction]){                               // if any high priorities in my direction
        cond_signal(&direction[2 + task.direction],&mutex); 
    } else if (priority[1 - task.direction]) {                  // if any high priorities in other direction
        cond_signal(&direction[3 - task.direction],&mutex); 
    } else {                                                    // otherwise signal low priority my direction
        cond_signal(&direction[task.direction],&mutex); 
    }
    lock_release(&mutex);
}
