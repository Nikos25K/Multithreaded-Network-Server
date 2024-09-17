#ifndef UTILS_H
#define UTILS_H

//checks if the pointer is NULL and exits the program if it is
//used for malloc and other functions that return a pointer
void check_ptr(void* ptr, char* message);

//checks if the number is negative and exits the program if it is
//used for reading, writing, opening files etc
void check_number(int num, char* message);

//prints an error message and exits the program
void print_error(char* message);

//checks if there is an error in a thread function and exits the program if there is
void check_thread(int error, char* message);

//checks if the string is a number and exits the program if it is not
void check_str_to_num(char* str, char* message);

int count_digits(int number);

//checks if the arguments are correct and exits the program if they are not
void check_commander_arguments(int argc, char* argv[]);
void check_executor_arguments(int argc, char* argv[]);

//the Commander waits for the Executor's response
void handle_response(char* arg, int clientSocket);


#endif