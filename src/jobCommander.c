#include "includes.h"

int main(int argc, char* argv[]){
    check_commander_arguments(argc, argv);

    // Create socket and connect to server
    struct sockaddr_in server;
    int fd = connect_to_socket(&server, argv[1], argv[2]);

    // Send command to server
    //first write how many arguments commander will send
    write_int_to_socket(fd, argc-3);

    for(int i = 3; argv[i] != NULL; i++)
        write_string_to_socket(fd, argv[i]);

    // Read response from server
    handle_response(argv[3], fd);

    close(fd);

    return 0;
}