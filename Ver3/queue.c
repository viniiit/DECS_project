#include "queue.h"
#include <stdio.h>
int front=0,rear=0;
int queue[QUEUE_SIZE];

void queue_insert(int newsockfd)
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
    queue[rear]=newsockfd;
  }

}

int queue_delete()
{
  int fd;
  if(front == rear)
  {
    printf("\nQueue is empty\n");
  }
  else{
    front = (front + 1)% QUEUE_SIZE;
    fd = queue[front];
    return fd;
  }
}