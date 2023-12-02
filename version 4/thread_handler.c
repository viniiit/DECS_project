#include "hashmap.h"
#include "thread_handler.h"
#include <pthread.h>
#include "queue.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

extern int front;
extern int rear;
extern HashMap myMap;

pthread_mutex_t queue_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ready=PTHREAD_COND_INITIALIZER;

int no_req=0; //no of request avilable in the queue

void *workerthread(void *args)
{
  while(1)
  {
    long long ticket;
    int found=0;
    pthread_mutex_lock(&queue_lock);
    if(no_req == 0)
    {
      pthread_cond_wait(&ready,&queue_lock);
    }
    else if(no_req>0)
    {
      found=1;
      ticket=queue_delete();
      update_status(&myMap,ticket,1);
      --no_req;
      pthread_cond_signal(&ready);
    }
    pthread_mutex_unlock(&queue_lock);
    if(found)
    { 
      grader(ticket);
    }
  }
}

int grader(long long ticket)
{ 
  char filenamepath[256];
  sprintf(filenamepath,"%lld/%lld%s",ticket,ticket,".cpp");
  char buffer[256];
  char command[411];
  int n;
  bzero(buffer, 256);
  char outputfilepath[100],difffilepath[100],testfilepath[100],executable[100];
  //make file for the user
  // pthread_mutex_lock(&lock);
  // int j=i;
  // i=i+1;
  // pthread_mutex_unlock(&lock);

  sprintf(outputfilepath,"%lld/%lld-%s%s",ticket,ticket,"output",".txt");
  sprintf(difffilepath,"%lld/%s%s",ticket,"diff",".txt");
  sprintf(testfilepath,"%lld/%s%s",ticket,"output",".txt");

  char *act_out="1 2 3 4 5 6 7 8 9 10 ";
  int fd_t=open(testfilepath,O_CREAT|O_RDWR|O_TRUNC);
  write(fd_t,act_out,strlen(act_out));

  int fd_d=open(difffilepath,O_CREAT|O_RDWR|O_TRUNC);

  int fd_o;
  truncate(outputfilepath, 0);
  fd_o= open(outputfilepath,O_CREAT|O_RDWR|O_TRUNC);

  chmod(difffilepath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH); 
  chmod(outputfilepath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  chmod(testfilepath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

  //compile file if not success then the transfer the error into the output.txt
  memset(command,0,sizeof(command));
  sprintf(command,"g++ -w -o %lld/E%lld %s 2>%s",ticket,ticket,filenamepath,outputfilepath);
  int compile_success=system(command); 

  if(compile_success == 0)
  {
    //to clean buffer
    memset(command,0,sizeof(command));
    sprintf(command,"sh -c %lld/E%lld 1>%s 2>%s",ticket,ticket,outputfilepath,outputfilepath);
    
    //to check weather the run is successfully or not if not theen pass to the output.txt
    int run_success=system(command);
    if(run_success == 0)
    {
      char prog_out[512];

      //transfer the output in to the output.txt
      memset(command,0,sizeof(command));
      sprintf(command,"diff %s %s >%s",outputfilepath,testfilepath,difffilepath);
      system(command);
      
    }
  }
  
  update_status(&myMap,ticket,2);
  close(fd_o);
  close(fd_d);
  close(fd_t);
  return 0;
}

void masterfunction(HashMap *myMap,long long timestamp_ns)
{
  pthread_mutex_lock(&queue_lock);
  while((rear+1)%QUEUE_SIZE == front)
  {
    pthread_cond_wait(&ready,&queue_lock);
  }

  queue_insert(timestamp_ns);
  insert(myMap, timestamp_ns,0);
  no_req++;
  pthread_cond_signal(&ready);
  pthread_mutex_unlock(&queue_lock);
  return;
}