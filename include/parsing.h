#ifndef PARSING_H
#define PARSING_H

//to parse the command from commander
bool parse_command(Server server, char* buffer, int clientSocket);

//execute the job of the given identifier
void execute_job(Identifier id);

#endif