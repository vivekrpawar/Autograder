// Author: Vivek Pawar
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include "server-utils.h"
#define MAX_QUEUE_SIZE 1000
using namespace std;
int queue[MAX_QUEUE_SIZE];
int front = 0;
int rear = 0;
int request_count = 0;
pthread_mutex_t queue_mutex; 
pthread_cond_t req_ready;

//server
void *grader_thread(void *sockfd) {
    while(true) {
        // Lock queue before accessing 
        pthread_mutex_lock(&queue_mutex);
        while(request_count == 0)
            pthread_cond_wait(&req_ready, &queue_mutex);
        int socket_fd = dequeue(queue, &front, &rear); 
        request_count--;    // Decrease the cout of request
        pthread_mutex_unlock(&queue_mutex);
        // Unlock Queue
        grader(socket_fd); // grader fuction to grader the submission included server-utils.h
    }
    return (void *)NULL;
}

int main(int argc, char* argv[]) {
        // create socket for communication with client
        if(argc != 3) {
            cout << "Usage: " << argv[0] << " <PORT> <thread_pool_size>" << endl;
        }
        
        int thread_pool_size = atoi(argv[2]);
        int socket_fd; 
        struct sockaddr_in address;
        int address_length;
        pthread_t threads[thread_pool_size];
        pthread_mutex_init(&queue_mutex, NULL); 
        pthread_cond_init(&req_ready, NULL);
        for(int i = 0; i < thread_pool_size; i++){
            if(pthread_create(&threads[i], NULL, &grader_thread, NULL) != 0) {
                printf("ERROR: Could not create thread %d", i);
                exit(EXIT_FAILURE);
            } 
        }

        //Create Socket
	    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	        perror("Socket creation failed!");
            exit(EXIT_FAILURE);
        } 
        
        int PORT = atoi(argv[1]);
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons( PORT );
        address_length = sizeof(address); 
        
        // Bind Socket
        if (bind(socket_fd, (struct sockaddr *)&address, sizeof(address))<0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }
         
        // Listen for incoming connection
        if (listen(socket_fd, 1000) < 0) {
            perror("Listening to Socket failed");
            exit(EXIT_FAILURE);
        }
        
        cout << "Listening on port number: "<< PORT << endl;;
        while(true) {
            int client_socket_fd = accept(socket_fd, (struct sockaddr *)&address, (socklen_t*)&address_length);
            if (client_socket_fd <0) {
                perror("Error Accepting connection.");
                continue;
            }  
            
            pthread_mutex_lock(&queue_mutex);
            while((rear +1)%MAX_QUEUE_SIZE == front) {
                pthread_cond_wait(&req_ready, &queue_mutex);
            } 
            enqueue(client_socket_fd, queue, &front, &rear);
            request_count++;
            pthread_cond_signal(&req_ready);
            pthread_mutex_unlock(&queue_mutex);

        }
	return 0;
}
