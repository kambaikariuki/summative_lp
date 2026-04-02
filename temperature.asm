section .data
    filename db "temperature_data.txt", 0

    ; Output messages
    msg_total db "Total readings: ", 0
    msg_valid db "Valid readings: ", 0
    newline db 10, 0

    err_msg db "Error: Cannot open file", 10, 0

section .bss
    buffer resb 4096        ; buffer for file content
    total_count resq 1      ; total lines
    valid_count resq 1      ; non-empty lines

section .text
    global _start

_start:

; ------------------------------------------
; OPEN FILE
; sys_open(filename, O_RDONLY, 0)
; ------------------------------------------
    mov rax, 2              ; sys_open
    mov rdi, filename
    mov rsi, 0              ; O_RDONLY
    mov rdx, 0
    syscall

    cmp rax, 0
    js file_error           ; if < 0 → error

    mov r12, rax            ; save file descriptor

; ------------------------------------------
; READ FILE INTO MEMORY
; sys_read(fd, buffer, size)
; ------------------------------------------
    mov rax, 0              ; sys_read
    mov rdi, r12
    mov rsi, buffer
    mov rdx, 4096
    syscall

    mov r13, rax            ; bytes read

; ------------------------------------------
; INITIALIZE COUNTERS
; ------------------------------------------
    mov qword [total_count], 0
    mov qword [valid_count], 0

    mov rsi, buffer         ; pointer to buffer
    mov rcx, r13            ; number of bytes

    xor rbx, rbx            ; line_has_data flag (0 = empty)

; ==========================================
; LINE TRAVERSAL LOOP
; ==========================================
process_loop:
    cmp rcx, 0
    je done_processing

    mov al, [rsi]

; ------------------------------------------
; CHECK FOR NEWLINE (\n)
; ------------------------------------------
    cmp al, 10              ; '\n'
    je handle_newline

; ------------------------------------------
; IGNORE CR (\r)
; ------------------------------------------
    cmp al, 13              ; '\r'
    je skip_char

; ------------------------------------------
; NON-EMPTY CHARACTER FOUND
; ------------------------------------------
    mov rbx, 1              ; mark line as non-empty
    jmp skip_char

; ------------------------------------------
; HANDLE NEWLINE (end of line)
; ------------------------------------------
handle_newline:
    ; increment total line count
    inc qword [total_count]

    ; if line had data → increment valid count
    cmp rbx, 1
    jne reset_flag

    inc qword [valid_count]

reset_flag:
    xor rbx, rbx            ; reset line flag
    jmp skip_char

; ------------------------------------------
; MOVE TO NEXT CHARACTER
; ------------------------------------------
skip_char:
    inc rsi
    dec rcx
    jmp process_loop

; ==========================================
; END OF FILE HANDLING
; (handles last line if no newline at EOF)
; ==========================================
done_processing:
    cmp rbx, 0
    je print_results

    ; last line had data
    inc qword [total_count]
    inc qword [valid_count]

; ==========================================
; PRINT RESULTS
; ==========================================
print_results:

    ; print "Total readings: "
    mov rdi, msg_total
    call print_string

    mov rdi, [total_count]
    call print_number

    call print_newline

    ; print "Valid readings: "
    mov rdi, msg_valid
    call print_string

    mov rdi, [valid_count]
    call print_number

    call print_newline

    jmp exit

; ==========================================
; ERROR HANDLING
; ==========================================
file_error:
    mov rdi, err_msg
    call print_string
    jmp exit

; ==========================================
; PRINT STRING (null-terminated)
; ==========================================
print_string:
    push rdi
    mov rsi, rdi
    xor rcx, rcx

.find_len:
    cmp byte [rsi + rcx], 0
    je .len_found
    inc rcx
    jmp .find_len

.len_found:
    mov rax, 1      ; sys_write
    mov rdi, 1      ; stdout
    mov rdx, rcx
    pop rsi
    syscall
    ret

; ==========================================
; PRINT NUMBER (simple decimal)
; ==========================================
print_number:
    mov rax, rdi
    mov rcx, 10
    mov rbx, 0
    mov rsi, buffer

.convert:
    xor rdx, rdx
    div rcx
    add dl, '0'
    push rdx
    inc rbx
    cmp rax, 0
    jne .convert

.print:
    mov rax, 1
    mov rdi, 1

.next_digit:
    pop rsi
    mov [buffer], sil
    mov rsi, buffer
    mov rdx, 1
    syscall
    dec rbx
    jnz .next_digit
    ret

; ==========================================
; PRINT NEWLINE
; ==========================================
print_newline:
    mov rax, 1
    mov rdi, 1
    mov rsi, newline
    mov rdx, 1
    syscall
    ret

; ==========================================
; EXIT PROGRAM
; ==========================================
exit:
    mov rax, 60
    xor rdi, rdi
    syscall
