# Linux Summative Project

This repository contains solutions and analysis for five different questions related to Linux system programming, assembly, and Python C extensions.

## Question 1: Reverse Engineering a C Program

### Purpose
This question involves reverse engineering a C program (`question1`) to understand its functionality, control flow, and interactions with the operating system and files. The analysis is documented in `reverse_engineering_report.md`. The program processes student names, saves them to `students.txt`, sorts them, and saves the sorted names to `sorted_students.txt`.

### Files
- `question1`: The compiled executable to be reverse-engineered.
- `reverse_engineering_report.md`: A detailed report covering `objdump`, `strace`, and `gdb` analysis of the `question1` executable.
- `students.txt`: Contains unsorted student names (input/intermediate file).
- `sorted_students.txt`: Contains sorted student names (output file).
- `strace_output.txt`: The output of `strace` when run on the `question1` executable.

### How to Run/Use
1.  **Examine the Report**: Read `reverse_engineering_report.md` to understand the analysis performed.
2.  **Inspect Data Files**: View `students.txt` and `sorted_students.txt` to see the program's input and output.
3.  **Reproduce Analysis (Optional)**:
    *   To get `strace_output.txt`:
        ```bash
        strace ./question1 > strace_output.txt
        ```
    *   To use `objdump` and `gdb`, refer to the commands within `reverse_engineering_report.md`.

### Key Findings
The `question1` program takes student names, writes them to a file, reads them back, sorts them alphabetically, and writes the sorted names to a new file. The `reverse_engineering_report.md` details the functions, system calls, and memory interactions involved, providing insights into the program's execution flow.

## Question 2: Assembly Program for Line Counting

### Purpose
This question involves an assembly program (`question2.asm`) that reads a log file (`sensor_log.txt`) and counts the number of lines (sensor readings) within it.

### Files
- `question2.asm`: The NASM assembly source code for the line counting program.
- `question2.o`: The object file compiled from `question2.asm`.
- `question2`: The executable linked from `question2.o`.
- `sensor_log.txt`: A sample log file containing sensor readings, used as input for `question2`.

### How to Run/Use
1.  **Compile the Assembly Program**:
    ```bash
    nasm -f elf64 Question\ 2/question2.asm -o Question\ 2/question2.o
    ```
2.  **Link the Object File**:
    ```bash
    ld Question\ 2/question2.o -o Question\ 2/question2
    ```
3.  **Execute the Program**:
    ```bash
    ./Question\ 2/question2
    ```
    The program will output the total number of sensor readings to the console.

### Key Findings
The assembly program demonstrates fundamental system calls for file I/O (`sys_open`, `sys_read`, `sys_close`) and standard output (`sys_write`), along with basic string processing to count newline characters, effectively determining the number of lines in a file. It also includes error handling for file operations.

## Question 3: Python C Extension for Temperature Statistics

### Purpose
This question involves a Python C extension (`temp_stats.c`) that provides optimized functions for calculating statistics (min, max, average, variance, count) on NumPy arrays of temperature readings. A `setup.py` script is provided to build the extension, and `test.py` demonstrates its usage and verifies its functionality.

### Files
- `temp_stats.c`: The C source code implementing the temperature statistics functions, integrating with the Python C API and NumPy.
- `setup.py`: A Python script to build and install the `temp_stats` C extension using `setuptools`.
- `test.py`: A Python script to test the `temp_stats` module, including edge cases like empty and single-element arrays.
- `TemperatureStatistics.egg-info/`: Directory created during the build process.
- `build/`: Directory created during the build process, containing intermediate build files.

### How to Run/Use
1.  **Build and Install the C Extension**:
    Navigate to the `Question 3` directory and run:
    ```bash
    python setup.py install
    ```
    This will compile `temp_stats.c` and make the `temp_stats` module available to Python.

2.  **Run the Test Script**:
    From the `Question 3` directory, execute:
    ```bash
    python test.py
    ```
    This script will demonstrate the usage of the C extension functions and print the calculated statistics, including error handling for invalid inputs.

### Key Findings
This question highlights the integration of C code with Python using the C API and NumPy for performance-critical operations. The C extension provides efficient calculations for temperature statistics directly on NumPy arrays, showcasing the benefits of using compiled languages for numerical computing in Python. The `test.py` script ensures the robustness of the C extension by testing various scenarios, including empty and single-element inputs.

## Question 4: Producer-Consumer Problem (Barista-Waiter)

### Purpose
This question implements the classic Producer-Consumer problem using pthreads (mutexes and condition variables) in C. It simulates a barista (producer) making drinks and a waiter (consumer) serving them from a shared queue. This demonstrates thread synchronization and inter-thread communication.

### Files
- `barista_waiter.c`: The C source code implementing the barista and waiter threads, shared queue, and synchronization mechanisms.
- `barista_waiter.h`: Header file containing constants and function prototypes for the barista-waiter simulation.
- `barista_waiter`: The compiled executable of the simulation.

### How to Run/Use
1.  **Compile the Program**:
    ```bash
    gcc -o Question\ 4/barista_waiter Question\ 4/barista_waiter.c -pthread
    ```
2.  **Execute the Program**:
    ```bash
    ./Question\ 4/barista_waiter
    ```
    The program will run indefinitely, simulating the barista preparing drinks and the waiter serving them, printing messages to the console about the queue status.

### Key Findings
This program effectively demonstrates how to use `pthread_mutex_t` for critical section protection and `pthread_cond_t` for signaling between threads in a producer-consumer scenario. The barista waits when the queue is full, and the waiter waits when the queue is empty, ensuring proper synchronization and preventing race conditions or deadlocks.

## Question 5: Multi-client Server and Client for an Exam System

### Purpose
This question implements a multi-client TCP server and a corresponding client in C. It simulates an online exam system where multiple students (clients) can connect to a server, authenticate themselves, receive an exam question, submit answers, and receive immediate feedback. The server also broadcasts a list of active students. This demonstrates socket programming, multi-threading, and inter-process communication.

### Files
- `server.c`: The C source code for the multi-threaded exam server.
- `client.c`: The C source code for the exam client.
- `common.h`: Header file defining shared constants (like `PORT`, `BUFFER_SIZE`) and message structures (`MessageType`, `Message`).
- `Makefile`: A makefile to compile both the server and client executables.
- `server`: The compiled server executable.
- `client`: The compiled client executable.
- `server_log.txt`: An empty file, presumably for server logging (though not actively used in the provided code).

### How to Run/Use
1.  **Compile the Programs**:
    Navigate to the `Question 5` directory and run `make`:
    ```bash
    cd Question\ 5
    make
    ```
    This will compile both `server.c` and `client.c` to create `server` and `client` executables.

2.  **Start the Server**:
    In one terminal, navigate to the `Question 5` directory and run the server:
    ```bash
    ./server
    ```
    The server will start listening on `PORT` 8080.

3.  **Start Clients**:
    In separate terminals, navigate to the `Question 5` directory and run the client:
    ```bash
    ./client
    ```
    Each client will prompt for a username. Valid usernames are `student1`, `student2`, `student3`, `student4`. After successful authentication, clients will receive an exam question, can submit answers, and receive feedback. The server will broadcast active student updates to all connected clients.

### Key Findings
This project showcases robust network programming techniques including socket creation, binding, listening, and accepting connections. It effectively uses multi-threading with `pthread` to handle concurrent client connections, preventing blocking operations. Synchronization is handled using mutexes for shared resources (like the list of active clients). The message-passing mechanism between client and server, defined in `common.h`, demonstrates a clear protocol for communication, including authentication, question delivery, answer submission, and feedback.

