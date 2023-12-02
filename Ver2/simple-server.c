#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>

pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t diff_file_lock=PTHREAD_MUTEX_INITIALIZER;
int i =0;

void error(char *msg) {
  perror(msg);
}

void *start_function(void *sockfd) 
{
  int newsockfd = *(int *)sockfd;
  char buffer[256];
  char command[100];
  int n;
  bzero(buffer, 256);

  //make file for the user
  pthread_mutex_lock(&lock);
  int j=i;
  i=i+1;
  pthread_mutex_unlock(&lock);
  
  char cppfile[20];
  sprintf(cppfile,"demo%d.cpp",j);

  //get the size of the file
  int no_of_bytes;
  read(newsockfd, &no_of_bytes, sizeof(int));
  
  //open file to store the data 
  int fd;
  fd = open(cppfile,O_CREAT|O_RDWR|O_TRUNC);

  int file_bytes;
  file_bytes=0;
  
  //file reading operation
  do
  {
    n = read(newsockfd, buffer, sizeof(buffer));
    file_bytes=file_bytes+n;
    if (n < 0) 
    {
        error("Error reading from socket");
        pthread_exit(NULL);
    }

    //write into the file
    int bytes_written = write(fd, buffer, n);
    if (bytes_written < 0) 
    {
        error("Error writing to file");
        pthread_exit(NULL);
    }
    
    //if file size is reached
    if (file_bytes==no_of_bytes)
    {
        break;
    }

    memset(buffer, 0, sizeof(buffer));
  }while(1);

  //open output file to store the output
  char outputfile[20];
  sprintf(outputfile,"output%d.txt",j);
  printf("client : %d\n",j);
  int fd_o;
  truncate(outputfile, 0);
  fd_o= open(outputfile,O_CREAT|O_RDWR|O_TRUNC);
    
  //compile file if not success then the transfer the error into the output.txt
  memset(command,0,sizeof(command));
  sprintf(command,"g++ -w -o demo%d %s 2>%s",j,cppfile,outputfile);
  int compile_success=system(command); 

  if(compile_success == 0)
  {
    //to clean buffer
    memset(command,0,sizeof(command));
    sprintf(command,"./demo%d 2>%s >/dev/null",j,outputfile);

    //to check weather the run is successfully or not if not theen pass to the output.txt
    int run_success=system(command);
    if(run_success == 0)
    {
      char *act_out="1 2 3 4 5 6 7 8 9 10 ";
      char prog_out[512];

      //transfer the output in to the output.txt
      memset(command,0,sizeof(command));
      sprintf(command,"./demo%d >%s",j,outputfile);
      system(command);

      //read output from the output file
      n=read(fd_o, prog_out,512);
      
      // compare the two string
      if(strcmp(prog_out,act_out) == 0)
      {
        char message[]="pass\n";

        truncate(outputfile, 0);
        lseek(fd_o, 0, SEEK_SET);

        int n2=write(fd_o,message,sizeof(message));
        if(n2 < 0)
        {
          error("problem while writing the message into the output.txt");
          pthread_exit(NULL);
        }
          
      }
      else
      {
        memset(buffer,0,sizeof(buffer));
        lseek(fd_o, 0, SEEK_SET);

        //compare two output with diff lock on file
        pthread_mutex_lock(&diff_file_lock);

        int fd_d= open("diff.txt",O_CREAT|O_RDWR);
        memset(command,0,sizeof(command));

        sprintf(command,"diff actual_output.txt %s > diff.txt",outputfile);
        int diff= system(command);

        int f1 = read(fd_d,buffer,sizeof(buffer)); 
        if(f1<0)
        {
          error("Error while reading the diff file");
          pthread_exit(NULL);
        }

        close(fd_d);

        pthread_mutex_unlock(&diff_file_lock);

        int f2 = write(fd_o,buffer,f1);
        if(f2<0)
        {
          error("While writing the file");
          pthread_exit(NULL);
        }
      }
    }
  }

  int bytes;
  lseek(fd_o, 0, SEEK_SET);

  //findout the size of the output file
  int o_bytes;
  struct stat st;
  stat(outputfile, &st);
  o_bytes= st.st_size;
  write(newsockfd,&o_bytes, sizeof(int));
  lseek(fd_o, 0, SEEK_SET);

  while (1) 
  {
    ssize_t n2 = read(fd_o, buffer, sizeof(buffer));

    if (n2 < 0) 
    {
        error("Error while reading data from the file output.txt");
        pthread_exit(NULL);
    } 
    else if (n2 == 0) 
    {
        // End of file reached, break the loop
        break;
    }
      // Write the data read from the file to the network socket
    ssize_t n1 = write(newsockfd, buffer, n2);
    if (n1 < 0) 
    {
      error("Error while writing to the socket");
      pthread_exit(NULL);
    }
    memset(buffer,0,sizeof(buffer));
  }
  close(fd_o);
  close(fd);
  close(newsockfd);
  remove(outputfile);
  remove(cppfile);
  sprintf(command,"rm demo%d",j);
  system(command);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
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

  /* accept a new request, create a newsockfd */

  while (1) {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");
    pthread_t thread;
    if (pthread_create(&thread, NULL, start_function, &newsockfd) != 0)
      printf("Failed to create Thread\n");
  }

  return 0;
}
