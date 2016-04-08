OBJS = main.o functions.o encrypt.o decrypt.o
CC = g++ -std=c++11
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

default: server

server : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o server

main.o : main.cpp functions.h cipher.h
	$(CC) $(CFLAGS) main.cpp

fucntions.o : functions.cpp functions.h
	$(CC) $(CFLAGS) functions.cpp

encrypt.o : encrypt.cpp cipher.h
	$(CC) $(CFLAGS) encrypt.cpp

decrypt.o : decrypt.cpp cipher.h
	$(CC) $(CFLAGS) decrypt.cpp

clean :
	 \rm *.o *~ server


