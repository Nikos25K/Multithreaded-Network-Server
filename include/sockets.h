#ifndef SOCKETS_H
#define SOCKETS_H

// Creates a socket and returns the file descriptor
int create_socket(int port);

// Connects and returns the file descriptor
int connect_to_socket(struct sockaddr_in *server, char *hostname, char *port_str);

int read_int_from_socket(int fd);
char* read_string_from_socket(int fd);

void write_int_to_socket(int fd, int number);
void write_string_to_socket(int fd, char* buffer);

// Reads the arguments from the socket
char* read_args_from_socket(int fd);

// Writes jobID output start/end to the socket
void write_output_part_to_socket(int fd, char* jobID, bool is_start);

// Writes the file contents to the socket after the job is done
void write_file_contents_to_socket(int fd, char* filename, char* jobID);

// Reads size bytes from fd_in and writes them to the fd_out
void read_and_write(int fd_in, int fd_out, int size);

#endif