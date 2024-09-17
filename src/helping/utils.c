#include "includes.h"

//prints an error message to stderr and exits the program
void print_error(char* message){
    fprintf(stderr,"%s",message);
    exit(1);
}

//checks if the pointer is NULL and exits the program if it is
//used for malloc and other functions that return a pointer
void check_ptr(void* ptr, char* message){
    if(!ptr)
        print_error(message);
}
//checks if the number is negative and exits the program if it is
//used for reading, writing, opening files etc
void check_number(int num, char* message){
    if(num < 0){
        perror(message);
        exit(1);
    }
}

//checks if there is an error in a thread function and exits the program if there is
void check_thread(int error, char* message){
    if(error){
        fprintf(stderr,"%s: %s\n", message, strerror(error));
        exit(1);
    }
}

//checks if the string is a number and exits the program if it is not
void check_str_to_num(char* str, char* message){
    check_ptr(str, "Error in check_str_to_num: str is NULL\n");
    check_ptr(message, "Error in check_str_to_num: message is NULL\n");

    //if the number is negative
    if(atoi(str) < 0)
        print_error(message);

    //if the number is not a number
    for(int i = 0; i < strlen(str); i++)
        if(!isdigit(str[i]))
            print_error(message);
}

//a function to count the digits of a number
//used to calculate the size chars to allocate for the string
int count_digits(int number){
    int count = 0;

    // Count digits
    while(number != 0){
        number /= 10;
        count++;
    }

    return count;
}

//if the user enters an invalid instruction, print the valid ones
void print_valid_instructions(){
    printf("Valid instructions:\n");
    printf("issueJob [jobName]\n");
    printf("setConcurrency [N]\n");
    printf("stop [jobID]\n");
    printf("poll\n");
    printf("exit\n");
}

//checks if the commander arguments are correct and exits the program if they are not
void check_commander_arguments(int argc, char* argv[]){
    //if the arguments are not correct
    if(argc < 4)
        print_error("Usage: ./jobCommmander [serverName] [portNum] [jobCommanderInputCommand]\n");

    //if the port number is not a number
    check_str_to_num(argv[2], "Port number must be a positive number\n");

    if(strcmp(argv[3],"issueJob") == 0){
        //if the arguments are not correct
        if(argc < 5)
            print_error("Usage: ./jobCommmander [serverName] [portNum] issueJob [jobName]\n");
    }

    else if(strcmp(argv[3], "setConcurrency") == 0){
        //if the arguments are not correct
        if(argc != 5)
            print_error("Usage: ./jobCommmander [serverName] [portNum] setConcurrency [N]\n");

        check_str_to_num(argv[4], "Concurrency must be a positive number\n");
    }

    else if(strcmp(argv[3], "stop") == 0){
        bool flag = false;

        //if the arguments are not correct
        if (argc != 5)
            flag = true;

        //if the argument is not a job
        if (strcmp(argv[4], "job_") == 0 || strlen(argv[4]) < 5)
            flag = true;

        //if the job number is not a number
        check_str_to_num(argv[4] + 4, "Job number must be a positive number\n");

        if (flag)
            print_error("Usage: ./jobCommander [serverName] [portNum] stop [job_XX]\n");
    }

    else if(strcmp(argv[3], "poll") == 0){
        if(argc != 4)
            print_error("Usage: ./jobCommmander [serverName] [portNum] poll\n");
    }

    else if(strcmp(argv[3], "exit") == 0){
        if (argc != 4)
            print_error("Usage: ./jobCommmander [serverName] [portNum] exit\n");
    }

    else{
        fprintf(stderr,"Invalid instruction\n \n");
        print_valid_instructions();
        exit(1);
    }
}

void check_executor_arguments(int argc, char* argv[]){
    //if the arguments are not correct
    if(argc != 4)
        print_error("Usage: jobExecutorServer [portnum] [bufferSize] [threadPoolSize]\n");

    //if the port number is not a number
    check_str_to_num(argv[1], "Port number must be a positive number\n");

    //if the buffer size is not a number
    check_str_to_num(argv[2], "Buffer size must be a positive number\n");

    //if the thread pool size is not a number
    check_str_to_num(argv[3], "Thread pool size must be a positive number\n");
}

void handle_response(char* arg, int fd){
    if(strcmp(arg, "poll") == 0){
        // Read number of jobs
        int length = read_int_from_socket(fd);
        if(length == 0){
            printf("Queue is empty\n");
            return;
        }

        for(int i = 0; i < length; i++){
            char* jobID = read_string_from_socket(fd);
            char* job = read_string_from_socket(fd);
            printf("%s %s\n", jobID, job);
            free(jobID);
            free(job);
        }
        return;
    }

    // Mutex for printing to stdout if the Commanders are running in parallel 
    // and writing to the same file descriptor
    pthread_mutex_t print_mtx = PTHREAD_MUTEX_INITIALIZER;

    char* response = read_string_from_socket(fd);
    pthread_mutex_lock(&print_mtx);
    printf("%s\n", response);
    pthread_mutex_unlock(&print_mtx);
    free(response);

    // Read also the output of the instruction
    if(strcmp(arg, "issueJob") == 0){
        char* first = read_string_from_socket(fd);
        pthread_mutex_lock(&print_mtx);
        printf("\n%s", first);
        fflush(stdout);
        pthread_mutex_unlock(&print_mtx);
        free(first);

        // Read the size of the file
        int size = read_int_from_socket(fd);
        pthread_mutex_lock(&print_mtx);
        if(size == 0)
            printf("\tNo output\n");

        // Read the file and print it
        else
            read_and_write(fd, STDOUT_FILENO, size);
        pthread_mutex_unlock(&print_mtx);

        char* last = read_string_from_socket(fd);
        pthread_mutex_lock(&print_mtx);
        printf("%s\n", last);
        fflush(stdout);
        pthread_mutex_unlock(&print_mtx);
        free(last);
    }
    pthread_mutex_destroy(&print_mtx);
}