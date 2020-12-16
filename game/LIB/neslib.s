;NES hardware-dependent functions by Shiru (shiru@mail.ru)
;with improvements by VEG
;Feel free to do anything you want with this code, consider it Public Domain

;nesdoug version, 2019-09
;minor change %%, added ldx #0 to functions returning char
;removed sprid from c functions to speed them up




	.export _pal_all,_pal_bg,_pal_spr,_pal_col,_pal_clear
	.export _pal_bright,_pal_spr_bright,_pal_bg_bright
	.export _ppu_off,_ppu_on_all,_ppu_on_bg,_ppu_on_spr,_ppu_mask,_ppu_system
	.export _oam_clear,_oam_size,_oam_spr,_oam_meta_spr,_oam_hide_rest
	.export _ppu_wait_frame,_ppu_wait_nmi
	.export _scroll,_split
	.export _bank_spr,_bank_bg
	.export _vram_read,_vram_write
	.export _music_play,_music_stop,_music_pause
	.export _sfx_play,_sample_play
	.export _pad_poll,_pad_trigger,_pad_state
	.export _rand8,_rand16,_set_rand
	.export _vram_adr,_vram_put,_vram_fill,_vram_inc,_vram_unrle
	.export _set_vram_update,_flush_vram_update
	.export _memcpy,_memfill,_delay
	
	.export _flush_vram_update_nmi, _oam_set, _oam_get


;NMI handler

nmi:
	pha
	txa
	pha
	tya
	pha

	; check for credits in the first slot
	lda CREDITS1_PREV
	and #%00100000
	beq @check_slot_2_creds
	lda $4016
	and #%00100000
	bne @check_slot_2_creds
	inc CREDITS_QUEUED

@check_slot_2_creds:
	lda $4016
	sta CREDITS1_PREV

	; check for credits in the second slot
	lda CREDITS2_PREV
	and #%01000000
	beq @done_credit_check
	lda $4016
	and #%01000000
	bne @done_credit_check
	inc CREDITS_QUEUED

@done_credit_check:
	lda $4016
	sta CREDITS2_PREV	


	; if prev_val_a == 1
		; if new_val_a == 0
			; inc CREDITS_QUEUED
	; prev_val_a = new_val_b
	; repeat for b

		; check for zero
		; if now zero

;on the Vs system, check for credits!
;todo: check for 1 and 2 independently. hook up to c logic.
; 	lda CREDITS_COUNTDOWN
; 	beq @check_for_creds
; 	dec CREDITS_COUNTDOWN
; 	jmp @skip_creds

; @check_for_creds:
; 	lda $4016
; 	and #%01100000
; 	beq @skip_creds
; 	inc CREDITS_QUEUED
; 	lda #70
; 	sta CREDITS_COUNTDOWN
	
;@skip_creds:

	lda <PPU_MASK_VAR	;if rendering is disabled, do not access the VRAM at all
	and #%00011000
	bne @doUpdate
	jmp	@skipAll

@doUpdate:

	lda #>OAM_BUF		;update OAM
	sta PPU_OAM_DMA

	lda <PAL_UPDATE		;update palette if needed
	bne @updPal
	jmp @updVRAM

@updPal:

	ldx #0
	stx <PAL_UPDATE

	lda #$3f ; 63
	sta PPU_ADDR
	stx PPU_ADDR

	ldy PAL_BUF				;background color, remember it in X
	lda (PAL_BG_PTR),y		;pointer to palBrightTable
	sta PPU_DATA
	tax
	
	.repeat 3,I
	ldy PAL_BUF+1+I
	lda (PAL_BG_PTR),y
	sta PPU_DATA
	.endrepeat

	.repeat 3,J		
	stx PPU_DATA			;background color
	.repeat 3,I
	ldy PAL_BUF+5+(J*4)+I
	lda (PAL_BG_PTR),y
	sta PPU_DATA
	.endrepeat
	.endrepeat

	.repeat 4,J		
	stx PPU_DATA			;background color
	.repeat 3,I
	ldy PAL_BUF+17+(J*4)+I
	lda (PAL_SPR_PTR),y
	sta PPU_DATA
	.endrepeat
	.endrepeat

@updVRAM:

	lda <VRAM_UPDATE
	beq @skipUpd
	lda #0
	sta <VRAM_UPDATE
	
	lda <NAME_UPD_ENABLE
	beq @skipUpd

	jsr _flush_vram_update_nmi

@skipUpd:

	lda #0
	sta PPU_ADDR
	sta PPU_ADDR

	lda <SCROLL_X
	sta PPU_SCROLL
	lda <SCROLL_Y
	sta PPU_SCROLL

	lda <PPU_CTRL_VAR
	sta PPU_CTRL

@skipAll:

	lda <PPU_MASK_VAR
	sta PPU_MASK

	inc <FRAME_CNT1
	inc <FRAME_CNT2
	lda <FRAME_CNT2
	cmp #6
	bne @skipNtsc
	lda #0
	sta <FRAME_CNT2

@skipNtsc:

	jsr FamiToneUpdate

	pla
	tay
	pla
	tax
	pla

irq:

    rti



;void __fastcall__ pal_all(const char *data);

_pal_all:

	sta <PTR
	stx <PTR+1
	ldx #$00
	lda #$20

pal_copy:

	sta <LEN

	ldy #$00

@0:

	lda (PTR),y
	sta PAL_BUF,x
	inx
	iny
	dec <LEN
	bne @0

	inc <PAL_UPDATE

	rts



;void __fastcall__ pal_bg(const char *data);

_pal_bg:

	sta <PTR
	stx <PTR+1
	ldx #$00
	lda #$10
	bne pal_copy ;bra



;void __fastcall__ pal_spr(const char *data);

_pal_spr:

	sta <PTR
	stx <PTR+1
	ldx #$10
	txa
	bne pal_copy ;bra



;void __fastcall__ pal_col(unsigned char index,unsigned char color);

_pal_col:

	sta <PTR
	jsr popa
	and #$1f
	tax
	lda <PTR
	sta PAL_BUF,x
	inc <PAL_UPDATE
	rts



;void __fastcall__ pal_clear(void);

_pal_clear:

	lda #$0f
	ldx #0

@1:

	sta PAL_BUF,x
	inx
	cpx #$20
	bne @1
	stx <PAL_UPDATE
	rts



;void __fastcall__ pal_spr_bright(unsigned char bright);

_pal_spr_bright:

	tax

.if(VS_SYS_ENABLED)	
	lda PPU_VERSION
	beq @PPU0
	
	cmp #1
	beq @PPU1
	
	cmp #2
	beq @PPU2
	
	cmp #3
	beq @PPU3
	
	cmp #4
	beq @PPU4
	
	cmp #5
	beq @PPU5

.endif ;.if(VS_SYS_ENABLED)	

@PPU0:
	lda palBrightTableL,x
	sta <PAL_SPR_PTR
	lda palBrightTableH,x	;MSB is never zero
	sta <PAL_SPR_PTR+1
	jmp @PPU_END

.if(VS_SYS_ENABLED)	
@PPU1:
	lda palBrightTableL_RP2C04_0002,x
	sta <PAL_SPR_PTR
	lda palBrightTableH_RP2C04_0002,x	;MSB is never zero
	sta <PAL_SPR_PTR+1
	jmp @PPU_END
	
@PPU2:
	lda palBrightTableL_RP2C04_0003,x
	sta <PAL_SPR_PTR
	lda palBrightTableH_RP2C04_0003,x	;MSB is never zero
	sta <PAL_SPR_PTR+1
	jmp @PPU_END
	
@PPU3:
	lda palBrightTableL_RP2C04_0004,x
	sta <PAL_SPR_PTR
	lda palBrightTableH_RP2C04_0004,x	;MSB is never zero
	sta <PAL_SPR_PTR+1
	jmp @PPU_END
	
@PPU4:
	lda palBrightTableL_2C03,x
	sta <PAL_SPR_PTR
	lda palBrightTableH_2C03,x	;MSB is never zero
	sta <PAL_SPR_PTR+1
	jmp @PPU_END
	
@PPU5:
	lda palBrightTableL_2C02,x
	sta <PAL_SPR_PTR
	lda palBrightTableH_2C02,x	;MSB is never zero
	sta <PAL_SPR_PTR+1
	jmp @PPU_END
.endif ;.if(VS_SYS_ENABLED)	
@PPU_END:
	sta <PAL_UPDATE
	rts



;void __fastcall__ pal_bg_bright(unsigned char bright);

_pal_bg_bright:

	tax

.if(VS_SYS_ENABLED)		
	lda PPU_VERSION
	beq @PPU0
	
	cmp #1
	beq @PPU1
	
	cmp #2
	beq @PPU2
	
	cmp #3
	beq @PPU3
	
	cmp #4
	beq @PPU4
	
	cmp #5
	beq @PPU5

.endif ;.if(VS_SYS_ENABLED)	

@PPU0:
	lda palBrightTableL,x
	sta <PAL_BG_PTR
	lda palBrightTableH,x	;MSB is never zero
	sta <PAL_BG_PTR+1
	jmp @PPU_END
	
.if(VS_SYS_ENABLED)	
@PPU1:
	lda palBrightTableL_RP2C04_0002,x
	sta <PAL_BG_PTR
	lda palBrightTableH_RP2C04_0002,x	;MSB is never zero
	sta <PAL_BG_PTR+1
	jmp @PPU_END
	
@PPU2:
	lda palBrightTableL_RP2C04_0003,x
	sta <PAL_BG_PTR
	lda palBrightTableH_RP2C04_0003,x	;MSB is never zero
	sta <PAL_BG_PTR+1
	jmp @PPU_END
	
@PPU3:
	lda palBrightTableL_RP2C04_0004,x
	sta <PAL_BG_PTR
	lda palBrightTableH_RP2C04_0004,x	;MSB is never zero
	sta <PAL_BG_PTR+1
	jmp @PPU_END
	
@PPU4:
	lda palBrightTableL_2C03,x
	sta <PAL_BG_PTR
	lda palBrightTableH_2C03,x	;MSB is never zero
	sta <PAL_BG_PTR+1
	jmp @PPU_END
	
@PPU5:
	lda palBrightTableL_2C02,x
	sta <PAL_BG_PTR
	lda palBrightTableH_2C02,x	;MSB is never zero
	sta <PAL_BG_PTR+1
	jmp @PPU_END
.endif ;.if(VS_SYS_ENABLED)	

@PPU_END:

	sta <PAL_UPDATE
	rts



;void __fastcall__ pal_bright(unsigned char bright);

_pal_bright:

	jsr _pal_spr_bright
	txa
	jmp _pal_bg_bright



;void __fastcall__ ppu_off(void);

_ppu_off:

	lda <PPU_MASK_VAR
	and #%11100111
	sta <PPU_MASK_VAR
	jmp _ppu_wait_nmi



;void __fastcall__ ppu_on_all(void);

_ppu_on_all:

	lda <PPU_MASK_VAR
	ora #%00011000

ppu_onoff:

	sta <PPU_MASK_VAR
	jmp _ppu_wait_nmi



;void __fastcall__ ppu_on_bg(void);

_ppu_on_bg:

	lda <PPU_MASK_VAR
	ora #%00001000
	bne ppu_onoff	;bra



;void __fastcall__ ppu_on_spr(void);

_ppu_on_spr:

	lda <PPU_MASK_VAR
	ora #%00010000
	bne ppu_onoff	;bra



;void __fastcall__ ppu_mask(unsigned char mask);

_ppu_mask:

	sta <PPU_MASK_VAR
	rts



;unsigned char __fastcall__ ppu_system(void);

_ppu_system:

	lda <NTSC_MODE
	ldx #0
	rts



;void __fastcall__ oam_clear(void);

_oam_clear:

	ldx #0
	stx SPRID ; automatically sets sprid to zero
	lda #$ff
@1:
	sta OAM_BUF,x
	inx
	inx
	inx
	inx
	bne @1
	rts
	
	
;void __fastcall__ oam_set(unsigned char index);	
;to manually set the position
;a = sprid

_oam_set:
	and #$fc ;strip those low 2 bits, just in case
	sta SPRID
	rts
	
	
;unsigned char __fastcall__ oam_get(void);	
;returns the sprid

_oam_get:
	lda SPRID
	ldx #0
	rts
	



;void __fastcall__ oam_size(unsigned char size);

_oam_size:

	asl a
	asl a
	asl a
	asl a
	asl a
	and #$20
	sta <TEMP
	lda <PPU_CTRL_VAR
	and #$df
	ora <TEMP
	sta <PPU_CTRL_VAR

	rts



;void __fastcall__ oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned char attr);
;sprid removed

_oam_spr:

	ldx SPRID
	;a = chrnum
	sta OAM_BUF+2,x

	ldy #0		;3 popa calls replacement
	lda (sp),y
	iny
	sta OAM_BUF+1,x
	lda (sp),y
	iny
	sta OAM_BUF+0,x
	lda (sp),y
	sta OAM_BUF+3,x

	lda <sp
	clc
	adc #3 ;4
	sta <sp
	bcc @1
	inc <sp+1

@1:

	txa
	clc
	adc #4
	sta SPRID
	rts



;void __fastcall__ oam_meta_spr(unsigned char x,unsigned char y,const unsigned char *data);
;sprid removed

_oam_meta_spr:

	sta <PTR
	stx <PTR+1

	ldy #1		;2 popa calls replacement, performed in reversed order
	lda (sp),y
	dey
	sta <SCRX
	lda (sp),y
	sta <SCRY
	
	ldx SPRID

@1:

	lda (PTR),y		;x offset
	cmp #$80
	beq @2
	iny
	clc
	adc <SCRX
	sta OAM_BUF+3,x
	lda (PTR),y		;y offset
	iny
	clc
	adc <SCRY
	sta OAM_BUF+0,x
	lda (PTR),y		;tile
	iny
	sta OAM_BUF+1,x
	lda (PTR),y		;attribute
	iny
	sta OAM_BUF+2,x
	inx
	inx
	inx
	inx
	jmp @1

@2:

	lda <sp
	adc #1 ;2			;carry is always set here, so it adds 3
	sta <sp
	bcc @3
	inc <sp+1

@3:

	stx SPRID
	rts



;void __fastcall__ oam_hide_rest(void);
;sprid removed

_oam_hide_rest:

	ldx SPRID
	lda #240

@1:

	sta OAM_BUF,x
	inx
	inx
	inx
	inx
	bne @1
	;x is zero
	stx SPRID
	rts



;void __fastcall__ ppu_wait_frame(void);

_ppu_wait_frame:

	lda #1
	sta <VRAM_UPDATE
	lda <FRAME_CNT1

@1:

	cmp <FRAME_CNT1
	beq @1
	lda <NTSC_MODE
	beq @3

@2:

	lda <FRAME_CNT2
	cmp #5
	beq @2

@3:

	rts



;void __fastcall__ ppu_wait_nmi(void);

_ppu_wait_nmi:

	lda #1
	sta <VRAM_UPDATE
	lda <FRAME_CNT1
@1:

	cmp <FRAME_CNT1
	beq @1
	rts



;void __fastcall__ vram_unrle(const unsigned char *data);

_vram_unrle:

	tay
	stx <RLE_HIGH
	lda #0
	sta <RLE_LOW

	lda (RLE_LOW),y
	sta <RLE_TAG
	iny
	bne @1
	inc <RLE_HIGH

@1:

	lda (RLE_LOW),y
	iny
	bne @11
	inc <RLE_HIGH

@11:

	cmp <RLE_TAG
	beq @2
	sta PPU_DATA
	sta <RLE_BYTE
	bne @1

@2:

	lda (RLE_LOW),y
	beq @4
	iny
	bne @21
	inc <RLE_HIGH

@21:

	tax
	lda <RLE_BYTE

@3:

	sta PPU_DATA
	dex
	bne @3
	beq @1

@4:

	rts



;void __fastcall__ scroll(unsigned int x,unsigned int y);

_scroll:

	sta <TEMP

	txa
	bne @1
	lda <TEMP
	cmp #240
	bcs @1
	sta <SCROLL_Y
	lda #0
	sta <TEMP
	beq @2	;bra

@1:

	sec
	lda <TEMP
	sbc #240
	sta <SCROLL_Y
	lda #2
	sta <TEMP

@2:

	jsr popax
	sta <SCROLL_X
	txa
	and #$01
	ora <TEMP
	sta <TEMP
	lda <PPU_CTRL_VAR
	and #$fc
	ora <TEMP
	sta <PPU_CTRL_VAR
	rts



;;void __fastcall__ split(unsigned int x);
;minor changes %%
_split:

;	jsr popax
	sta <SCROLL_X1
	txa
	and #$01
	sta <TEMP
	lda <PPU_CTRL_VAR
	and #$fc
	ora <TEMP
	sta <PPU_CTRL_VAR1

@3:

	bit PPU_STATUS
	bvs @3

@4:

	bit PPU_STATUS
	bvc @4

	lda <SCROLL_X1
	sta PPU_SCROLL
	lda #0
	sta PPU_SCROLL
	lda <PPU_CTRL_VAR1
	sta PPU_CTRL

	rts



;void __fastcall__ bank_spr(unsigned char n);

_bank_spr:

	and #$01
	asl a
	asl a
	asl a
	sta <TEMP
	lda <PPU_CTRL_VAR
	and #%11110111
	ora <TEMP
	sta <PPU_CTRL_VAR

	rts



;void __fastcall__ bank_bg(unsigned char n);

_bank_bg:

	and #$01
	asl a
	asl a
	asl a
	asl a
	sta <TEMP
	lda <PPU_CTRL_VAR
	and #%11101111
	ora <TEMP
	sta <PPU_CTRL_VAR

	rts



;void __fastcall__ vram_read(unsigned char *dst,unsigned int size);

_vram_read:

	sta <TEMP
	stx <TEMP+1

	jsr popax
	sta <TEMP+2
	stx <TEMP+3

	lda PPU_DATA

	ldy #0

@1:

	lda PPU_DATA
	sta (TEMP+2),y
	inc <TEMP+2
	bne @2
	inc <TEMP+3

@2:

	lda <TEMP
	bne @3
	dec <TEMP+1

@3:

	dec <TEMP
	lda <TEMP
	ora <TEMP+1
	bne @1

	rts



;void __fastcall__ vram_write(unsigned char *src,unsigned int size);

_vram_write:

	sta <TEMP
	stx <TEMP+1

	jsr popax
	sta <TEMP+2
	stx <TEMP+3

	ldy #0

@1:

	lda (TEMP+2),y
	sta PPU_DATA
	inc <TEMP+2
	bne @2
	inc <TEMP+3

@2:

	lda <TEMP
	bne @3
	dec <TEMP+1

@3:

	dec <TEMP
	lda <TEMP
	ora <TEMP+1
	bne @1

	rts



;void __fastcall__ music_play(unsigned char song);

_music_play=FamiToneMusicPlay



;void __fastcall__ music_stop(void);

_music_stop=FamiToneMusicStop



;void __fastcall__ music_pause(unsigned char pause);

_music_pause=FamiToneMusicPause



;void __fastcall__ sfx_play(unsigned char sound,unsigned char channel);

_sfx_play:

.if(FT_SFX_ENABLE)

	and #$03
	tax
	lda @sfxPriority,x
	tax
	jsr popa
	jmp FamiToneSfxPlay

@sfxPriority:

	.byte FT_SFX_CH0,FT_SFX_CH1,FT_SFX_CH2,FT_SFX_CH3
	
.else
	rts
.endif


;void __fastcall__ sample_play(unsigned char sample);

.if(FT_DPCM_ENABLE)
_sample_play=FamiToneSamplePlay
.else
_sample_play:
	rts
.endif


;unsigned char __fastcall__ pad_poll(unsigned char pad);

_pad_poll:

	tay
	ldx #3

@padPollPort:

	lda #1
	sta CTRL_PORT1
	sta <PAD_BUF-1,x
	lda #0
	sta CTRL_PORT1
	lda #8
	sta <TEMP

@padPollLoop:

	lda CTRL_PORT1,y
	lsr a
	rol <PAD_BUF-1,x
	bcc @padPollLoop

	dex
	bne @padPollPort

	lda <PAD_BUF
	cmp <PAD_BUF+1
	beq @done
	cmp <PAD_BUF+2
	beq @done
	lda <PAD_BUF+1

@done:

	sta <PAD_STATE,y
	tax
	eor <PAD_STATEP,y
	and <PAD_STATE ,y
	sta <PAD_STATET,y
	txa
	sta <PAD_STATEP,y
	
	ldx #0
	rts



;unsigned char __fastcall__ pad_trigger(unsigned char pad);

_pad_trigger:

	pha
	jsr _pad_poll
	pla
	tax
	lda <PAD_STATET,x
	ldx #0
	rts



;unsigned char __fastcall__ pad_state(unsigned char pad);

_pad_state:

	tax
	lda <PAD_STATE,x
	ldx #0
	rts



;unsigned char __fastcall__ rand8(void);
;Galois random generator, found somewhere
;out: A random number 0..255

rand1:

	lda <RAND_SEED
	asl a
	bcc @1
	eor #$cf

@1:

	sta <RAND_SEED
	rts

rand2:

	lda <RAND_SEED+1
	asl a
	bcc @1
	eor #$d7

@1:

	sta <RAND_SEED+1
	rts

_rand8:

	jsr rand1
	jsr rand2
	adc <RAND_SEED
	ldx #0
	rts



;unsigned int __fastcall__ rand16(void);

_rand16:

	jsr rand1
	tax
	jsr rand2

	rts


;void __fastcall__ set_rand(unsigned char seed);

_set_rand:

	sta <RAND_SEED
	stx <RAND_SEED+1

	rts



;void __fastcall__ set_vram_update(unsigned char *buf);

_set_vram_update:

	sta <NAME_UPD_ADR+0
	stx <NAME_UPD_ADR+1
	ora <NAME_UPD_ADR+1
	sta <NAME_UPD_ENABLE

	rts



;void __fastcall__ flush_vram_update(unsigned char *buf);

_flush_vram_update:

	sta <NAME_UPD_ADR+0
	stx <NAME_UPD_ADR+1

_flush_vram_update_nmi: ;minor changes %

	ldy #0

@updName:

	lda (NAME_UPD_ADR),y
	iny
	cmp #$40				;is it a non-sequental write?
	bcs @updNotSeq
	sta PPU_ADDR
	lda (NAME_UPD_ADR),y
	iny
	sta PPU_ADDR
	lda (NAME_UPD_ADR),y
	iny
	sta PPU_DATA
	jmp @updName

@updNotSeq:

	tax
	lda <PPU_CTRL_VAR
	cpx #$80				;is it a horizontal or vertical sequence?
	bcc @updHorzSeq
	cpx #$ff				;is it end of the update?
	beq @updDone

@updVertSeq:

	ora #$04
	bne @updNameSeq			;bra

@updHorzSeq:

	and #$fb

@updNameSeq:

	sta PPU_CTRL

	txa
	and #$3f
	sta PPU_ADDR
	lda (NAME_UPD_ADR),y
	iny
	sta PPU_ADDR
	lda (NAME_UPD_ADR),y
	iny
	tax

@updNameLoop:

	lda (NAME_UPD_ADR),y
	iny
	sta PPU_DATA
	dex
	bne @updNameLoop

	lda <PPU_CTRL_VAR
	sta PPU_CTRL

	jmp @updName

@updDone:

	rts
	
	
	
;void __fastcall__ vram_adr(unsigned int adr);

_vram_adr:

	stx PPU_ADDR
	sta PPU_ADDR

	rts



;void __fastcall__ vram_put(unsigned char n);

_vram_put:

	sta PPU_DATA

	rts



;void __fastcall__ vram_fill(unsigned char n,unsigned int len);

_vram_fill:

	sta <LEN
	stx <LEN+1
	jsr popa
	ldx <LEN+1
	beq @2
	ldx #0

@1:

	sta PPU_DATA
	dex
	bne @1
	dec <LEN+1
	bne @1

@2:

	ldx <LEN
	beq @4

@3:

	sta PPU_DATA
	dex
	bne @3

@4:

	rts



;void __fastcall__ vram_inc(unsigned char n);

_vram_inc:

	ora #0
	beq @1
	lda #$04

@1:

	sta <TEMP
	lda <PPU_CTRL_VAR
	and #$fb
	ora <TEMP
	sta <PPU_CTRL_VAR
	sta PPU_CTRL

	rts



;void __fastcall__ memcpy(void *dst,void *src,unsigned int len);

_memcpy:

	sta <LEN
	stx <LEN+1
	jsr popax
	sta <SRC
	stx <SRC+1
	jsr popax
	sta <DST
	stx <DST+1

	ldx #0

@1:

	lda <LEN+1
	beq @2
	jsr @3
	dec <LEN+1
	inc <SRC+1
	inc <DST+1
	jmp @1

@2:

	ldx <LEN
	beq @5

@3:

	ldy #0

@4:

	lda (SRC),y
	sta (DST),y
	iny
	dex
	bne @4

@5:

	rts



;void __fastcall__ memfill(void *dst,unsigned char value,unsigned int len);

_memfill:

	sta <LEN
	stx <LEN+1
	jsr popa
	sta <TEMP
	jsr popax
	sta <DST
	stx <DST+1

	ldx #0

@1:

	lda <LEN+1
	beq @2
	jsr @3
	dec <LEN+1
	inc <DST+1
	jmp @1

@2:

	ldx <LEN
	beq @5

@3:

	ldy #0
	lda <TEMP

@4:

	sta (DST),y
	iny
	dex
	bne @4

@5:

	rts



;void __fastcall__ delay(unsigned char frames);

_delay:

	tax

@1:

	jsr _ppu_wait_nmi
	dex
	bne @1

	rts



palBrightTableL:

	.byte <palBrightTable0,<palBrightTable1,<palBrightTable2
	.byte <palBrightTable3,<palBrightTable4,<palBrightTable5
	.byte <palBrightTable6,<palBrightTable7,<palBrightTable8

palBrightTableH:

	.byte >palBrightTable0,>palBrightTable1,>palBrightTable2
	.byte >palBrightTable3,>palBrightTable4,>palBrightTable5
	.byte >palBrightTable6,>palBrightTable7,>palBrightTable8

.if(VS_SYS_ENABLED)	
palBrightTableL_RP2C04_0002:

	.byte <palBrightTable0_RP2C04_0002,<palBrightTable1_RP2C04_0002,<palBrightTable2_RP2C04_0002
	.byte <palBrightTable3_RP2C04_0002,<palBrightTable4_RP2C04_0002,<palBrightTable5_RP2C04_0002
	.byte <palBrightTable6_RP2C04_0002,<palBrightTable7_RP2C04_0002,<palBrightTable8_RP2C04_0002

palBrightTableH_RP2C04_0002:

	.byte >palBrightTable0_RP2C04_0002,>palBrightTable1_RP2C04_0002,>palBrightTable2_RP2C04_0002
	.byte >palBrightTable3_RP2C04_0002,>palBrightTable4_RP2C04_0002,>palBrightTable5_RP2C04_0002
	.byte >palBrightTable6_RP2C04_0002,>palBrightTable7_RP2C04_0002,>palBrightTable8_RP2C04_0002

palBrightTableL_RP2C04_0003:

	.byte <palBrightTable0_RP2C04_0003,<palBrightTable1_RP2C04_0003,<palBrightTable2_RP2C04_0003
	.byte <palBrightTable3_RP2C04_0003,<palBrightTable4_RP2C04_0003,<palBrightTable5_RP2C04_0003
	.byte <palBrightTable6_RP2C04_0003,<palBrightTable7_RP2C04_0003,<palBrightTable8_RP2C04_0003

palBrightTableH_RP2C04_0003:

	.byte >palBrightTable0_RP2C04_0003,>palBrightTable1_RP2C04_0003,>palBrightTable2_RP2C04_0003
	.byte >palBrightTable3_RP2C04_0003,>palBrightTable4_RP2C04_0003,>palBrightTable5_RP2C04_0003
	.byte >palBrightTable6_RP2C04_0003,>palBrightTable7_RP2C04_0003,>palBrightTable8_RP2C04_0003

palBrightTableL_RP2C04_0004:

	.byte <palBrightTable0_RP2C04_0004,<palBrightTable1_RP2C04_0004,<palBrightTable2_RP2C04_0004
	.byte <palBrightTable3_RP2C04_0004,<palBrightTable4_RP2C04_0004,<palBrightTable5_RP2C04_0004
	.byte <palBrightTable6_RP2C04_0004,<palBrightTable7_RP2C04_0004,<palBrightTable8_RP2C04_0004

palBrightTableH_RP2C04_0004:

	.byte >palBrightTable0_RP2C04_0004,>palBrightTable1_RP2C04_0004,>palBrightTable2_RP2C04_0004
	.byte >palBrightTable3_RP2C04_0004,>palBrightTable4_RP2C04_0004,>palBrightTable5_RP2C04_0004
	.byte >palBrightTable6_RP2C04_0004,>palBrightTable7_RP2C04_0004,>palBrightTable8_RP2C04_0004

palBrightTableL_2C03:

	.byte <palBrightTable0_2C03,<palBrightTable1_2C03,<palBrightTable2_2C03
	.byte <palBrightTable3_2C03,<palBrightTable4_2C03,<palBrightTable5_2C03
	.byte <palBrightTable6_2C03,<palBrightTable7_2C03,<palBrightTable8_2C03

palBrightTableH_2C03:

	.byte >palBrightTable0_2C03,>palBrightTable1_2C03,>palBrightTable2_2C03
	.byte >palBrightTable3_2C03,>palBrightTable4_2C03,>palBrightTable5_2C03
	.byte >palBrightTable6_2C03,>palBrightTable7_2C03,>palBrightTable8_2C03

palBrightTableL_2C02:

	.byte <palBrightTable0_2C02,<palBrightTable1_2C02,<palBrightTable2_2C02
	.byte <palBrightTable3_2C02,<palBrightTable4_2C02,<palBrightTable5_2C02
	.byte <palBrightTable6_2C02,<palBrightTable7_2C02,<palBrightTable8_2C02

palBrightTableH_2C02:

	.byte >palBrightTable0_2C02,>palBrightTable1_2C02,>palBrightTable2_2C02
	.byte >palBrightTable3_2C02,>palBrightTable4_2C02,>palBrightTable5_2C02
	.byte >palBrightTable6_2C02,>palBrightTable7_2C02,>palBrightTable8_2C02
.endif ;.if(VS_SYS_ENABLED)	


.if(VS_SYS_ENABLED)
; Treat RP2C04_0001 as the default, for easier code compatitbility with NES.
palBrightTable0:
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30	;black
palBrightTable1:
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
palBrightTable2:
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
palBrightTable3:
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30

; Remapping (rgb: 772) to [54] (rgb: 770).
palBrightTable4:
        .byte $9,$1f,$2d,$35,$c,$b,$2b,$29,$e,$5,$33,$3a,$25,$1a,$1a,$1a
palBrightTable5:
        .byte $32,$39,$17,$2c,$27,$7,$2,$37,$3e,$31,$3a,$26,$4,$1a,$1a,$1a
palBrightTable6:
        .byte $8,$10,$3,$1,$1e,$3d,$2e,$a,$d,$13,$23,$19,$24,$1d,$1a,$1a
palBrightTable7:
        .byte $8,$21,$15,$22,$2a,$0,$16,$36,$3c,$34,$3f,$19,$14,$1c,$1a,$1a

palBrightTable8:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f	;white
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f

	;;;;;;;;;;;;;;;;;;;;;;;;;;

palBrightTable0_RP2C04_0002:
	.byte $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00	;black
palBrightTable1_RP2C04_0002:
	.byte $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
palBrightTable2_RP2C04_0002:
	.byte $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
palBrightTable3_RP2C04_0002:
	.byte $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00

; Remapping (rgb: 772) to [59] (rgb: 770).
palBrightTable4_RP2C04_0002:
        .byte $2a,$35,$27,$1f,$20,$3d,$12,$3a,$1d,$2b,$3e,$f,$34,$0,$0,$0
palBrightTable5_RP2C04_0002:
        .byte $26,$24,$2c,$9,$28,$36,$8,$33,$2,$37,$f,$14,$6,$0,$0,$0
palBrightTable6_RP2C04_0002:
        .byte $c,$11,$17,$d,$19,$5,$22,$1,$2e,$15,$32,$1b,$39,$3f,$0,$0
palBrightTable7_RP2C04_0002:
        .byte $c,$7,$1c,$23,$b,$3c,$21,$3b,$a,$3,$4,$1b,$e,$13,$0,$0

palBrightTable8_RP2C04_0002:
	.byte $0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c	;white
	.byte $0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c
	.byte $0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c
	.byte $0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c,$0c

	;;;;;;;;;;;;;;;;;;;;;;;;;;

palBrightTable0_RP2C04_0003:
	.byte $09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09	;black
palBrightTable1_RP2C04_0003:
	.byte $09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09
palBrightTable2_RP2C04_0003:
	.byte $09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09
palBrightTable3_RP2C04_0003:
	.byte $09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09,$09

; Remapping (rgb: 772) to [35] (rgb: 770).
palBrightTable4_RP2C04_0003:
        .byte $18,$8,$1b,$19,$29,$12,$1d,$34,$b,$7,$38,$4,$3f,$9,$9,$9
palBrightTable5_RP2C04_0003:
        .byte $3,$2c,$13,$3c,$0,$c,$1c,$16,$15,$14,$4,$17,$11,$9,$9,$9
palBrightTable6_RP2C04_0003:
        .byte $5,$2a,$10,$20,$2b,$1,$26,$25,$36,$28,$35,$3d,$30,$2d,$9,$9
palBrightTable7_RP2C04_0003:
        .byte $5,$6,$3a,$33,$1e,$1f,$a,$23,$3b,$32,$2,$3d,$f,$d,$9,$9

palBrightTable8_RP2C04_0003:
	.byte $05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05	;white
	.byte $05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05
	.byte $05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05
	.byte $05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05,$05	

	;;;;;;;;;;;;;;;;;;;;;;;;;;;

palBrightTable0_RP2C04_0004:
	.byte $04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04	;black
palBrightTable1_RP2C04_0004:
	.byte $04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04
palBrightTable2_RP2C04_0004:
	.byte $04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04
palBrightTable3_RP2C04_0004:
	.byte $04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04,$04

; Remapping (rgb: 772) to [13] (rgb: 770).
palBrightTable4_RP2C04_0004:
        .byte $26,$6,$23,$1,$1c,$20,$16,$22,$32,$3f,$21,$e,$28,$4,$4,$4
palBrightTable5_RP2C04_0004:
        .byte $8,$2a,$12,$24,$25,$27,$33,$7,$0,$1b,$e,$19,$2,$4,$4,$4
palBrightTable6_RP2C04_0004:
        .byte $31,$1f,$1a,$3a,$2f,$10,$17,$39,$3,$3c,$a,$35,$3e,$2e,$4,$4
palBrightTable7_RP2C04_0004:
        .byte $31,$3b,$3d,$30,$13,$5,$c,$d,$2d,$f,$1e,$35,$37,$29,$4,$4

palBrightTable8_RP2C04_0004:
	.byte $31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31	;white
	.byte $31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31
	.byte $31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31
	.byte $31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31,$31	


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;NOTE: 	This is basically indenticle to the NES except that 2d and 3d are black on this PPU (instead of grey).
;		You will find that some of the mappings are different too, but that is arbitrary. The alorithm to generate
;		this data just happen to choose (for instance) a different identicle white.
palBrightTable0_2C03:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f	;black
palBrightTable1_2C03:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable2_2C03:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable3_2C03:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f

; Remapping (rgb: 444) to [0] (rgb: 333).
; Remapping (rgb: 666) to [49] (rgb: 567).
palBrightTable4_2C03:
        .byte $0,$1,$2,$3,$4,$5,$6,$7,$8,$9,$a,$b,$c,$d,$d,$d
palBrightTable5_2C03:
        .byte $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$b,$1b,$1c,$d,$d,$d
palBrightTable6_2C03:
        .byte $20,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2a,$2b,$2c,$0,$d,$d
palBrightTable7_2C03:
        .byte $20,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3a,$2b,$3c,$31,$d,$d

palBrightTable8_2C03:
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30	;white
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; 1:1 NES PPU.
palBrightTable0_2C02:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f	;black
palBrightTable1_2C02:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable2_2C02:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable3_2C02:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable4_2C02:
	.byte $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0a,$0b,$0c,$0f,$0f,$0f	;normal colors
palBrightTable5_2C02:
	.byte $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$1a,$1b,$1c,$00,$00,$00
palBrightTable6_2C02:
	.byte $10,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2a,$2b,$2c,$10,$10,$10	;$10 because $20 is the same as $30
palBrightTable7_2C02:
	.byte $30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3a,$3b,$3c,$20,$20,$20
palBrightTable8_2C02:
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30	;white
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30	
	
.else
palBrightTable0:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f	;black
palBrightTable1:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable2:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable3:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable4:
	.byte $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0a,$0b,$0c,$0f,$0f,$0f	;normal colors
palBrightTable5:
	.byte $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$1a,$1b,$1c,$00,$00,$00
palBrightTable6:
	.byte $10,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2a,$2b,$2c,$10,$10,$10	;$10 because $20 is the same as $30
palBrightTable7:
	.byte $30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3a,$3b,$3c,$20,$20,$20
palBrightTable8:
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30	;white
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
.endif ;VS_SYS_ENABLED
;;	.include "famitone2.s"
; included in crt0.s
