# Job Executor Server

Job Executor Server is a multithreaded server designed to handle and execute jobs submitted by multiple clients (`jobCommanders`). It uses a producer-consumer model with a queue system, allowing jobs to be executed by worker threads. The server and its components are highly configurable, supporting dynamic thread pools, job cancellation, and job monitoring. Clients communicate with the server via sockets, submitting commands for job execution and server management.

## Table of Contents

- [Project Structure](#project-structure)
- [Scripts](#scripts)
- [Program Flow](#program-flow)
- [Commands](#commands)
  - [issueJob](#issuejob)
  - [setConcurrency](#setconcurrency)
  - [stop](#stop)
  - [poll](#poll)
  - [exit](#exit)
- [Signal Handling](#signal-handling)

## Project Structure

The project consists of the following 6 folders:

1. **bin**: Contains the executables generated from the compilation.
2. **build**: Contains the object files generated from the compilation.
3. **include**: Contains all the `.h` files. The `includes.h` file includes all the other headers and is used by all `.c` files.
4. **out**: Contains temporary `pid.output` files generated during job execution, which are used for sending the job output to the corresponding `jobCommander`. These files will be deleted after the program's execution.
5. **src**: Contains the source code.
6. **tests**: Includes tests provided in the first assignment, with some modifications (explained below).

> **Note**: The `lib` folder is omitted because the program does not use external libraries.

### Header Files and Their Corresponding C Files

1. **queue.h**: Implements the queue. This includes functions that use function pointers for comparing (`CompareFunc`), writing to a file descriptor (`WriteFunc`), and freeing memory (`FreePtr`). The queue follows a producer-consumer model, using condition variables and mutexes to block threads when necessary.
2. **structs.h**: Contains the structures and related functions used in the project. Specifically, it defines the `Server` structure and the `Identifiers` stored in the queue.
3. **sockets.h**: Contains functions for creating, reading/writing to a socket, and deleting a socket.
4. **threads.h**: Defines the server's thread functions, thread creation, joining of worker threads, and the main loop that accepts new connections from jobCommanders.
5. **utils.h**: Contains various utility functions for error checking, pointer validation, argument checking, etc.
6. **parsing.h**: Implements functions responsible for "understanding" input data and executing the corresponding operations.

### Scripts

- **run_server.sh**: A helper script that:
  1. Compiles the program and grants execution permissions to existing tests (if not already set), as well as to the generated executables.
  2. Starts the server.

   The program can be compiled either by using the `make` command or by running this script.
   Execution syntax: `./run_server.sh <port> <bufferSize> <threadPoolSize>`
   Default values for these arguments (if not provided) are `7856`, `8`, and `5` for the port number, queue size, and number of worker threads, respectively.

- **run_test.sh**: A helper script that:
  - Runs the test with the specified test number (from 1 to 8).

  Execution syntax: `./run_test.sh <test_number>`
  Each test uses default values `linux18.di.uoa.gr` and port `7856`, unless provided with other arguments (the first argument is the machine number, the second is the port number).

### Program Flow

1. The server is created and the arguments are checked for validity.
2. The queue and `main_thread` are created. The `main_thread` spawns the specified number of `worker_threads`.
3. A socket is created and the server enters an infinite loop waiting for connections.
4. When a `jobCommander` connects with valid arguments, the server creates a `controller_thread`, which handles the commands received through the socket.
5. The server sends a message to the `jobCommander`, and the commander terminates unless it is waiting for a job output.
6. If a `jobCommander` sends an `exit` command, the server completes the currently running jobs.
7. An informational message is sent to all `jobCommanders` waiting for job results, indicating that the server shut down before job execution.
8. The server terminates.

### Signal Handling

The only signal used is `SIGUSR1`, sent to the `main_thread` to indicate server termination after completing all running jobs. This signal is sent by a `controller_thread` when an `exit` command is issued.

### Commands

- **issueJob**:
  The `jobCommander` sends a message, which is read by the `controller_thread`. If there is space in the queue, a structure is created and enqueued. If the queue is full, the thread blocks until a spot opens. Once enqueued, the `jobCommander` is notified, and the job output is sent once available. A `worker_thread` handles the job execution, stores the output in a temporary file, and sends it to the `jobCommander` in chunks of 1024 bytes. Afterward, the server deletes the temporary file.

- **setConcurrency**:
  Adjusts the degree of concurrency based on the input. If the number is negative, concurrency is set to 0. There is no upper limit, and the user can specify any number. If the concurrency increases, new threads start processing queued jobs. If concurrency decreases, currently running threads continue, but no new jobs are processed until the number of running threads matches the concurrency level.

- **stop**:
  If the given job is in the queue, it is removed, and a success message is sent to the `jobCommander`. If the job is already executing, no action is taken, and a message is sent indicating that the job was not found.

- **poll**:
  Sends the contents of the queue (i.e., the jobs waiting to be executed) to the `jobCommander`. If the queue is empty, a "Queue is empty" message is displayed.

- **exit**:
  Sends a `SIGUSR1` signal to the `main_thread`, stopping the server after completing all currently running jobs. The queue is cleared, and all `jobCommanders` waiting for results are notified. `NULL` values are inserted into the queue for each `worker_thread` to signal them to terminate.

In all commands except `issueJob`, the `controller_thread` handles execution, updating concurrency, removing jobs, and signaling the `main_thread` for termination. `worker_threads` are responsible for executing jobs in the queue.
