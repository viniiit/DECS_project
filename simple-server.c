#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include "fault_tolerance.h"
#include "common.h"
// #include "hashmap.h"
#include "thread_handler.h"
#include <sys/stat.h>
#include <time.h>
// #include "queue.h"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ac = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t diff_file_lock = PTHREAD_MUTEX_INITIALIZER;
HashMap myMap;



int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  struct timespec ts;
  int thread_pool_size;
  char filename[256];
  char syscommand[256];
  char buffer[256];

  if (argc < 3)
  {
    fprintf(stderr, "ERROR, Usage will be ./server <port> <thread_pool_size>");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */
  listen(sockfd, 500);
  clilen = sizeof(cli_addr);

  thread_pool_size = atoi(argv[2]);
  /* accept a new request, create a newsockfd */
  pthread_t thread[thread_pool_size];
  pthread_t fault_tol;

  for (int i = 0; i < SIZE; i++)
  {
    myMap.array[i] = NULL;
  }

  if (pthread_create(&fault_tol, NULL, &periodicSave, NULL) != 0)
  {
    printf("Error : Fault_tolerance thread creation Problem");
  }

  // thread creation and pass to the workerthread function which handle the all thread
  for (int i = 0; i < thread_pool_size; i++)
  {
    if (pthread_create(thread + i, NULL, &workerthread, NULL) != 0)
    {
      printf("Error :thread creation Problem");
    }
  }

  while (1)
  {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

    int type;
    read(newsockfd, &type, sizeof(int));
    if (type == 0)
    {
      if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
      {
        perror("Error getting timestamp");
        return 1;
      }

      long long timestamp_ns = ts.tv_sec * 1000000000LL + ts.tv_nsec;

      sprintf(syscommand, "%lld", timestamp_ns);
      if (mkdir(syscommand, S_IRWXU) != 0)
      {
        perror("Error creating directory");
        return 1;
      }

      sprintf(filename, "%lld%s", timestamp_ns, ".cpp");

      if (chdir(syscommand) != 0)
      {
        perror("Error changing directory");
        return -1;
      }

      int filedesc = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
      chmod(filename, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

      int no_of_bytes;
      read(newsockfd, &no_of_bytes, sizeof(int));

      do
      {
        int file_bytes;
        file_bytes = 0;

        n = read(newsockfd, buffer, sizeof(buffer));
        file_bytes = file_bytes + n;
        if (n < 0)
        {
          error("Error reading from socket");
          return -1;
        }

        int bytes_written = write(filedesc, buffer, n);
        if (bytes_written < 0)
        {
          error("Error writing to file");
          return -1;
        }

        if (file_bytes == no_of_bytes)
        {
          break;
        }

        memset(buffer, 0, sizeof(buffer));
      } while (1);

      chdir("..");

      masterfunction(&myMap, timestamp_ns);

      write(newsockfd, &timestamp_ns, sizeof(long long));
      close(newsockfd);
    }

    else
    {

      long long ticket;
      int len;
      char message[131], difffilepath[100], outputfilepath[100], command[30],buffer[1000];

      read(newsockfd, &ticket, sizeof(long long));
      int status = know_status(&myMap, ticket);
      printf("%d",status);
      bzero(message, sizeof(message));

      if (status == 0)
      {
        sprintf(message, "Your grading request ID %lld has been accepted. It is currently at  position %d  in the queue.", ticket, know_position(ticket));
        puts(message);
        len=strlen(message);
        write(newsockfd,&len,sizeof(int));
        write(newsockfd, message, len);
      }
      else if (status == 1)
      {
        sprintf(message, "Your grading request ID %lld has been accepted and is currently being processed.", ticket);
        puts(message);
        len=strlen(message);
        write(newsockfd,&len,sizeof(int));
        write(newsockfd, message, len);
      }
      else if (status == 2)
      {
        printf("%d",status);
        sprintf(difffilepath, "%lld/%s%s", ticket, "diff", ".txt");
        sprintf(outputfilepath, "%lld/%lld-%s%s", ticket, ticket, "output", ".txt");
        int fd_d = open(difffilepath, O_CREAT | O_RDWR);
        int fd_o = open(outputfilepath, O_CREAT | O_RDWR);

        int size_d=lseek(fd_d,0,SEEK_END);
        int size_o=lseek(fd_o,0,SEEK_END);
        
        lseek(fd_d,0,SEEK_SET);
        lseek(fd_o,0,SEEK_SET);
        if(size_d==0)
        {
          write(newsockfd,&size_o,sizeof(int));
          n = read(fd_o, buffer, size_o);
          write(newsockfd,buffer,n);
        }
        else{
          write(newsockfd,&size_d,sizeof(int));
          n = read(fd_d, buffer, size_d);
          write(newsockfd,buffer,n);
        }

        sprintf(command, "rm -R %lld", ticket);
        system(command);

        close(fd_o);
        close(fd_d);
      }
      else
      {
        sprintf(message, "Grading request %lld not found. Please check and resend your request ID or re-send your original grading request.", ticket);
        puts(message);
        len=strlen(message);
        write(newsockfd,&len,sizeof(int));
        write(newsockfd, message, len);
      }
      close(newsockfd);
    }
  }

  return 0;
}
