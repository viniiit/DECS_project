CC = gcc

all: submit

# server : server.o thread_handler.o queue.o common.o
# 	$(CC) -o server server.o thread_handler.o queue.o common.o -lpthread

submit : submit.o common.o
	$(CC) -o submit submit.o common.o

# server.o: simple-server.c thread_handler.h common.h
# 	$(CC) -c simple-server.c -o server.o

# thread_handler.o: thread_handler.c thread_handler.h queue.h
# 	$(CC) -c thread_handler.c

# queue.o: queue.c queue.h
# 	$(CC) -c queue.c

submit.o: gradingclient.c common.h
	$(CC) -c gradingclient.c -o submit.o

common.o: common.c common.h
	$(CC) -c common.c

clean:
	rm -f submit submit.o common.o
