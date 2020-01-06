#ifndef	__SCHED_IMPL__H__
#define	__SCHED_IMPL__H__
#include "list.h"
#include <semaphore.h>
#include <pthread.h>
struct thread_info {
	/*...Fill this in...*/
	
	sem_t execute;
	list_elem_t *threadElt;
	sched_queue_t *queue;
};

struct sched_queue {
	/*...Fill this in...*/
	list_t *listQ;
	int sizeQ;
	pthread_mutex_t lock;
	sem_t readyToAdd;
	sem_t readyToExecute;
	sem_t release;
	/* type 0 is fifo and type 1 is rr*/
	int type;
};

#endif /* __SCHED_IMPL__H__ */
