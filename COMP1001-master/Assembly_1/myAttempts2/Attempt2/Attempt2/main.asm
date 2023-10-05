; A simple template for assembly programs.
.386  ; Specify instruction set
.model flat, stdcall  ; Flat memory model, std. calling convention
.stack 4096 ; Reserve stack space
ExitProcess PROTO, dwExitCode: DWORD  ; Exit process prototype

.data ; data segment
	; define your variables here
	Answer DWORD 0
	A BYTE ?
.code ; code segment

main PROC ; main procedure
	; write your assembly code here
	mov eax, 10
	mov ebx, 15
	mov ecx, 20
	mov edx, 25

	ADD eax, ebx
	ADD ecx, edx

	SUB ecx, eax
	mov Answer, ecx

	INVOKE ExitProcess, 0 ; call exit function
  
main ENDP ; exit main procedure
END main  ; stop assembling