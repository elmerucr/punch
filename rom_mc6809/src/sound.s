		include	"definitions.i"

		global	sound_reset

		section	TEXT

sound_reset:	pshu	y,x,a
		ldx	#$0080
		ldy	#SND
.1		clr	,y+
		leax	-1,x
		bne	.1

		lda	#$7f	; mixer at half volume
		ldx	#$0010
		ldy	#SNDM
.2		sta	,y+
		leax	-1,x
		bne	.2

		lda	#$0f	; set sid volumes to max
		sta	SID0V
		sta	SID1V

		pulu	y,x,a
		rts
