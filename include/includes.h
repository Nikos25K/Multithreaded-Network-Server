#ifndef INCLUDES_H
#define INCLUDES_H

//system includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>


//custom includes
#include "utils.h"
#include "queue.h"
#include "sockets.h"
#include "structs.h"
#include "threads.h"
#include "parsing.h"

#endif