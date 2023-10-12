; A simple example adding two numbers.
.386  ; Specify instruction set
.model flat, stdcall  ; Flat memory model, std. calling convention
.stack 4096 ; Reserve stack space
ExitProcess PROTO, dwExitCode: DWORD  ; Exit process prototype



.data ; data segment

; define your variables here

my_byte_array BYTE 1,2,3,4,5,6,7,8,9 ; this an array of bytes
my_dword_array DWORD 1,2,3,4,5,6,7,8,9 ;this is an array of DWORDS
my_var DWORD 99 ; this is DWORD variable
My_array_2 DWORD 12345678h, 23456789h, 3456789Ah, 456789ABh
int_Array DWORD 4,3,5,6
array DWORD 5, 10, 15,20

; WHAT IS THE SIZE OF THE ABOVE ARRAYS IN BYTES?

.code ; code segment


main PROC ; main procedure

; write your assembly code here


lea eax, int_array; load effective address
mov ebx, [eax + TYPE int_array * 1]
 mov ecx, 16
mov [eax + TYPE int_array * 1], ecx


lea eax, array
mov ecx, 8
mov ebx, [eax + TYPE array * 0]
mul ecx
mov [eax + TYPE array * 0], ebx

;--------------------------------------------

lea eax, array
mov ecx, 8
mov esi, 0

mov ebx, [eax + TYPE array * esi]
mul ecx

mov [eax + TYPE array * esi], eax
inc esi


    INVOKE ExitProcess, 0 ; call exit function

main ENDP ; exit main procedure
END main  ; stop assembling 