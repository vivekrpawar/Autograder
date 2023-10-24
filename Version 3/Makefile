all: server submit

server: server.o unique-file-name.o grader.o enqueue.o dequeue.o
	g++ server.o unique-file-name.o grader.o enqueue.o dequeue.o -o server

server.o: server.cpp server-utils.h
	g++ -c server.cpp

submit: submit.o
	g++ submit.o -o submit

submit.o: submit.cpp
	g++ -c submit.cpp

clean:
	rm *.o submit server

