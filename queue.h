#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// node -> it is to be pushed inside the queue

typedef struct node{
    int* Client;
    struct node* prev;
    struct node* next; 
}Node;


// structure of queue 
typedef struct queue{
   
        Node* front;
        Node* rear;
        int capacity;
        int curr_capacity;
}Queue;




// creates a node for a given client
Node* createNode(int* client){
    Node* tmp = (Node*)malloc(sizeof(Node));
    tmp->Client=client;
    tmp->prev = tmp->next = NULL;
    return tmp;
}


// creates the queue with the given capacity 
Queue* createQueue(int capacity){
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    q->capacity = capacity;
    q->curr_capacity = 0;
    return q;
}


// push the element at the back of the queue

// int push(Queue *q , int *client)
// {
//     if(q->curr_capacity==q->capacity)
//     return -1;
//     q->curr_capacity+=1;

// }
int enqueue(Queue* q, int* client){
    if(q->curr_capacity==q->capacity)
        return -1;
    q->curr_capacity += 1;
    // Node* temp  = createNode(client);
    Node* tmp = createNode(client);


    if(q->front==q->rear){
        q->front = tmp;
          // Node* temp  = createNode(client);
        q->rear=tmp;
    }
    else{
          // Node* temp  = createNode(client);
        q->rear->next = tmp;
        tmp->prev = q->rear;
        q->rear = tmp;
    }
    return 1;
}

// int * pop(Queue *q)
// {
//     if(q->curr_capacity==0)
//     return NULL;
//     q->curr_capacity = q->curr_capacity -1;
//     Node* temp  = q->front ; 
// }

int* dequeue(Queue* q){
    if(q->curr_capacity==0)
        return NULL;
    
    q->curr_capacity -= 1;
    Node* temp=q->front;

    if(q->front==q->rear)
        q->front = q->rear = NULL;
    else{
        q->front = q->front->next;
        q->front->prev = NULL;
    }
    temp->next = NULL;
    int* client = temp->Client;
    free(temp);
    return client;
} 



// int main()
// {
//     Queue* q;
//     q = createQueue(50);

//     printf("%d",q->capacity);


//     return 0 ; 
// }