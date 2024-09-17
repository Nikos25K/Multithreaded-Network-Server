#include "includes.h"

Server create_server(int buffer_size, int threadPoolSize){
    check_number(buffer_size, "create_server received invalid buffer size");
    check_number(threadPoolSize, "create_server received invalid thread pool size");

    Server server = malloc(sizeof(struct server));
    check_ptr(server, "malloc failed in create_server");

    server->threadPoolSize = threadPoolSize;
    // No leading zeros
    server->count_ids = 1;

    server->concurrency = 1;
    server->active_workers = 0;

    // Initialize the mutex and condition variable
    if (pthread_mutex_init(&server->conc_mutex, NULL) != 0) {
        free(server);
        print_error("pthread_mutex_init failed in create_server\n");
    }
    if (pthread_cond_init(&server->can_run, NULL) != 0) {
        pthread_mutex_destroy(&server->conc_mutex);
        free(server);
        print_error("pthread_cond_init failed in create_server\n");
    }

    // Create the queue
    server->queue = queue_create(buffer_size);

    return server;
}

void destroy_server(Server server, FreePtr free_func){
    check_ptr(server, "destroy_server received NULL pointer");
    check_ptr(free_func, "destroy_server received NULL pointer");

    pthread_mutex_destroy(&server->conc_mutex);
    pthread_cond_destroy(&server->can_run);

    // Destroy the queue using the provided function
    queue_destroy(server->queue);
    free(server);
}

// Create an identifier for a job and return it
Identifier create_identifier(int count_ids, char* job, int clientSocket){
    check_number(count_ids, "create_identifier: count_ids\n");
    check_ptr(job, "create_identifier: job is NULL\n");
    check_number(clientSocket, "create_identifier: clientSocket\n");

    Identifier id = malloc(sizeof(struct identifier));
    check_ptr(id, "create_identifier: malloc\n");

    //update the id
    int len = count_digits(count_ids);
    id->jobID = malloc(len + strlen("job_")+1);
    check_ptr(id->jobID, "update_identifier: malloc\n");
    sprintf(id->jobID, "job_%d", count_ids);

    //update the job
    id->job = malloc(strlen(job)+1);
    check_ptr(id->job, "update_identifier: malloc\n");
    strcpy(id->job, job);

    id->clientSocket = clientSocket;

    return id;
}

//will be used in queue_remove_by, id1 is an identifier, id2 is a job
bool compare_identifiers(void* id1, void* id2){
    check_ptr(id1, "compare_identifiers: id1 is NULL\n");
    check_ptr(id2, "compare_identifiers: id2 is NULL\n");

    Identifier i1 = (Identifier) id1;
    char* job2 = (char*) id2;

    check_ptr(i1, "compare_identifiers: i1 is NULL\n");
    check_ptr(i1->jobID, "compare_identifiers: i1->jobID is NULL\n");
    check_ptr(id2, "compare_identifiers: id2 is NULL\n");

    return strcmp(i1->jobID, job2) == 0;
}

//write the identifier to the socket
void write_identifier(void* data, int fd_write){
    check_ptr(data, "write_identifier: data is NULL\n");
    check_number(fd_write, "write_identifier: fd_write\n");

    Identifier id = (Identifier) data;
    check_ptr(id, "write_identifier: id is NULL\n");
    check_ptr(id->jobID, "write_identifier: id->jobID is NULL\n");
    check_ptr(id->job, "write_identifier: id->job is NULL\n");

    write_string_to_socket(fd_write, id->jobID);
    write_string_to_socket(fd_write, id->job);
}

//free the identifier
void free_identifier(void* data){
    check_ptr(data, "free_identifier: data is NULL\n");

    Identifier id = (Identifier) data;
    check_ptr(id, "free_identifier: id is NULL\n");
    check_ptr(id->jobID, "free_identifier: id->jobID is NULL\n");
    check_ptr(id->job, "free_identifier: id->job is NULL\n");

    close(id->clientSocket);

    free(id->jobID);
    free(id->job);
    free(id);
}

void free_identifier_and_send_message(void* data){
    check_ptr(data, "free_identifier_and_send_message: data is NULL\n");

    Identifier id = (Identifier) data;
    check_ptr(id, "free_identifier_and_send_message: id is NULL\n");
    check_ptr(id->jobID, "free_identifier_and_send_message: id->jobID is NULL\n");
    check_ptr(id->job, "free_identifier_and_send_message: id->job is NULL\n");
    check_number(id->clientSocket, "free_identifier_and_send_message: id->clientSocket\n");

    write_output_part_to_socket(id->clientSocket, id->jobID, true);
    write_string_to_socket(id->clientSocket, "SERVER TERMINATED BEFORE EXECUTION\n");
    write_output_part_to_socket(id->clientSocket, id->jobID, false);

    free(id->jobID);
    free(id->job);
    free(id);
}