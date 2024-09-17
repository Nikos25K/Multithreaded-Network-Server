#include "includes.h"

struct queue {
    void** data;

    int count;
    int capacity;

    int start;
    int end;

    pthread_mutex_t mtx;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;
};

Queue queue_create(int capacity) {
    check_number(capacity, "queue_create received invalid capacity");

    Queue ptr = (Queue) malloc(sizeof (struct queue));
    check_ptr(ptr, "malloc failed in queue_create");

    ptr->count = 0;
    ptr->capacity = capacity;
    ptr->start = 0;
    ptr->end = 0;

    // Initialize mutex and condition variables
    if (pthread_mutex_init(&ptr->mtx, NULL) != 0) {
        free(ptr);
        print_error("pthread_cond_init failed in queue_create\n");
    }
    if (pthread_cond_init(&ptr->cond_nonempty, NULL) != 0) {
        free(ptr);
        pthread_mutex_destroy(&ptr->mtx);
        print_error("pthread_cond_init failed in queue_create\n");
    }
    if (pthread_cond_init(&ptr->cond_nonfull, NULL) != 0) {
        pthread_cond_destroy(&ptr->cond_nonempty);
        pthread_mutex_destroy(&ptr->mtx);
        free(ptr);
        print_error("pthread_cond_init failed in queue_create\n");
    }

    // Allocate memory for the data array
    ptr->data = (void**) malloc(sizeof(void*) * capacity);
    check_ptr(ptr->data, "malloc failed in queue_create");
    for (int i = 0; i < capacity; i++)
        ptr->data[i] = NULL;

    return ptr;
}

int queue_size(Queue q) {
    check_ptr(q, "queue_size received NULL pointer");
    return q->count;
}

// Add an element to the queue if it is not full and signal the consumer
void queue_produce(Queue q, void* data) {
    check_ptr(q, "queue_produce received NULL pointer");

    pthread_mutex_lock(&q->mtx);

    // Wait until the queue is not full
    while (q->count == q->capacity)
        pthread_cond_wait(&q->cond_nonfull, &q->mtx);

    q->data[q->end] = data;
    q->end = (q->end + 1) %  q->capacity;
    q->count++;

    // Signal the consumer that the queue is not empty
    pthread_cond_signal(&q->cond_nonempty);

    pthread_mutex_unlock(&q->mtx);
}

// Remove an element from the queue using the provided function
void* queue_remove_by(Queue q, CompareFunc compare_func, void* arg) {
    check_ptr(q, "queue_remove_by received NULL pointer");
    check_ptr(compare_func, "queue_remove_by received NULL pointer");
    check_ptr(arg, "queue_remove_by received NULL pointer");

    pthread_mutex_lock(&q->mtx);

    // Empty queue
    if (q->count == 0) {
        pthread_mutex_unlock(&q->mtx);
        return NULL;
    }

    void* data = NULL;
    int found_index = -1;
    // Find the element to remove and its index using the provided function
    for (int i = 0; i < q->capacity; i++) {
        if (q->data[i] != NULL && compare_func(q->data[i], arg)) {
            data = q->data[i];
            found_index = i;
            break;
        }
    }

    if (found_index != -1) {
        // Shift elements to fill the removed element's place
        for (int i = found_index; i < q->capacity - 1; i++)
            q->data[i] = q->data[i + 1];

        q->data[q->capacity - 1] = NULL;
        q->count--;
    }

    pthread_mutex_unlock(&q->mtx);

    // Signal the producer that the queue is not full
    pthread_cond_signal(&q->cond_nonfull);

    return data;
}

// Remove an element from the queue and signal the producer
void* queue_consume(Queue q) {
    check_ptr(q, "queue_consume received NULL pointer");

    pthread_mutex_lock(&q->mtx);

    // Wait until the queue is not empty
    while (q->count == 0)
        pthread_cond_wait(&q->cond_nonempty, &q->mtx);

    void* data = q->data[q->start];

    q->data[q->start] = NULL;
    q->start = (q->start + 1) % q->capacity;
    q->count--;

    pthread_mutex_unlock(&q->mtx);

    // Signal the producer that the queue is not full
    pthread_cond_signal(&q->cond_nonfull);

    return data;
}

// Send all elements in the queue using the provided function
void queue_send_all(Queue q, WriteFunc write_func, int clientSocket) {
    check_ptr(q, "queue_send_all received NULL pointer");
    check_ptr(write_func, "queue_send_all received NULL pointer");
    check_number(clientSocket, "queue_send_all received invalid clientSocket");

    pthread_mutex_lock(&q->mtx);

    // Send the number of elements in the queue
    write_int_to_socket(clientSocket, q->count);

    // Send all elements in the queue
    for (int i = 0; i < q->capacity; i++)
        if (q->data[i] != NULL)
            write_func(q->data[i], clientSocket);

    pthread_mutex_unlock(&q->mtx);
}

// Destroy the queue and free all elements using the provided function
void queue_destroy(Queue q){
    check_ptr(q, "queue_destroy received NULL pointer");

    // Destroy the mutex and condition variables
    pthread_cond_destroy(&q->cond_nonempty);
    pthread_cond_destroy(&q->cond_nonfull);
    pthread_mutex_destroy(&q->mtx);

    free(q->data);
    free(q);
}

void queue_delete_and_notify_all(Queue q, FreePtr free_func) {
    check_ptr(q, "queue_notify_all received NULL pointer");
    check_ptr(free_func, "queue_notify_all received NULL pointer");

    pthread_mutex_lock(&q->mtx);

    for (int i = 0; i < q->capacity; i++){
        if (q->data[i] != NULL){
            free_func(q->data[i]);
            q->data[i] = NULL;
        }
    }

    pthread_mutex_unlock(&q->mtx);
}