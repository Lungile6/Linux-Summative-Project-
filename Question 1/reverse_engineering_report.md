# Reverse Engineering Report: question1

## 1. Objdump Analysis

### Identified Functions:

```
_init
.plt (Procedure Linkage Table)
__cxa_finalize@plt
strcpy@plt
puts@plt
fclose@plt
__stack_chk_fail@plt
printf@plt
strcspn@plt
fgets@plt
strcmp@plt
fprintf@plt
fopen@plt
perror@plt
exit@plt
_start
deregister_tm_clones
register_tm_clones
__do_global_dtors_aux
frame_dummy
main
getStudentNames
writeToFile
readFromFile
sortNames
printNames
_fini
```

### Example Jump Instruction:

```assembly
100f:   48 85 c0             test   %rax,%rax
1012:   74 02                je     1016 <_init+0x16>
1014:   ff d0                call   *%rax
```

**Explanation:**
At address `100f`, the `test %rax,%rax` instruction performs a bitwise AND operation between the `rax` register and itself, setting the CPU's flags (specifically the Zero Flag, ZF).

The subsequent instruction at `1012` is `je 1016 <_init+0x16>`, a conditional jump.

*   **If `rax` is zero (ZF is set)**: The program will jump to address `1016`, skipping the `call *%rax` instruction.
*   **If `rax` is non-zero (ZF is not set)**: The program will *not* jump and will continue to execute `call *%rax` at `1014`, transferring control to the address stored in `rax`.

This demonstrates how control flow is altered based on the value of a register.


## 2. Strace Analysis

### Identified System Calls:

```
execve: Executes the binary.
Memory Management (brk, mmap, mprotect, munmap): Dynamic memory allocation, loading libraries.
File I/O (access, openat, newfstatat, read, write, close): Opening/closing files, reading/writing data.
Process Control (arch_prctl, set_tid_address, set_robust_list, rseq, prlimit64, exit_group): Process-related configurations and termination.
getrandom: Obtaining random data.
```

### Memory/I/O Interaction:

*   **Memory Interaction**: `brk`, `mmap`, `mprotect`, and `munmap` are heavily used for dynamic memory allocation, especially for loading the executable, shared libraries (`libc.so.6`), and allocating space for program data like student names.
*   **I/O Interaction**:
    *   **File System**: `openat`, `read`, `write`, and `close` are used to interact with `students.txt` (read/write) and `sorted_students.txt` (write), as well as to load system libraries.
    *   **Standard I/O**: `write(1, ...)` sends prompts to the console, and `read(0, ...)` attempts to read user input from the console.


## 3. GDB Analysis

### Breakpoints and Execution Flow:

Breakpoints were set at the entry points of the `main`, `getStudentNames`, `writeToFile`, `readFromFile`, `sortNames`, and `printNames` functions.

Execution flow observed:
1.  `main` (program entry)
2.  `getStudentNames` (called from `main` to collect input)
3.  `writeToFile` (called from `main` to save initial names)
4.  `readFromFile` (called from `main` to load names)
5.  `sortNames` (called from `main` to sort names)
6.  `writeToFile` (called from `main` to save sorted names)

The `printNames` function was not hit during this execution, indicating it's either not called or called conditionally under different circumstances.

### Function Transitions and Memory Accesses:

*   **`main`**: Initializes the program, calls other core functions in sequence to handle name input, file operations, and sorting.
*   **`getStudentNames`**: Called by `main`. Likely responsible for reading user input (student names) and storing them into an array passed as an argument (likely a buffer on `main`'s stack frame). The `main` function allocates a large stack frame (`sub $0x3f0,%rsp`) which likely holds this array.
*   **`writeToFile`**: Called by `main` twice. The first call writes the unsorted names (from the buffer populated by `getStudentNames`) to `students.txt`. The second call writes the sorted names (from the buffer modified by `sortNames`) to `sorted_students.txt`.
*   **`readFromFile`**: Called by `main`. Reads names from `students.txt` into a memory buffer (likely the same or a similar buffer in `main`'s stack frame).
*   **`sortNames`**: Called by `main`. Processes the names in the memory buffer (populated by `readFromFile`) and sorts them in place, modifying the contents of that buffer.
*   **Notable Memory Accesses**: Primarily involves stack memory for local variables and the array of student names (within `main`'s stack frame). Global memory would be used for string literals (prompts, filenames). Heap memory usage, if any, is implicit through standard library calls, but not directly observable as specific allocations related to the student name data itself from this GDB analysis without source or more detailed stepping.


## 4. Summary

### Control Flow Diagram/Textual Explanation:

The program begins execution in `_start`, which sets up the environment and eventually calls `main`. The `main` function orchestrates the primary logic. Its control flow is largely sequential, calling several sub-functions in a defined order:
1.  `main` initializes.
2.  Calls `getStudentNames` to collect student names from user input.
3.  Calls `writeToFile` to save the unsorted names to `students.txt`.
4.  Calls `readFromFile` to load names from `students.txt` into memory.
5.  Calls `sortNames` to arrange the names in alphabetical order.
6.  Calls `writeToFile` again to save the sorted names to `sorted_students.txt`.

Conditional jumps (e.g., `je` in `_init`) and unconditional jumps (`jmp`) are present in the low-level assembly to manage program startup, function calls, and control program flow based on conditions.

### Identified Function Calls and Their Purposes:

*   **`main`**: The orchestrator; manages the overall flow of the program.
*   **`getStudentNames`**: Prompts the user for a specified number of student names and stores them in an in-memory array.
*   **`writeToFile`**: Opens a specified file, and writes the contents of an in-memory array (student names) to it.
*   **`readFromFile`**: Opens a specified file, reads its contents, and populates an in-memory array with the data.
*   **`sortNames`**: Takes an array of names in memory and sorts them (likely alphabetically) using a comparison algorithm (indicated by `strcmp` calls).
*   **`printNames`**: (Not executed in our analysis) Presumably designed to print the names to standard output.
*   Standard library functions (`printf`, `fgets`, `fopen`, `fclose`, `perror`, `strcpy`, `strcspn`, `strcmp`, `exit`, etc.) are used for various tasks like console I/O, file operations, string manipulation, and error handling.

### Notable Memory Accesses and Context:

*   **Stack**: Heavily used for local variables within functions and for passing arguments. `main` allocates a substantial stack frame, likely to hold the array of student names. Function calls push return addresses and save registers onto the stack.
*   **Global Data (`.rodata`, `.data`)**: Stores string literals (e.g., prompts, filenames) and potentially other global variables.
*   **Heap**: Implicitly used by standard library functions (e.g., `fopen` might allocate memory for `FILE` structures), as indicated by `brk` and `mmap` system calls during `strace`. Direct user-level heap allocations for the primary data (student names) were not explicitly identified but cannot be ruled out without source code.

### Strace Output and Runtime Behavior:

`strace` revealed that the program performs significant system-level interactions:
*   **Memory Management**: Extensive use of `mmap`, `brk`, `mprotect`, and `munmap` to allocate/deallocate memory for the executable itself, shared libraries (e.g., `libc.so.6`), and program data.
*   **File I/O**: Opens and closes `students.txt` for both writing and reading, and `sorted_students.txt` for writing. This confirms the program's persistent storage of data. It also accesses system-level files like `/etc/ld.so.cache` and `/lib/x86_64-linux-gnu/libc.so.6` during startup.
*   **Standard I/O**: `write` calls to file descriptor 1 (stdout) display prompts to the user, and `read` calls from file descriptor 0 (stdin) are used to collect user input.

The runtime behavior involves a clear sequence of operations: program setup, user interaction (input names), saving to file, loading from file, processing (sorting), and finally saving the processed data to another file.
