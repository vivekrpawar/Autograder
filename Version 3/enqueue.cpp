// Source: Lab 4 solutions
#include <cstdio>
#include <iostream>
using namespace std;
#define QUEUE_SIZE 1000
void enqueue(int item, int *queue, int *front, int *rear)
{    
    *rear = (*rear + 1) % QUEUE_SIZE;
    if (*front == *rear)
    {
        printf("\nOverflow\n");
        if (*rear == 0)
            *rear = QUEUE_SIZE - 1;
        else
            *rear--;
    }
    else
    {
        queue[*rear] = item;
    } 
}
