#ifndef THREADS_H
#define THREADS_H

// The thread functions used from server
void* main_thread(void* args);

void* worker_thread(void* args);

void* controller_thread(void* args);

// Create the workers and join
void create_worker_threads(Server server);
void destroy_worker_threads(Server server);

// Main loop for the server to accept connections
void wait_for_connections(Server server, int port);

#endif