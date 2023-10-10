; A simple template for assembly programs.
.386  ; Specify instruction set
.model flat, stdcall  ; Flat memory model, std. calling convention
.stack 4096 ; Reserve stack space
ExitProcess PROTO, dwExitCode: DWORD  ; Exit process prototype

.data ; data segment
	; define your variables here
	x DWORD 35
	d DWORD 60
	y DWORD 45
	cc DWORD 55
	fifty DWORD 50
	store1 DWORD ?
	store2 DWORD ?
	quotient DWORD 0
    remainder DWORD 0
.code ; code segment

main PROC ; main procedure
	; write your assembly code here
	mov eax, x
	mul fifty
	mul d
	mov store1, eax

	mov eax, y
	mul fifty
	mul cc

	add store1, eax

	mov eax, cc
	mul d
	mov store2, eax

	mov edx, 0
	mov eax, store1
	mov ebx, store2
	div ebx
	mov quotient, eax
	mov remainder, edx
	INVOKE ExitProcess, 0 ; call exit function
  
main ENDP ; exit main procedure
END main  ; stop assembling