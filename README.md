# Client-Server Application README

## Overview

This client-server application aims to demonstrate different implementations of a server handling multiple client requests. Three versions of the server are provided: single-threaded, multi-threaded, and using a thread pool. Each version is designed to handle multiple client connections and requests.

## Versions

### Version 1: Single-Threaded

- **Description**: This version operates using a single thread to handle all incoming client requests sequentially.
- **Key Features**:
  - Simple, easy-to-understand design.
  - Handles one client request at a time.
- **Performance**:
  - Limited scalability when handling multiple simultaneous requests.

### Version 2: Multi-Threaded

- **Description**: Utilizes multiple threads to handle concurrent client requests.
- **Key Features**:
  - Improved concurrency by handling multiple client requests simultaneously.
  - Scales better than the single-threaded version.
- **Performance**:
  - Better responsiveness under high load compared to the single-threaded version.

### Version 3: Thread Pool

- **Description**: Implements a thread pool to manage a fixed set of threads handling incoming client requests.
- **Usage**: Compile and execute the thread pool server binary as detailed in the server's README.
- **Key Features**:
  - Efficiently manages a limited number of threads for request handling.
  - Balances resource utilization and scalability.
- **Performance**:
  - Optimal resource usage, improved scalability, and reduced overhead compared to the multi-threaded approach.

### Version 4: Asynchronus with threadpool

- **Description**: Implements a thread pool to manage a fixed set of threads handling incoming clients request asynchronusly.
- **Usage**: Compile and execute the thread pool server binary as detailed in the server's README.
- **Key Features**:
  - client run asynchronously.
  - Efficiently manages a limited number of threads for request handling.
  - Balances resource utilization and scalability.
- **Performance**:
  - Optimal resource usage, improved scalability, and reduced overhead compared to the multi-threaded approach.

## How to Run

#### Version 1
- first Execute the server
``` bash 
gcc gradingserver.c -o server
./server <portnumber>
```
- secnd execute the bash script
``` bash 
bash load.sh <ip> <port> <clients> <loopnumber> <sleeptime>
```
### Version 2
``` bash 
gcc simple-server.c -o server
./server <portnumber>
```
- secnd execute the bash script
``` bash 
bash load.sh <ip> <port> <clients> <loopnumber> <sleeptime> <timeouttime>
```
### Version 3
- first Execute the server 
``` bash 
make
./server <portnumber> <threadnumber>
```
- secnd execute the bash script
``` bash 
bash load.sh <ip> <port> <clients> <loopnumber> <sleeptime> <timeouttime>
```
### Version 4
- first Execute the server
``` bash 
make
./server <portnumber> <threadnumber>
```
- secnd execute the bash script
``` bash 
./submit <new/status> <ip:port> <filename> <ticket_id>
```
## Performance Comparison

- Each version's performance metrics, including response times, throughput, and CPU utilization, have been measured. and the graphs are in the perticular version.

## Notes

- Known issues, limitations, or specific considerations for each version are listed in their respective README files.
- Additional details or special instructions are included in individual version folders.

## Contibuters

-Utsav Manani (23M0788)
-Vinit Patel (23M0775)


