/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>

int i=0;
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[]) 
{
  int sockfd, //the listen socket descriptor (half-socket)
   newsockfd, //the full socket after the client connection is made
   portno; //port number at which server listens

  socklen_t clilen; //a type of an integer for holding length of the socket address
  char buffer[256]; //buffer for reading and writing the messages
  struct sockaddr_in serv_addr, cli_addr; //structure for holding IP addresses
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  //AF_INET means Address Family of INTERNET. SOCK_STREAM creates TCP socket (as opposed to UDP socket)
 // This is just a holder right now, note no port number either. It needs a 'bind' call


  if (sockfd < 0)
    error("ERROR opening socket");

 
  bzero((char *)&serv_addr, sizeof(serv_addr)); // initialize serv_address bytes to all zeros
  
  serv_addr.sin_family = AF_INET; // Address Family of INTERNET
  serv_addr.sin_addr.s_addr = INADDR_ANY;  //Any IP address. 

//Port number is the first argument of the server command
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);  // Need to convert number from host order to network order

  /* bind the socket created earlier to this port number on this machine 
 First argument is the socket descriptor, second is the address structure (including port number).
 Third argument is size of the second argument */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */

  listen(sockfd, 100); // 2 means 2 connection requests can be in queue. 
  //now server is listening for connections


  clilen = sizeof(cli_addr);  //length of struct sockaddr_in

  while(1)
  {
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      error("ERROR on accept");

    /* read message from client */

    bzero(buffer, 256); //set buffer to zero

    //open file to store the data 
    truncate("server_demo.cpp", 0);
    int fd;
    fd = open("server_demo.cpp",O_CREAT|O_RDWR);

    //get the size of the file
    int no_of_bytes;
    read(newsockfd, &no_of_bytes, sizeof(int));
    
    truncate("server_demo.cpp", 0);
    do
    {
      //no of bytes server reading
      int file_bytes;
      file_bytes=0;

      // Read data from the client socket into the buffer
      n = read(newsockfd, buffer, sizeof(buffer));
      //printf("%d\n",n);
      file_bytes=file_bytes+n;
      if (n < 0) {
          // Handle read error
          error("Error reading from socket");
          break;
      }
      // Transfer data into the server_demo.cpp file
      int bytes_written = write(fd, buffer, n);
      if (bytes_written < 0) {
          // Handle write error
          error("Error writing to file");
      }
      if (file_bytes==no_of_bytes) {
          // End of file reached, no more data from the client
          break;
      }

      // Reset the buffer
      memset(buffer, 0, sizeof(buffer));
    } while(1);
    //open output file to store the output
    int fd_o;
    truncate("output.txt", 0);
    fd_o= open("output.txt",O_CREAT|O_RDWR|O_TRUNC);
    
    //compile file if not success then the transfer the error into the output.txt
    int compile_success=system("g++ -w -o server_demo server_demo.cpp 2> output.txt");

    if(compile_success == 0)
    {
      //to check weather the run is successfully or not if not theen pass to the output.txt
      int run_success=system("./server_demo 2> output.txt 1> output.txt");
      if(run_success == 0)
      {
        char *act_out="1 2 3 4 5 6 7 8 9 10 ";
        char prog_out[512];

        //read output from the output file
        n=read(fd_o, prog_out,512);

        //printf("%d\n",strcmp(prog_out,act_out) );
        // compare the two string
        if(strcmp(prog_out,act_out) == 0)
        {
          // message pass to the client
          char message[]="pass\n";

          truncate("output.txt", 0);

          lseek(fd_o, 0, SEEK_SET);

          //store into the output.txt
          int n2=write(fd_o,message,sizeof(message));

          if(n2 < 0)
          {
            error("problem while writing the message into the output.txt");
          }
          
        }
        else
        {
          //compare two output

          int fd_d= open("diff.txt",O_CREAT|O_RDWR);
          int diff = system("diff actual_output.txt output.txt > diff.txt");
          
          //buffer set to zero

          memset(buffer,0,sizeof(buffer));

          //set the file descriptopr to first
          lseek(fd_o, 0, SEEK_SET);
          int f1 = read(fd_d,buffer,sizeof(buffer));
          
          if(f1<0)
          {
            error("Error while reading the diffrance file");
          }
          int f2 = write(fd_o,buffer,f1);

          if(f2<0)
          {
            error("While riting the file");
          }

        }
      }
    }

    //setting the fd_o to start of the file
    int bytes;
    lseek(fd_o, 0, SEEK_SET);

    //findout the size of the
    int o_bytes;
    struct stat st;
    char *file = "output.txt";
    stat(file, &st);
    o_bytes= st.st_size;
    write(newsockfd,&o_bytes, sizeof(int));
    lseek(fd_o, 0, SEEK_SET);

    while (1) 
    {
      ssize_t n2 = read(fd_o, buffer, sizeof(buffer));
      printf("%s\n",buffer);
      //printf("\n%ld\n",n2);
      if (n2 < 0) {
          error("Error while reading data from the file output.txt");
      } else if (n2 == 0) {
          // End of file reached, break the loop
          break;
      }
      // Write the data read from the file to the network socket
      ssize_t n1 = write(newsockfd, buffer, n2);
      if (n1 < 0) {
          error("Error while writing to the socket");
      }
      memset(buffer,0,sizeof(buffer));
    }

    close(fd_o);
    close(fd);
    close(newsockfd);
  }
  close(sockfd);

  return 0;
}
