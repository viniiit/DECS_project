#include "thread_handler.h"
#include <pthread.h>
#include "queue.h"
extern int front;
extern int rear;

pthread_mutex_t queue_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ready=PTHREAD_COND_INITIALIZER;

int no_fd=0; //no of thread avilable in the queue

void *workerthread(void *args)
{
  while(1)
  {
    int newsockfd;
    int found=0;
    pthread_mutex_lock(&queue_lock);
    if(no_fd == 0)
    {
      pthread_cond_wait(&ready,&queue_lock);
    }
    if(no_fd>0)
    { 
      found=1;
      newsockfd=queue_delete();
      --no_fd;
      pthread_cond_signal(&ready);
    }
    pthread_mutex_unlock(&queue_lock);
    if(found)
    {
      start_function(newsockfd);
    }
  }
}

void masterfunction(int newsockfd)
{
  pthread_mutex_lock(&queue_lock);
  while((rear+1)%QUEUE_SIZE == front)
  {
    pthread_cond_wait(&ready,&queue_lock);
  }

  queue_insert(newsockfd);
  no_fd++;
  pthread_cond_signal(&ready);
  pthread_mutex_unlock(&queue_lock);

}