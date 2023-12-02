CC = gcc

all: server submit

server : server.o thread_handler.o queue.o common.o hashmap.o fault_tolerance.o
	$(CC) -o server server.o thread_handler.o queue.o common.o hashmap.o fault_tolerance.o -lpthread

submit : submit.o common.o
	$(CC) -o submit submit.o common.o

server.o: simple-server.c thread_handler.h common.h hashmap.h
	$(CC) -c simple-server.c -o server.o

thread_handler.o: thread_handler.c hashmap.h thread_handler.h queue.h
	$(CC) -c thread_handler.c

queue.o: queue.c queue.h
	$(CC) -c queue.c

submit.o: gradingclient.c common.h
	$(CC) -c gradingclient.c -o submit.o

common.o: common.c common.h
	$(CC) -c common.c

hashmap.o: hashmap.c hashmap.h
	$(CC) -c hashmap.c

fault_tolerance.o: fault_tolerance.c fault_tolerance.h hashmap.h queue.h
	$(CC) -c fault_tolerance.c

clean:
	rm -f server submit server.o thread_handler.o queue.o submit.o hashmap.o
