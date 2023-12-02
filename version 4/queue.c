#include "queue.h"
#include <stdio.h>
#include <pthread.h>
int front=0,rear=0;
long long queue[QUEUE_SIZE];

extern pthread_mutex_t queue_lock;

void queue_insert(long long ticket)
{
  rear=(rear+1)%QUEUE_SIZE;
  if(rear==front)
  {
    printf("\nOverflow\n");

    if(rear==0)
    {
      rear=QUEUE_SIZE-1;
    }
    else
    {
      --rear;
    }
  }
  else
  {
    queue[rear]=ticket;
  }

}

long long queue_delete()
{
  long long ticket;
  if(front == rear)
  {
    printf("\nQueue is empty\n");
  }
  else{
    front = (front + 1)% QUEUE_SIZE;
    ticket = queue[front];
    return ticket;
  }
}

int know_position(long ticket)
{
  pthread_mutex_lock(&queue_lock);
  int i=front,pos=1;

  for(i;i<=rear;i++)
  {
    if(queue[i]==ticket){
    printf("current position:%d",pos);
    return pos;
    }
    else
    pos++;
  }
  pthread_mutex_unlock(&queue_lock);
}