#include "scheduler.h"
#include "sched_impl.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fill in your scheduler implementation code below: */

/* initialie thread info */
static void init_thread_info(thread_info_t *info, sched_queue_t *queue)
{
	/*...Code goes here...*/
	info->queue = queue;
	info->threadElt = NULL;
	if (sem_init(&info->execute, 0, 0) == -1) {
		printf("thread info init fails\n");
	}
}

/* clean up thread info */
static void destroy_thread_info(thread_info_t *info)
{
	/*...Code goes here...*/
	info->queue = NULL;
	free(info->queue);
	info->threadElt = NULL;
	free(info->threadElt);
	if (sem_destroy(&info->execute) == -1 ) {
		printf("fail to destroy semaphore in thread\n");
	}
}

/* insert thread to sheduler queue*/
static void enter_sched_queue(thread_info_t *info)
{
	usleep(70000);
	info->threadElt = malloc(sizeof(list_elem_t));
	list_elem_init(info->threadElt, info);
	if (sem_wait(&info->queue->readyToAdd) == 0) {
		pthread_mutex_lock(&info->queue->lock);
		list_insert_tail(info->queue->listQ, info->threadElt);
		/* there is thread in scheduler queue for execution */
		sem_post(&info->queue->readyToExecute);
		pthread_mutex_unlock(&info->queue->lock);
	} else {
		printf("enter schedule queue fails\n");
	}
}

/* remove thread from scheduler queue */
static void leave_sched_queue(thread_info_t *info) 
{
	if (sem_wait(&info->queue->readyToExecute) == 0) {
		pthread_mutex_lock(&info->queue->lock);
		list_remove_elem(info->queue->listQ, info->threadElt);
		free(info->threadElt);
		/* there is space to add another thread */
		sem_post(&info->queue->readyToAdd);
		pthread_mutex_unlock(&info->queue->lock);
	} else {
		printf("leave schedule queue fails\n");
	}
}

/* wait for current thread */
static void wait_for_cpu(thread_info_t *info) 
{
	if (sem_wait(&info->execute) == -1) {
		printf("fail to wait for cpu\n");
	}
}

/* current thread relinquish cpu */
static void release_cpu(thread_info_t *info)
{
	if (sem_post(&info->queue->release) == -1) {
		printf("fail to release cpu\n");
	}
	usleep(40000);
}


/*...More functions go here...*/
/* initialie scheduler queue */
static void init_sched_queue(sched_queue_t *queue, int queue_size)
{
	/*...Code goes here...*/
	queue->listQ = malloc(sizeof(list_t));
	queue->sizeQ = queue_size;
	list_init(queue->listQ);
	if (sem_init(&queue->readyToExecute, 0, 0) == -1 || sem_init(&queue->readyToAdd, 0, queue_size) == -1 || sem_init(&queue->release, 0, 1) == -1)
		printf("schedule queue init fails\n");
	if (pthread_mutex_init(&queue->lock, NULL) != 0) {
		printf("queue lock init fails\n");
	}
}

/* clean up scheduler queue */
static void destroy_sched_queue(sched_queue_t *queue)
{
	/*...Code goes here...*/
	if (sem_destroy(&queue->readyToAdd) == -1 || sem_destroy(&queue->readyToExecute) == -1 || sem_destroy(&queue->release) == -1) {
		printf("fail to destroy semaphore in queue\n");
	}
	if (pthread_mutex_destroy(&queue->lock) != 0) {
		printf("fail to destroy mutex in queue\n");
	}
	free(queue->listQ);
}

/* notify thread to be ready for execution */
static void wake_up_worker(thread_info_t *info)
{
	if (sem_post(&info->execute) == -1) {
		printf("wake up worker fails\n");
	}
}

/* wait for current thread */
static void wait_for_worker(sched_queue_t *queue)
{
	/*list_elem_t *nextElt;
	thread_info_t *nextWorker;
	nextElt = list_get_head(queue->listQ);
	if (nextElt == NULL) {
		return;
	}
	nextWorker = (thread_info_t*) nextElt->datum;
	if (sem_wait(&nextWorker->release) == -1) {
		printf("wait for worker fails\n");
	}*/
	if (sem_wait(&queue->release) == -1) {
		printf("wait for worker fails\n");
	}
	
}

/* select next worker to execute for FIFO */
static thread_info_t *next_worker(sched_queue_t *queue)
{
	thread_info_t *nextWorker;
	list_elem_t *nextElt;
	pthread_mutex_lock(&queue->lock);
	nextElt = list_get_head(queue->listQ);
	if (nextElt != NULL) {
		nextWorker = (thread_info_t *) nextElt->datum;
	} else {
		pthread_mutex_unlock(&queue->lock);
		return;
	}
	pthread_mutex_unlock(&queue->lock);
	return nextWorker;
}

/* select next worker to execute for RR */
static thread_info_t *next_worker_rr(sched_queue_t *queue) {
	thread_info_t *nextWorker;
	list_elem_t *nextElt;
	pthread_mutex_lock(&queue->lock);
	nextElt = list_get_head(queue->listQ);
	if (nextElt != NULL) {
		list_remove_elem(queue->listQ, nextElt);
		list_insert_tail(queue->listQ, nextElt);
		nextElt = list_get_head(queue->listQ);
		nextWorker = (thread_info_t *) nextElt->datum;
	} else {
		pthread_mutex_unlock(&queue->lock);
		return;
	}
	pthread_mutex_unlock(&queue->lock);
	return nextWorker;
}

/* wait until there is thread in scheduler queue */
static void wait_for_queue(sched_queue_t *queue)
{
	while(!list_size(queue->listQ)) {
		sched_yield();
	}
}
/*...More functions go here...*/

/* You need to statically initialize these structures: */
sched_impl_t sched_fifo = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu, release_cpu  /*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker, wait_for_queue/*, ...etc... */ } },
sched_rr = {
	{ init_thread_info, destroy_thread_info, enter_sched_queue, leave_sched_queue, wait_for_cpu, release_cpu  /*, ...etc... */ }, 
	{ init_sched_queue, destroy_sched_queue, wake_up_worker, wait_for_worker, next_worker_rr, wait_for_queue/*, ...etc... */ } };
