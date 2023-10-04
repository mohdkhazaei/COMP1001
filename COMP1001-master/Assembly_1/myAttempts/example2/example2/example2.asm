; A simple template for assembly programs.
.386  ; Specify instruction set
.model flat, stdcall  ; Flat memory model, std. calling convention
.stack 4096 ; Reserve stack space
ExitProcess PROTO, dwExitCode: DWORD  ; Exit process prototype

.data ; data segment
	; define your variables here
	result SDWORD ?
	
.code ; code segment

main PROC ; main procedure
	; write your assembly code here

	MOV eax, 10 ; assign 10 to eax register
	MOV ebx, 5 ; assign 5 to ebx register
	MOV ecx, 15 ; assign 15 to ecx register
	MOV edx, 20 ; assign 20 to edx register

	ADD eax, ebx ; 10 + 5
	ADD ecx, edx ; 15 + 20

	SUB ecx, eax ; 35 - 15

	MOV result, eax ; move the answer to variable



	INVOKE ExitProcess, 0 ; call exit function
  
main ENDP ; exit main procedure
END main  ; stop assembling