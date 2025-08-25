		include	"definitions.i"


		section	VECTORS_GENERAL

.0		jmp	[VECTOR_ILLOP_INDIRECT]		; $ffd4 (instr is 4 bytes)
.1		jmp	[VECTOR_SWI3_INDIRECT]		; $ffd8
.2		jmp	[VECTOR_SWI2_INDIRECT]		; $ffdc
.3		jmp	[VECTOR_FIRQ_INDIRECT]		; $ffe0
.4		jmp	[VECTOR_IRQ_INDIRECT]		; $ffe4
.5		jmp	[VECTOR_SWI_INDIRECT]		; $ffe8
.6		jmp	[VECTOR_NMI_INDIRECT]		; $ffec


		section	VECTORS_SYSTEM

		dw	.0
		dw	.1
		dw	.2
		dw	.3
		dw	.4
		dw	.5
		dw	.6
		dw	exc_reset
