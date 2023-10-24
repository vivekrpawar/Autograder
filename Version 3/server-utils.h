#ifndef __GRADERUTIL_H
#define __GRADERUTIL_H
#include <string>
void grader(int client_socket_fd);
std::string generateUniqueFileName();
void enqueue(int item, int *queue, int *front, int *rear);
int dequeue(int *queue, int *front, int *rear);
#endif