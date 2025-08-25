		include	"definitions.i"

		global	core_frame_done_irq
		global	core_load_bin_irq
		global	core_load_squirrel_irq

		section	TEXT

core_frame_done_irq:
	clr	BLITTER_DST	; surface 0 (screen buffer) will be the destination

	lda	#$01
	sta	BLITTER_COLOR	; set color to black

	lda	#%00000100	; clear surface
	sta	BLITTER_CR

	lda	#$0c		; set source surface to logo
	sta	BLITTER_SRC
	lda	#%00000001	; and blit this to the framebuffer
	sta	BLITTER_CR

	lda	#$0e
	sta	BLITTER_SRC
	lda	#$0d
	sta	BLITTER_TILE
	lda	#%00000010
	sta	BLITTER_CR
	rti

core_load_bin_irq:
	; temp hack
	lda	$04c1
	adda	#$23
	sta	$04c1
	rti

core_load_squirrel_irq:
	; alternative irq routine that connects to commander lua

	; turn off all timer interrupts
	clr	TIMER_CR
	; turn off all core interrupts
	clr	CORE_CR

	orcc	#%00010000	; disable irqs
	ldx	#exc_irq_squirrel
	stx	VECTOR_IRQ_INDIRECT
	andcc	#%11101111	; enable irqs

	lda	#%10000000
	sta	COMMANDER_SQ_CR	; run init in squirrel script

	rti
