#include "includes.h"

static pthread_t main_thread_id;
static pthread_t *worker_tids;

static int sockfd;

void handle_shutdown_signal(int sig) {
    shutdown(sockfd, SHUT_RDWR);
}

void* main_thread(void* args) {
    void** arguments = (void**) args;
    Server server = (Server) arguments[0];
    int port = *(int*) arguments[1];

    // Store the main thread ID
    main_thread_id = pthread_self();

    // Set up the signal handler
    struct sigaction sa;
    sa.sa_handler = handle_shutdown_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    create_worker_threads(server);

    // Wait for connections loop
    wait_for_connections(server, port);

    // Notify all Commanders in queue that server exits before executing their job
    // and free all elements using the provided function
    queue_delete_and_notify_all(server->queue, free_identifier_and_send_message);

    destroy_worker_threads(server);

    free(arguments[1]);
    free(args);

    pthread_exit(NULL);
}

void* worker_thread(void* args) {
    void** arguments = (void**) args;
    Server server = (Server) arguments[0];

    bool shutdown = false;
    while (1) {
        pthread_mutex_lock(&server->conc_mutex);
        while (server->active_workers >= server->concurrency)
            pthread_cond_wait(&server->can_run, &server->conc_mutex);

        server->active_workers++;
        pthread_mutex_unlock(&server->conc_mutex);

        Identifier id = queue_consume(server->queue);

        shutdown = (id == NULL);
        if (!shutdown){
            execute_job(id);
            free_identifier(id);
        }

        pthread_mutex_lock(&server->conc_mutex);
        server->active_workers--;
        pthread_cond_signal(&server->can_run);
        pthread_mutex_unlock(&server->conc_mutex);

        if (shutdown)
            break;
    }

    pthread_exit(NULL);
}

void* controller_thread(void* args) {
    void** arguments = (void**) args;
    Server server = (Server) arguments[0];
    int newsockfd = *(int*) arguments[1];

    char* buffer = read_args_from_socket(newsockfd);

    bool shutdown = parse_command(server, buffer, newsockfd);

    // Send signal to the main thread
    if (shutdown)
        pthread_kill(main_thread_id, SIGUSR1);

    free(buffer);

    free(arguments[1]);
    free(args);

    pthread_exit(NULL);
}

void create_worker_threads(Server server) {
    worker_tids = malloc(server->threadPoolSize * sizeof(pthread_t));
    check_ptr(worker_tids, "malloc failed in create_worker_threads");

    for (int i = 0; i < server->threadPoolSize; i++) {
        void** args = malloc(sizeof(void*));
        check_ptr(args, "malloc failed in create_worker_threads");
        args[0] = server;

        int check = pthread_create(&worker_tids[i], NULL, worker_thread, args);
        check_thread(check, "pthread_create failed in create_worker_threads");
    }
}

void destroy_worker_threads(Server server) {
    // Produce NULLs to signal the worker threads to exit
    // By producing NULLs, the worker threads exit their while loop
    for (int i = 0; i < server->threadPoolSize; i++)
        queue_produce(server->queue, NULL);

    for (int i = 0; i < server->threadPoolSize; i++) {
        int check = pthread_join(worker_tids[i], NULL);
        check_thread(check, "pthread_join failed in destroy_worker_threads");
    }
    free(worker_tids);
}

void wait_for_connections(Server server, int port) {
    sockfd = create_socket(port);

    int newsockfd;
    struct sockaddr_in client_addr;
    struct sockaddr *clientptr = (struct sockaddr *)&client_addr;
    socklen_t clilen = sizeof(client_addr);

    while ((newsockfd = accept(sockfd, clientptr, &clilen)) > 0) {

        pthread_t controller_thread_id;
        void** args = malloc(2 * sizeof(void*));
        check_ptr(args, "malloc failed in wait_for_connections");
        args[0] = server;
        args[1] = malloc(sizeof(int));
        check_ptr(args[1], "malloc failed in wait_for_connections");
        *(int*)args[1] = newsockfd;

        int check = pthread_create(&controller_thread_id, NULL, controller_thread, args);
        check_thread(check, "pthread_create failed in wait_for_connections");

        // Detach the controller thread
        pthread_detach(controller_thread_id);
    }
}