.CODE

GetCurrentProcessCR3 PROC
	mov		rax, cr3;

	ret;
GetCurrentProcessCR3 ENDP;

SetCurrentProcessCR3 PROC
	cli;

	mov		cr3, rcx;

	sti;
	
	ret;
SetCurrentProcessCR3 ENDP;

END