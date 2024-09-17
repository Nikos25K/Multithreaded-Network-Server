#include "includes.h"

// Creates a socket and returns the file descriptor
int create_socket(int port){
    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    check_number(sockfd, "socket failed in create_socket");

    // Set socket option to reuse address
    int opt = 1;
    int check = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    check_number(check, "setsockopt failed in create_socket");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    // Bind socket
    check = bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr));
    check_number(check, "bind failed in create_socket");

    // Listen for connections
    check = listen(sockfd, 5);
    check_number(check, "listen failed in create_socket");

    return sockfd;
}

// Connects and returns the file descriptor
int connect_to_socket(struct sockaddr_in *server, char *hostname, char *port_str) {
    struct sockaddr *serverptr = (struct sockaddr *) server;

    // Create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    check_number(fd, "socket failed in connect_to_socket");

    // Find server's address
    struct hostent *rem = gethostbyname(hostname);
    check_ptr(rem, "gethostbyname failed in connect_to_socket");

    server->sin_family = AF_INET;
    memcpy(&server->sin_addr, rem->h_addr_list[0], rem->h_length);
    int port = atoi(port_str);
    server->sin_port = htons(port);

    // Connect to server
    int con = connect(fd, serverptr, sizeof(*server));
    check_number(con, "connect failed in connect_to_socket");

    return fd;
}

// Reads an integer from the socket
int read_int_from_socket(int fd){
    // Pointer to the integer
    int number;
    char *ptr = (char *)&number;
    check_ptr(ptr, "read_int_from_pipe\n");

    int nread=0, tmp_nread;
    //read until the integer is read
    while(nread < sizeof(int)){
        tmp_nread = read(fd, ptr + nread, sizeof(int) - nread);
        if(tmp_nread > 0)
            nread += tmp_nread;
    }
    return number;
}

// Reads a string from the socket
char* read_string_from_socket(int fd){
    int size = read_int_from_socket(fd);
    char* buffer = malloc(size);
    check_ptr(buffer, "malloc failed in read_string_from_socket\n");

    int nread=0, tmp_nread;
    //read until the string is read
    while(nread < size){
        tmp_nread = read(fd, buffer + nread, size - nread);
        if(tmp_nread > 0)
            nread += tmp_nread;
    }
    return buffer;
}

// Writes an integer to the socket
void write_int_to_socket(int fd, int number){
    // Pointer to the integer
    char *ptr = (char *)&number;
    check_ptr(ptr, "write_int_to_pipe\n");

    int nwrite=0, tmp_nwrite;
    //write until the integer is written
    while(nwrite < sizeof(int)){
        tmp_nwrite = write(fd, ptr + nwrite, sizeof(int) - nwrite);
        if(tmp_nwrite > 0)
            nwrite += tmp_nwrite;
    }
}

// Writes a string to the socket
void write_string_to_socket(int fd, char* buffer){
    int size = strlen(buffer) + 1;
    write_int_to_socket(fd, size);

    int nwrite=0, tmp_nwrite;
    //write until the string is written
    while(nwrite < size){
        tmp_nwrite = write(fd, buffer + nwrite, size - nwrite);
        if(tmp_nwrite > 0)
            nwrite += tmp_nwrite;
    }
}

// Reads the arguments from the socket and returns them as a single string
char* read_args_from_socket(int fd){
    //read the number of arguments
    int argc = read_int_from_socket(fd);
    char** argv = malloc(argc * sizeof(char*));
    check_ptr(argv, "malloc failed in controller_thread");

    //read the arguments
    int length = 0;
    for(int i = 0; i < argc; i++){
        argv[i] = read_string_from_socket(fd);
        length += strlen(argv[i]) + 1;
    }

    //concatenate argv to a single string
    char* buffer = malloc(length);
    check_ptr(buffer, "malloc failed in controller_thread");
    buffer[0] = '\0';
    for(int i = 0; i < argc; i++){
        strcat(buffer, argv[i]);
        if (i < argc - 1)
            strcat(buffer, " ");
        strcat(buffer, "\0");
    }

    //free argv
    for(int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);

    return buffer;
}

// Writes the input/output parts of the job to the socket
void write_output_part_to_socket(int fd, char* jobID, bool is_start){
    int len;
    if(is_start)
        len = strlen("-----") + strlen(jobID) + 1 + strlen("  output start------\n") + 1;
    else
        len = strlen("-----") + strlen(jobID) + 1 + strlen(" output end------\n") + 1;

    char* buff = malloc(len);
    check_ptr(buff, "malloc failed in write_output_parts_to_socket");

    if(is_start)
        sprintf(buff, "-----%s output start------\n", jobID);
    else
        sprintf(buff, "-----%s output end------\n", jobID);

    write_string_to_socket(fd, buff);
    free(buff);
}

// Writes the contents of a file to the socket
void write_file_contents_to_socket(int fd, char* filename, char* jobID){
    // Open the file
    int file = open(filename, O_RDONLY);
    check_number(file, "open");

    //output start
    write_output_part_to_socket(fd, jobID, true);

    // Get the size of the file
    int size = lseek(file, 0, SEEK_END);
    write_int_to_socket(fd, size);

    if(size == 0){
        //output end
        write_output_part_to_socket(fd, jobID, false);
        close(file);
        return;
    }

    // Read the file
    lseek(file, 0, SEEK_SET);

    //write the file to the socket
    read_and_write(file, fd, size);

    //output end
    write_output_part_to_socket(fd, jobID, false);

    // Close the file
    close(file);
}

// Reads size bytes from fd_in and writes them to the fd_out
// Reads and writes in chunks of 1024 bytes
void read_and_write(int fd_in, int fd_out, int size){
    char str[1024];
    int nread=0, tmp_nread;
    int nwrite=0, tmp_nwrite;
    //read until size bytes are read
    while(size > 0){
        int to_read = size > 1024 ? 1024 : size;
        tmp_nread = read(fd_in, str, to_read);
        if(tmp_nread < 0)
            print_error("read failed in read_and_write");

        nread = tmp_nread;
        size -= nread;
        nwrite = 0;
        //write until number of read bytes are written
        while(nwrite < nread){
            tmp_nwrite = write(fd_out, str + nwrite, nread - nwrite);
            if(tmp_nwrite < 0)
                print_error("write failed in read_and_write");

            nwrite += tmp_nwrite;
        }
    }
}