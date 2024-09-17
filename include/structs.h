#ifndef STRUCTS_H
#define STRUCTS_H

// The server struct
struct server{
    Queue queue;
    int threadPoolSize;
    int concurrency;
    int count_ids;
    int active_workers;

    pthread_mutex_t conc_mutex;
    pthread_cond_t can_run;
};
typedef struct server *Server;

// Create a new server with the given number of worker threads and the given concurrency
Server create_server(int worker_threads, int concurrency);

// Free the server and the queue using the provided function
void destroy_server(Server server, FreePtr free_func);


// Struct for the identifier of a job stored in the queue
struct identifier {
    char* jobID;
    char* job;
    int clientSocket;
};
typedef struct identifier *Identifier;

// Create an identifier for a job
Identifier create_identifier(int count_ids, char* job, int clientSocket);

// Compare function for identifiers
bool compare_identifiers(void* id1, void* id2);

// Write function for identifiers
void write_identifier(void* data, int fd_write);

// Free functions for identifiers
void free_identifier(void* data);
void free_identifier_and_send_message(void* data);


#endif