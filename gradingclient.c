/* run client using: ./client localhost <server_port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>
#include<time.h>
#include<pthread.h>
#include "common.h"

pthread_mutex_t con=PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char *argv[]) {
  
  int sockfd, portno, n;
  int wait_second, loop;
  int responses;
  struct sockaddr_in serv_addr; //Socket address structure
  struct hostent *server; //return type of gethostbyname
  int err=0;

  char buffer[256]; //buffer for message

  if (argc != 4) {
    fprintf(stderr, "usage : ./submit  <new|status> <serverIP:port> <sourceCodeFileTobeGraded|requestID>\n");
    exit(0);
  }
  
  responses =0;

  char ip_addr[20];
  char *ip = strtok(argv[2], ":");
  char *port_str = strtok(NULL, ":");
  portno = atoi(port_str);
  char *type=argv[1];
  /* fill in server address in sockaddr_in datastructure */
  server = gethostbyname(ip);
  //finds the IP address of a hostname. 
  //Address is returned in the 'h_addr' field of the hostend struct

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero

  serv_addr.sin_family = AF_INET; // Address Family is IP

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
/*Copy server IP address being held in h_addr field of server variable
to sin_addr.s_addr of serv_addr structure */

//convert host order port number to network order
  serv_addr.sin_port = htons(portno);

  //printf("Please enter the message: ");
  bzero(buffer, 256); //reset buffer to zero

  //array to store the start and end time to get the response
  struct timeval start_time[loop], end_time[loop];

  //time variables to claculate the time of while loop
  struct timeval  start,end;

  int k=0;
  
    /* create socket, get sockfd handle */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the half socket. 
    //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)  
    if (sockfd < 0)
      error("ERROR opening socket");
    
    /* connect to server 
    First argument is the half-socket, second is the server address structure
    which includes IP address and port number, third is size of 2nd argument
    */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
      error("ERROR connecting");
      ++err;
    }

    if(strcmp(type,"new")==0)
    {
      int t=0;
      int fd = open(argv[3],O_RDONLY);  
      //count the no of bytes in the file
      int bytes;
      bytes= filesize(argv[3]);
      printf("filesize : %d\n",bytes);

      write(sockfd,&t,sizeof(int));

      n=write(sockfd,&bytes,sizeof(bytes));

      lseek(fd, 0, SEEK_SET);
      //transfer the file
      while(1)
      {
        n = read(fd,buffer, sizeof(buffer));

        if(n==0)
          break;
        k = write(sockfd,buffer,n);
        if (k < 0)
        {
          error("error writing to socket");
          ++err;
          continue;
        }
        memset(buffer,0,sizeof(buffer));
      }
      long long  ticket;
      n=read(sockfd,&ticket,sizeof(long long));
      printf("Ticket : %lld",ticket);

      close(sockfd);
  return 0;
  }

else
{
  int t=1,len=-1;
  char message[1000];
  char *endptr;
  long long ticket = strtoll(argv[3], &endptr, 10);


  write(sockfd,&t,sizeof(int));

  write(sockfd,&ticket,sizeof(long long));

  read(sockfd,&len,sizeof(int));
  
  read(sockfd,message,len);
  
  printf("%s\n",message);

}
}
