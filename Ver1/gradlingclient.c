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

void error(char *msg) {
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[]) {
  
  int sockfd, portno, n;
  int wait_second, loop;
  int responses;
  struct sockaddr_in serv_addr; //Socket address structure
  struct hostent *server; //return type of gethostbyname

  char buffer[256]; //buffer for message

  if (argc != 5) {
    fprintf(stderr, "usage %s <serverip:port> <SourceCodeFileToBeGraded> <loopNumber> <SleepTimeSeconds>\n", argv[0]);
    exit(0);
  }
  
  responses =0;



  char ip_addr[20];

  //String slicing 
  char *ip = strtok(argv[1], ":");
  char *port_str = strtok(NULL, ":");
  portno = atoi(port_str);
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

  /* connect to server 
  First argument is the half-socket, second is the server address structure
  which includes IP address and port number, third is size of 2nd argument
  */

  //If here means connection was complete

  /* ask user for input */

  //printf("Please enter the message: ");
  bzero(buffer, 256); //reset buffer to zero
  //fgets(buffer, 255, stdin); //read message from stdin, into buffer

  //getvalue from the command line interface
  wait_second=atoi(argv[4]);
  loop =atoi(argv[3]);

  //array to store the start and end time to get the response
  struct timeval start_time[loop], end_time[loop];

  //time variables to claculate the time of while loop
  struct timeval  start,end;

  //array to store diffrenet time diffrance size of the file
  double diff[loop];
  
  //open file 
  int fd = open(argv[2],O_RDONLY);

  int k;
  
  gettimeofday(&start, NULL); 
  //count the no of bytes in the file
  for (int x= 0 ;x<loop ; x++)
  {
    /* create socket, get sockfd handle */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the half socket. 
    //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)
    if (sockfd < 0)
      error("ERROR opening socket");

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
      error("ERROR connecting");
    
    int bytes;
    struct stat st;
    stat(argv[2], &st);
    bytes= st.st_size;
    
    //send size of the file
    n=write(sockfd,&bytes,sizeof(int));

    //transfer the file
    lseek(fd, 0, SEEK_SET);
    
    while(1)
    {
      n = read(fd,buffer, sizeof(buffer));

      if(n==0)
        break;

      k = write(sockfd, buffer,n);
      if (k < 0)
        error("error writing to socket");

      memset(buffer,0,sizeof(buffer));
    }
    gettimeofday(start_time+x, NULL); 

    //reading bytes from the server
    int o_bytes;
    read(sockfd, &o_bytes, sizeof(int));

    int k;
    k=0;
    printf(" %d ",x);
    while(1)
    {

      n = read(sockfd, buffer, 255);
      k += n;
      if (n < 0)
      error("ERROR reading from socket");
      //if the recieved bytes are the same than break the while loop
      printf("%s",buffer);
      if(k==o_bytes)
      {
        responses +=1;
        break;
      }
      memset(buffer,0,sizeof(buffer));

    }
    gettimeofday(end_time+x, NULL);
    double start_microseconds = start_time[x].tv_sec * 1000000LL + start_time[x].tv_usec;
    double end_microseconds = end_time[x].tv_sec * 1000000LL + end_time[x].tv_usec;
    diff[x] = (end_microseconds - start_microseconds)/1000000;
    //printf("%lld\n",diff[x]);
    memset(buffer,0,sizeof(buffer));

    close(sockfd);
    sleep(wait_second);
  }

  gettimeofday(&end, NULL); 
  double total_time;
  total_time = (end.tv_sec*1000000 + end.tv_usec)-(start.tv_sec*1000000 + start.tv_usec);
  total_time /= 1000000;

  double sum;
  sum =0 ;

  for(int i=0;i<loop;i++)
  {
    sum += diff[i];
    //printf("%\n",sum);
  }
  printf("Throughput is : %f\n",(((double)responses/((double)sum))));
  printf("Average time is : %f\n",(sum)/loop);
  printf("The No of Successfull response : %d\n",responses);
  printf("The time taken for completing the loop : %f\n",total_time);
  return 0;
}
