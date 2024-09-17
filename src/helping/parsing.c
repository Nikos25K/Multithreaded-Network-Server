#include "includes.h"

//breaks the command into words and returns an array of strings
char** break_command(char* command){
    int word_count = 0;

    //create a second copy of the command to tokenize it
    char* copy = malloc(strlen(command) + 1);
    check_ptr(copy, "break_command: malloc\n");
    strcpy(copy, command);

    //count the number of words in the command
    char* token = strtok(copy, " \n");
    while(token != NULL){
        word_count++;
        token = strtok(NULL, " \n");
    }

    char** args = malloc((word_count + 1) * sizeof(char*));
    check_ptr(args, "break_command: malloc\n");

    //tokenize the command and store the words in the array
    token = strtok(command, " \n");
    for(int i = 0; i < word_count; i++){
        args[i] = token;
        token = strtok(NULL, " \n");
    }

    //last element of the array must be NULL
    args[word_count] = NULL;
    free(copy);
    return args;
}

bool parse_command(Server server, char* buffer, int clientSocket){
    char *saveptr;
    // Get the first token
    char *token = strtok_r(buffer, " ", &saveptr);

    if(strcmp(token, "issueJob") == 0){
        //create an identifier for the job and add it to the queue
        pthread_mutex_lock(&server->conc_mutex);
        Identifier id = create_identifier(server->count_ids, saveptr, clientSocket);
        server->count_ids++;
        pthread_mutex_unlock(&server->conc_mutex);

        queue_produce(server->queue, id);

        //send a message to the client that the job has been submitted
        int len = strlen("JOB ") + strlen(id->job) +1 + strlen(id->jobID) + strlen(" SUBMITTED") + 1;
        char* message = (char*) malloc(len);
        check_ptr(message, "malloc failed in parse_command");
        sprintf(message, "JOB %s %s SUBMITTED", id->jobID, id->job);
        write_string_to_socket(clientSocket, message);
        free(message);
    }

    else if(strcmp(token, "setConcurrency") == 0){
        //set the concurrency to the new value
        pthread_mutex_lock(&server->conc_mutex);
        int old = server->concurrency;
        int new = atoi(saveptr);
        if(new < 0){
            write_string_to_socket(clientSocket, "CONCURRENCY CANNOT BE NEGATIVE, SET AT 0");
            server->concurrency = 0;
            pthread_mutex_unlock(&server->conc_mutex);
            return false;
        }

        server->concurrency = new;
        //if the new concurrency is greater than the old one, we need to wake up some threads
        if (old < server->concurrency)
            pthread_cond_broadcast(&server->can_run);
        pthread_mutex_unlock(&server->conc_mutex);

        //send a message to the client that the concurrency has been set
        int len = strlen("CONCURRENCY SET AT ") + strlen(saveptr) + 1;
        char message[len];
        sprintf(message, "CONCURRENCY SET AT %s", saveptr);
        write_string_to_socket(clientSocket, message);
    }

    else if(strcmp(token, "stop") == 0){
        //remove the job from the queue and send a message to the client
        Identifier id = queue_remove_by(server->queue, compare_identifiers, saveptr);
        if(id == NULL){
            int len = strlen("JOB ") + strlen(saveptr) + strlen(" NOTFOUND") + 1;
            char message[len];
            sprintf(message, "JOB %s NOT FOUND", saveptr);
            write_string_to_socket(clientSocket, message);
        }
        else{
            int len = strlen("JOB ") + strlen(saveptr) + strlen(" REMOVED") + 1;
            char message[len];
            sprintf(message, "JOB %s REMOVED", saveptr);
            write_string_to_socket(clientSocket, message);
            free_identifier(id);
        }
    }

    else if(strcmp(token, "poll") == 0)
        //send all the jobs in the queue to the client
        queue_send_all(server->queue, write_identifier, clientSocket);

    else if(strcmp(token, "exit") == 0){
        write_string_to_socket(clientSocket, "SERVER TERMINATED");
        return true;
    }

    return false;
}

// Creates the output filename for the job with the given pid
char* create_output_filename(pid_t pid) {
    int len = strlen("../out/") + count_digits(pid) + strlen(".output") + 1;
    char* output_filename = (char*) malloc(len);
    check_ptr(output_filename, "malloc failed in create_output_filename");
    sprintf(output_filename, "../out/%d.output", pid);
    return output_filename;
}

void execute_job(Identifier id){
    char** args = break_command(id->job);

    pid_t pid = fork();
    check_number(pid, "execute job: fork\n");

    // filename for the output of the job
    char* output_filename = NULL;

    if (pid == 0) {
        // Open the file for writing and redirection
        output_filename = create_output_filename(getpid());

        int fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        check_number(fd, "open");

        // Redirect stdout and stderr to the file
        check_number(dup2(fd, STDOUT_FILENO), "dup2\n");
        check_number(dup2(fd, STDERR_FILENO), "dup2\n");

        // Close the file descriptor as we have duplicated it
        close(fd);
        free(output_filename);

        execvp(args[0], args);
        print_error("execvp: Could not execute job\n");
    }
    else {
        waitpid(pid, NULL, 0);

        // Read the output file and send it to the client
        output_filename = create_output_filename(pid);

        write_file_contents_to_socket(id->clientSocket, output_filename, id->jobID);

        //remove the output file
        remove(output_filename);

        free(args);
        free(output_filename);
    }
}