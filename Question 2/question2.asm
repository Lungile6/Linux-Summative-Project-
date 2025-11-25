; question2.asm - Assembly program to count lines in sensor_log.txt
; Compliation and execution: (using NASM and LD on Linux)
;   nasm -f elf64 question2.asm -o question2.o
;   ld question2.o -o question2
;   ./question2

section .data
    ; Constants for system calls
    SYS_OPEN    equ 2       ; sys_open system call number
    SYS_READ    equ 0       ; sys_read system call number
    SYS_WRITE   equ 1       ; sys_write system call number
    SYS_CLOSE   equ 3       ; sys_close system call number
    SYS_EXIT    equ 60      ; sys_exit system call number

    ; File open flags
    O_RDONLY    equ 0       ; Read-only access

    ; Standard file descriptors
    STDIN       equ 0
    STDOUT      equ 1
    STDERR      equ 2

    ; Filename for sensor data
    filename db "sensor_log.txt", 0
    filename_len equ $ - filename

    ; Message to display the total count
    msg db "Total sensor readings: ", 0
    msg_len equ $ - msg

    ; Buffer to store file contents (increased size)
    file_buffer_size equ 4096
    file_buffer resb file_buffer_size

    ; Buffer for converting number to string
    num_buffer resb 20
    num_buffer_len equ $ - num_buffer

section .text
    global _start

_start:
    ; --- File Handling: Open sensor_log.txt ---
    ; rax = SYS_OPEN (2)
    ; rdi = filename (pointer to "sensor_log.txt")
    ; rsi = O_RDONLY (0)
    mov rax, SYS_OPEN
    mov rdi, filename
    mov rsi, O_RDONLY
    syscall
    mov r12, rax          ; Store file descriptor in r12

    ; Check for file open error (fd < 0)
    cmp r12, 0
    jl exit_error         ; If fd is negative, an error occurred

    ; --- Load File Contents into Memory ---
    ; rax = SYS_READ (0)
    ; rdi = file_descriptor (r12)
    ; rsi = file_buffer (address of buffer)
    ; rdx = file_buffer_size (max bytes to read)
    mov rax, SYS_READ
    mov rdi, r12          ; file descriptor
    mov rsi, file_buffer  ; buffer to read into
    mov rdx, file_buffer_size ; max bytes to read
    syscall
    mov r13, rax          ; Store bytes_read in r13

    ; Check for read error (bytes_read < 0)
    cmp r13, 0
    jl exit_error         ; If bytes_read is negative, an error occurred

    ; --- File Handling: Close sensor_log.txt ---
    ; rax = SYS_CLOSE (3)
    ; rdi = file_descriptor (r12)
    mov rax, SYS_CLOSE
    mov rdi, r12
    syscall

    ; --- String Traversal and Line Counting Logic ---
    ; Initialize line_count = 0
    ; rbx will hold the line count
    xor rbx, rbx          ; rbx = 0
    
    ; Pointer to current position in buffer
    ; rsi will serve as our buffer pointer
    mov rsi, file_buffer

    ; Loop through the buffer until bytes_read (r13) are processed
    ; rcx will be our loop counter (current byte index)
    xor rcx, rcx          ; rcx = 0

count_loop:
    cmp rcx, r13          ; Compare current index with total bytes read
    jae end_count_loop    ; If rcx >= r13, we've processed all bytes

    ; Check if current character is a newline (0xA)
    mov al, byte [rsi + rcx]
    cmp al, 0xA
    jne next_char

    ; If it's a newline, increment line_count
    inc rbx

next_char:
    inc rcx               ; Move to the next character
    jmp count_loop        ; Continue loop

end_count_loop:
    ; If the file is not empty and the last character is NOT a newline,
    ; we need to count the last line.
    cmp r13, 0            ; Check if file was empty
    je display_count      ; If empty, no need for extra check

    ; Check the last character of the buffer
    mov al, byte [rsi + r13 - 1] ; rsi is file_buffer, r13 is bytes_read
    cmp al, 0xA
    je display_count      ; If last char is newline, all lines counted

    ; If last char is not newline and file is not empty, increment count for last line
    inc rbx

display_count:
    ; --- Convert Count to String and Display ---
    ; Convert rbx (line_count) to ASCII string in num_buffer
    ; r8 will be a pointer to the end of num_buffer for writing digits backwards
    mov r8, num_buffer + num_buffer_len - 1 ; Point to end of num_buffer
    mov byte [r8], 0        ; Null-terminate the string
    mov rax, rbx            ; Move line_count to rax for division
    mov r9, 10              ; Divisor (10)

    cmp rax, 0              ; Handle case where count is 0
    jne convert_loop_start
    dec r8                  ; Move pointer back for '0'
    mov byte [r8], '0'      ; Place '0' in buffer
    jmp print_result

convert_loop_start:
    xor rdx, rdx            ; Clear rdx for division
    div r9                  ; rax = rax / 10, rdx = rax % 10
    add dl, '0'             ; Convert remainder to ASCII digit
    dec r8                  ; Move pointer back
    mov byte [r8], dl       ; Store digit
    cmp rax, 0              ; Check if quotient is 0
    jne convert_loop_start  ; Continue if quotient is not 0

print_result:
    ; --- Display "Total sensor readings: " ---
    ; rax = SYS_WRITE (1)
    ; rdi = STDOUT (1)
    ; rsi = msg (pointer to message string)
    ; rdx = msg_len
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    mov rsi, msg
    mov rdx, msg_len
    syscall

    ; --- Display the converted count (number_string) ---
    ; rax = SYS_WRITE (1)
    ; rdi = STDOUT (1)
    ; rsi = r8 (pointer to the start of the number string)
    ; rdx = length of the number string
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    mov rsi, r8             ; Start of number string
    mov rdx, num_buffer + num_buffer_len - 1 ; End of num_buffer
    sub rdx, r8             ; Calculate length
    syscall

    ; --- Display a newline character ---
    mov rax, SYS_WRITE
    mov rdi, STDOUT
    mov rsi, newline_char
    mov rdx, 1
    syscall

    ; --- Program Termination (Exit Success) ---
    ; rax = SYS_EXIT (60)
    ; rdi = 0 (exit code)
    mov rax, SYS_EXIT
    xor rdi, rdi
    syscall

exit_error:
    ; --- Program Termination (Exit Error) ---
    ; rax = SYS_EXIT (60)
    ; rdi = 1 (exit code for error)
    mov rax, SYS_EXIT
    mov rdi, 1
    syscall

section .data
    newline_char db 0xA
