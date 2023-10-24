// Source: Lab 4 solutions
#include <cstdio>
#define QUEUE_SIZE 1000
int dequeue(int *queue, int *front, int *rear)
{
    int item;
    if (*front == *rear)
    {
        printf("\nThe Queue is empty\n");
    }
    else
    {
        *front = (*front + 1) % QUEUE_SIZE;
        item = queue[*front];
    }
    return item;
}