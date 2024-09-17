#include "includes.h"

int main(int argc, char* argv[]){
    check_executor_arguments(argc, argv);

    int buffer_size = atoi(argv[2]);
    int threadPoolSize = atoi(argv[3]);
    Server server = create_server(buffer_size, threadPoolSize);

    //create main thread
    pthread_t main_thread_id;
    int port = atoi(argv[1]);

    void** args = malloc(2*sizeof(void*));
    check_ptr(args, "malloc failed in main");
    args[1] = malloc(sizeof(int));
    check_ptr(args[1], "malloc failed in main");

    args[0] = server;
    *(int*) args[1] = port;

    int check = pthread_create(&main_thread_id, NULL, main_thread, args);
    check_thread(check, "pthread_create failed in main");

    //join main thread
    check = pthread_join(main_thread_id, NULL);
    check_thread(check, "pthread_join failed in main");

    // queue_destroy(server->queue, free_identifier_and_send_message);
    destroy_server(server, free_identifier_and_send_message);

    free(args[1]);
    free(args);
    return 0;
}