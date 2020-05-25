;
; File generated by cc65 v 2.18 - Git f8be35b
;
	.fopt		compiler,"cc65 v 2.18 - Git f8be35b"
	.setcpu		"6502"
	.smart		on
	.autoimport	on
	.case		on
	.debuginfo	off
	.importzp	sp, sreg, regsave, regbank
	.importzp	tmp1, tmp2, tmp3, tmp4, ptr1, ptr2, ptr3, ptr4
	.macpack	longbranch
	.forceimport	__STARTUP__
	.import		_pal_bg
	.import		_pal_spr
	.import		_ppu_wait_nmi
	.import		_ppu_off
	.import		_ppu_on_all
	.import		_oam_clear
	.import		_oam_spr
	.import		_pad_poll
	.import		_scroll
	.import		_bank_spr
	.import		_rand8
	.import		_vram_adr
	.import		_vram_fill
	.import		_vram_write
	.import		_vram_unrle
	.import		_memfill
	.import		_set_vram_buffer
	.import		_one_vram_buffer
	.import		_clear_vram_buffer
	.import		_get_pad_new
	.import		_get_ppu_addr
	.import		_seed_rng
	.export		_RoundSprL
	.export		_RoundSprR
	.export		_game_area
	.export		_tick_count
	.export		_pad1
	.export		_pad1_new
	.export		_text
	.export		_state
	.export		_cur_block
	.export		_fall_rate
	.export		_def_z_clust
	.export		_def_z_rev_clust
	.export		_def_line_clust
	.export		_def_box_clust
	.export		_def_tee_clust
	.export		_def_L_clust
	.export		_def_L_rev_clust
	.export		_cluster_defs
	.export		_cur_rot
	.export		_cur_cluster
	.export		_next_cluster
	.export		_cluster_sprites
	.export		_do_line_check
	.export		_line_crush_y
	.export		_game_board
	.export		_empty_row
	.export		_full_row
	.export		_palette_bg
	.export		_palette_sp
	.export		_draw_sprites
	.export		_movement
	.export		_set_block
	.export		_clear_block
	.export		_put_cur_cluster
	.export		_get_block
	.export		_is_block_free
	.export		_is_cluster_colliding
	.export		_spawn_new_cluster
	.export		_rotate_cur_cluster
	.export		_debug_fill_nametables
	.export		_debug_draw_board_area
	.export		_main

.segment	"DATA"

_state:
	.byte	$00
_cur_block:
	.byte	$00
	.byte	$00
_fall_rate:
	.byte	$3C
_cluster_defs:
	.addr	_def_z_clust
	.addr	_def_z_rev_clust
	.addr	_def_line_clust
	.addr	_def_box_clust
	.addr	_def_tee_clust
	.addr	_def_L_clust
	.addr	_def_L_rev_clust
_cluster_sprites:
	.byte	$10
	.byte	$11
	.byte	$12
	.byte	$13
	.byte	$14
	.byte	$15
	.byte	$16
_empty_row:
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$00
_full_row:
	.byte	$01
	.byte	$01
	.byte	$01
	.byte	$01
	.byte	$01
	.byte	$01
	.byte	$01
	.byte	$01
	.byte	$01
	.byte	$01

.segment	"RODATA"

_RoundSprL:
	.byte	$FF
	.byte	$FF
	.byte	$02
	.byte	$00
	.byte	$07
	.byte	$FF
	.byte	$03
	.byte	$00
	.byte	$FF
	.byte	$07
	.byte	$12
	.byte	$00
	.byte	$07
	.byte	$07
	.byte	$13
	.byte	$00
	.byte	$80
_RoundSprR:
	.byte	$FF
	.byte	$FF
	.byte	$00
	.byte	$00
	.byte	$07
	.byte	$FF
	.byte	$01
	.byte	$00
	.byte	$FF
	.byte	$07
	.byte	$10
	.byte	$00
	.byte	$07
	.byte	$07
	.byte	$11
	.byte	$00
	.byte	$80
_game_area:
	.byte	$01
	.byte	$00
	.byte	$01
	.byte	$16
	.byte	$B2
	.byte	$E7
	.byte	$00
	.byte	$01
	.byte	$02
	.byte	$A4
	.byte	$A5
	.byte	$A6
	.byte	$00
	.byte	$01
	.byte	$08
	.byte	$E4
	.byte	$E7
	.byte	$00
	.byte	$01
	.byte	$0B
	.byte	$E4
	.byte	$C9
	.byte	$F7
	.byte	$00
	.byte	$B2
	.byte	$B3
	.byte	$B4
	.byte	$B5
	.byte	$B6
	.byte	$B7
	.byte	$00
	.byte	$A4
	.byte	$A5
	.byte	$A6
	.byte	$00
	.byte	$01
	.byte	$03
	.byte	$87
	.byte	$89
	.byte	$88
	.byte	$8B
	.byte	$A5
	.byte	$00
	.byte	$01
	.byte	$07
	.byte	$C0
	.byte	$87
	.byte	$8A
	.byte	$88
	.byte	$8B
	.byte	$C2
	.byte	$C3
	.byte	$C4
	.byte	$C5
	.byte	$C6
	.byte	$C7
	.byte	$B3
	.byte	$B4
	.byte	$B5
	.byte	$B6
	.byte	$B7
	.byte	$00
	.byte	$01
	.byte	$02
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$80
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$D2
	.byte	$D3
	.byte	$D4
	.byte	$D5
	.byte	$D6
	.byte	$D7
	.byte	$C3
	.byte	$C4
	.byte	$C5
	.byte	$C6
	.byte	$C7
	.byte	$00
	.byte	$00
	.byte	$E4
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$99
	.byte	$98
	.byte	$9B
	.byte	$E6
	.byte	$E7
	.byte	$00
	.byte	$01
	.byte	$03
	.byte	$D3
	.byte	$D4
	.byte	$D5
	.byte	$D6
	.byte	$D7
	.byte	$00
	.byte	$00
	.byte	$F4
	.byte	$97
	.byte	$A8
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$B1
	.byte	$B1
	.byte	$E6
	.byte	$E7
	.byte	$00
	.byte	$01
	.byte	$08
	.byte	$D8
	.byte	$97
	.byte	$A9
	.byte	$AA
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$A7
	.byte	$A8
	.byte	$98
	.byte	$9B
	.byte	$F5
	.byte	$B1
	.byte	$E8
	.byte	$F7
	.byte	$00
	.byte	$01
	.byte	$08
	.byte	$D0
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$C9
	.byte	$E8
	.byte	$F6
	.byte	$F7
	.byte	$00
	.byte	$01
	.byte	$07
	.byte	$A4
	.byte	$A5
	.byte	$A7
	.byte	$A8
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$D9
	.byte	$F8
	.byte	$F9
	.byte	$A3
	.byte	$00
	.byte	$01
	.byte	$05
	.byte	$B2
	.byte	$B3
	.byte	$B4
	.byte	$B4
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$A9
	.byte	$98
	.byte	$9B
	.byte	$A2
	.byte	$A2
	.byte	$EA
	.byte	$FA
	.byte	$00
	.byte	$01
	.byte	$04
	.byte	$E4
	.byte	$F5
	.byte	$B4
	.byte	$E8
	.byte	$F6
	.byte	$97
	.byte	$99
	.byte	$98
	.byte	$BB
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$9A
	.byte	$9B
	.byte	$E2
	.byte	$E3
	.byte	$00
	.byte	$01
	.byte	$06
	.byte	$F4
	.byte	$C9
	.byte	$F6
	.byte	$F6
	.byte	$B4
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$F2
	.byte	$F3
	.byte	$00
	.byte	$01
	.byte	$06
	.byte	$D0
	.byte	$C2
	.byte	$C3
	.byte	$C4
	.byte	$C5
	.byte	$A7
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$A7
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$E2
	.byte	$E3
	.byte	$00
	.byte	$01
	.byte	$07
	.byte	$EA
	.byte	$D1
	.byte	$D3
	.byte	$D3
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$B8
	.byte	$98
	.byte	$9B
	.byte	$E2
	.byte	$F3
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$E0
	.byte	$E1
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$83
	.byte	$84
	.byte	$85
	.byte	$86
	.byte	$00
	.byte	$01
	.byte	$07
	.byte	$F0
	.byte	$F1
	.byte	$97
	.byte	$B8
	.byte	$98
	.byte	$BB
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$93
	.byte	$94
	.byte	$95
	.byte	$96
	.byte	$00
	.byte	$01
	.byte	$07
	.byte	$F2
	.byte	$E3
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$B9
	.byte	$BA
	.byte	$9B
	.byte	$87
	.byte	$8B
	.byte	$97
	.byte	$FA
	.byte	$9D
	.byte	$00
	.byte	$C7
	.byte	$00
	.byte	$00
	.byte	$A4
	.byte	$A5
	.byte	$00
	.byte	$E2
	.byte	$E3
	.byte	$A7
	.byte	$98
	.byte	$99
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$FA
	.byte	$00
	.byte	$01
	.byte	$02
	.byte	$AD
	.byte	$00
	.byte	$80
	.byte	$01
	.byte	$02
	.byte	$90
	.byte	$82
	.byte	$84
	.byte	$85
	.byte	$84
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$02
	.byte	$BC
	.byte	$BD
	.byte	$BE
	.byte	$00
	.byte	$01
	.byte	$02
	.byte	$A1
	.byte	$92
	.byte	$94
	.byte	$95
	.byte	$94
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$02
	.byte	$CC
	.byte	$CD
	.byte	$CE
	.byte	$00
	.byte	$01
	.byte	$03
	.byte	$97
	.byte	$9A
	.byte	$98
	.byte	$99
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$BB
	.byte	$00
	.byte	$00
	.byte	$DB
	.byte	$DC
	.byte	$DD
	.byte	$DE
	.byte	$9E
	.byte	$9F
	.byte	$00
	.byte	$00
	.byte	$A7
	.byte	$98
	.byte	$98
	.byte	$99
	.byte	$97
	.byte	$9A
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$00
	.byte	$EB
	.byte	$EC
	.byte	$ED
	.byte	$EE
	.byte	$AE
	.byte	$AF
	.byte	$8C
	.byte	$8C
	.byte	$97
	.byte	$A8
	.byte	$98
	.byte	$B9
	.byte	$97
	.byte	$B8
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$8C
	.byte	$BF
	.byte	$FB
	.byte	$FC
	.byte	$FD
	.byte	$FE
	.byte	$8C
	.byte	$01
	.byte	$03
	.byte	$97
	.byte	$98
	.byte	$01
	.byte	$02
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$00
	.byte	$01
	.byte	$09
	.byte	$A7
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$CF
	.byte	$8C
	.byte	$8C
	.byte	$CF
	.byte	$8C
	.byte	$8C
	.byte	$8E
	.byte	$01
	.byte	$03
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$A9
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$AB
	.byte	$82
	.byte	$83
	.byte	$84
	.byte	$85
	.byte	$84
	.byte	$85
	.byte	$83
	.byte	$84
	.byte	$85
	.byte	$86
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$8E
	.byte	$8F
	.byte	$8C
	.byte	$01
	.byte	$07
	.byte	$97
	.byte	$B8
	.byte	$98
	.byte	$98
	.byte	$97
	.byte	$98
	.byte	$9A
	.byte	$9B
	.byte	$92
	.byte	$93
	.byte	$94
	.byte	$95
	.byte	$94
	.byte	$95
	.byte	$93
	.byte	$94
	.byte	$95
	.byte	$96
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$8C
	.byte	$01
	.byte	$09
	.byte	$8D
	.byte	$8E
	.byte	$01
	.byte	$02
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$8D
	.byte	$8E
	.byte	$01
	.byte	$07
	.byte	$8F
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$8C
	.byte	$01
	.byte	$06
	.byte	$BF
	.byte	$8C
	.byte	$01
	.byte	$05
	.byte	$97
	.byte	$98
	.byte	$98
	.byte	$8C
	.byte	$01
	.byte	$0B
	.byte	$98
	.byte	$98
	.byte	$9B
	.byte	$8C
	.byte	$01
	.byte	$04
	.byte	$AE
	.byte	$8C
	.byte	$01
	.byte	$07
	.byte	$8D
	.byte	$8E
	.byte	$8E
	.byte	$8F
	.byte	$8C
	.byte	$01
	.byte	$09
	.byte	$8D
	.byte	$8E
	.byte	$8E
	.byte	$8F
	.byte	$8C
	.byte	$01
	.byte	$03
	.byte	$BF
	.byte	$8C
	.byte	$01
	.byte	$20
	.byte	$55
	.byte	$55
	.byte	$A7
	.byte	$F5
	.byte	$F5
	.byte	$BD
	.byte	$67
	.byte	$55
	.byte	$55
	.byte	$DD
	.byte	$AA
	.byte	$00
	.byte	$00
	.byte	$88
	.byte	$EE
	.byte	$77
	.byte	$D5
	.byte	$FF
	.byte	$AA
	.byte	$00
	.byte	$00
	.byte	$88
	.byte	$EE
	.byte	$57
	.byte	$5D
	.byte	$DF
	.byte	$AA
	.byte	$00
	.byte	$00
	.byte	$88
	.byte	$EE
	.byte	$75
	.byte	$FF
	.byte	$FF
	.byte	$AA
	.byte	$00
	.byte	$00
	.byte	$88
	.byte	$AA
	.byte	$56
	.byte	$51
	.byte	$AA
	.byte	$AA
	.byte	$00
	.byte	$00
	.byte	$88
	.byte	$66
	.byte	$55
	.byte	$55
	.byte	$5A
	.byte	$AA
	.byte	$5F
	.byte	$5F
	.byte	$9B
	.byte	$66
	.byte	$55
	.byte	$05
	.byte	$01
	.byte	$06
	.byte	$05
	.byte	$01
	.byte	$00
_text:
	.byte	$2D,$20,$50,$52,$45,$53,$53,$20,$53,$54,$41,$52,$54,$20,$2D,$00
_def_z_clust:
	.word	$0C60
	.word	$0264
	.word	$0C60
	.word	$0264
_def_z_rev_clust:
	.word	$06C0
	.word	$8C40
	.word	$06C0
	.word	$8C40
_def_line_clust:
	.word	$00F0
	.word	$4444
	.word	$00F0
	.word	$4444
_def_box_clust:
	.word	$0660
	.word	$0660
	.word	$0660
	.word	$0660
_def_tee_clust:
	.word	$4E00
	.word	$4640
	.word	$0E40
	.word	$4C40
_def_L_clust:
	.word	$0E80
	.word	$C440
	.word	$2E00
	.word	$4460
_def_L_rev_clust:
	.word	$0E20
	.word	$44C0
	.word	$8E00
	.word	$6440
_palette_bg:
	.byte	$3C
	.byte	$01
	.byte	$21
	.byte	$30
	.byte	$3C
	.byte	$22
	.byte	$01
	.byte	$30
	.byte	$3C
	.byte	$0F
	.byte	$1D
	.byte	$22
	.byte	$3C
	.byte	$0F
	.byte	$26
	.byte	$29
_palette_sp:
	.byte	$3C
	.byte	$01
	.byte	$21
	.byte	$30
	.byte	$0F
	.byte	$09
	.byte	$19
	.byte	$29
	.byte	$0F
	.byte	$07
	.byte	$28
	.byte	$38
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$00

.segment	"BSS"

.segment	"ZEROPAGE"
_tick_count:
	.res	1,$00
_pad1:
	.res	1,$00
_pad1_new:
	.res	1,$00
_cur_rot:
	.res	1,$00
_cur_cluster:
	.res	5,$00
_next_cluster:
	.res	5,$00
_do_line_check:
	.res	1,$00
_line_crush_y:
	.res	1,$00
.segment	"BSS"
_game_board:
	.res	200,$00

; ---------------------------------------------------------------
; void __near__ draw_sprites (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_draw_sprites: near

.segment	"CODE"

;
; oam_clear();
;
	jsr     decsp4
	jsr     _oam_clear
;
; start_x = (cur_block.x << 3) + BOARD_START_X_PX;
;
	lda     _cur_block
	asl     a
	asl     a
	asl     a
	clc
	adc     #$60
	ldy     #$03
	sta     (sp),y
;
; start_y = (cur_block.y << 3) + BOARD_START_Y_PX;
;
	lda     _cur_block+1
	asl     a
	asl     a
	asl     a
	clc
	adc     #$20
	dey
	sta     (sp),y
;
; for (iy = 0; iy < 4; ++iy)
;
	lda     #$00
	tay
L0545:	sta     (sp),y
	cmp     #$04
	bcs     L0548
;
; for (ix = 0; ix < 4; ++ix)
;
	tya
	iny
L0544:	sta     (sp),y
	cmp     #$04
	bcs     L03FD
;
; unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4
;
	dey
	lda     (sp),y
	asl     a
	asl     a
	sta     ptr1
	iny
	lda     (sp),y
	and     #$03
	clc
	adc     ptr1
	jsr     pusha
;
; if (cur_cluster.layout & (0x8000 >> bit))
;
	ldy     #$00
	lda     (sp),y
	tay
	lda     #$00
	ldx     #$80
	jsr     shraxy
	and     _cur_cluster
	pha
	txa
	and     _cur_cluster+1
	sta     tmp1
	pla
	ora     tmp1
	beq     L040F
;
; oam_spr(start_x + (ix << 3), start_y + (iy << 3), cur_cluster.sprite, 0);
;
	jsr     decsp3
	ldy     #$05
	lda     (sp),y
	asl     a
	asl     a
	asl     a
	clc
	ldy     #$07
	adc     (sp),y
	ldy     #$02
	sta     (sp),y
	ldy     #$04
	lda     (sp),y
	asl     a
	asl     a
	asl     a
	clc
	ldy     #$06
	adc     (sp),y
	ldy     #$01
	sta     (sp),y
	lda     _cur_cluster+4
	dey
	sta     (sp),y
	tya
	jsr     _oam_spr
;
; }
;
L040F:	jsr     incsp1
;
; for (ix = 0; ix < 4; ++ix)
;
	ldy     #$01
	clc
	tya
	adc     (sp),y
	jmp     L0544
;
; for (iy = 0; iy < 4; ++iy)
;
L03FD:	dey
	clc
	lda     #$01
	adc     (sp),y
	jmp     L0545
;
; start_x = 15 << 3;
;
L0548:	lda     #$78
	ldy     #$03
	sta     (sp),y
;
; start_y = 0 << 3;
;
	lda     #$00
	dey
	sta     (sp),y
;
; for (iy = 0; iy < 4; ++iy)
;
	tay
L0547:	sta     (sp),y
	cmp     #$04
	bcs     L041E
;
; for (ix = 0; ix < 4; ++ix)
;
	tya
	iny
L0546:	sta     (sp),y
	cmp     #$04
	bcs     L041F
;
; unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4
;
	dey
	lda     (sp),y
	asl     a
	asl     a
	sta     ptr1
	iny
	lda     (sp),y
	and     #$03
	clc
	adc     ptr1
	jsr     pusha
;
; if (next_cluster.layout & (0x8000 >> bit))
;
	ldy     #$00
	lda     (sp),y
	tay
	lda     #$00
	ldx     #$80
	jsr     shraxy
	and     _next_cluster
	pha
	txa
	and     _next_cluster+1
	sta     tmp1
	pla
	ora     tmp1
	beq     L0431
;
; oam_spr(start_x + (ix << 3), start_y + (iy << 3), next_cluster.sprite, 0);
;
	jsr     decsp3
	ldy     #$05
	lda     (sp),y
	asl     a
	asl     a
	asl     a
	clc
	ldy     #$07
	adc     (sp),y
	ldy     #$02
	sta     (sp),y
	ldy     #$04
	lda     (sp),y
	asl     a
	asl     a
	asl     a
	clc
	ldy     #$06
	adc     (sp),y
	ldy     #$01
	sta     (sp),y
	lda     _next_cluster+4
	dey
	sta     (sp),y
	tya
	jsr     _oam_spr
;
; }
;
L0431:	jsr     incsp1
;
; for (ix = 0; ix < 4; ++ix)
;
	ldy     #$01
	clc
	tya
	adc     (sp),y
	jmp     L0546
;
; for (iy = 0; iy < 4; ++iy)
;
L041F:	dey
	clc
	lda     #$01
	adc     (sp),y
	jmp     L0547
;
; }
;
L041E:	jmp     incsp4

.endproc

; ---------------------------------------------------------------
; void __near__ movement (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_movement: near

.segment	"CODE"

;
; if (pad1_new & PAD_SELECT)
;
	jsr     decsp3
	lda     _pad1_new
	and     #$20
	beq     L054E
;
; spawn_new_cluster();
;
	jsr     _spawn_new_cluster
;
; if (pad1_new & PAD_A)
;
L054E:	lda     _pad1_new
	and     #$80
	beq     L054F
;
; rotate_cur_cluster(1);
;
	lda     #$01
;
; else if (pad1_new & PAD_B)
;
	jmp     L054B
L054F:	lda     _pad1_new
	and     #$40
	beq     L0550
;
; rotate_cur_cluster(-1);
;
	lda     #$FF
L054B:	jsr     _rotate_cur_cluster
;
; if (/*(pad1 & PAD_RIGHT && (tick_count % 4 == 0)) ||*/ pad1_new & PAD_RIGHT)
;
L0550:	lda     _pad1_new
	and     #$01
	beq     L0551
;
; old_x = cur_block.x;
;
	lda     _cur_block
	ldy     #$00
	sta     (sp),y
;
; cur_block.x += 1;
;
	inc     _cur_block
;
; else if (/*(pad1 & PAD_LEFT && (tick_count % 4 == 0)) ||*/ pad1_new & PAD_LEFT)
;
	jmp     L044F
L0551:	lda     _pad1_new
	and     #$02
	beq     L044F
;
; old_x = cur_block.x;
;
	lda     _cur_block
	ldy     #$00
	sta     (sp),y
;
; cur_block.x -= 1; // note: wrap around
;
	dec     _cur_block
;
; if (is_cluster_colliding())
;
L044F:	jsr     _is_cluster_colliding
	tax
	beq     L0552
;
; cur_block.x = old_x;
;
	ldy     #$00
	lda     (sp),y
	sta     _cur_block
;
; temp_fall_rate = fall_rate;
;
L0552:	lda     _fall_rate
	ldy     #$01
	sta     (sp),y
;
; if (pad1_new & PAD_DOWN || pad1 & PAD_UP)
;
	lda     _pad1_new
	and     #$04
	bne     L0553
	lda     _pad1
	and     #$08
	beq     L0554
;
; temp_fall_rate = tick_count;
;
L0553:	lda     _tick_count
;
; else if (pad1 & PAD_DOWN)
;
	jmp     L054D
L0554:	lda     _pad1
	and     #$04
	beq     L0555
;
; temp_fall_rate >>= 4;
;
	lda     (sp),y
	lsr     a
	lsr     a
	lsr     a
	lsr     a
L054D:	sta     (sp),y
;
; if (tick_count % temp_fall_rate == 0)
;
L0555:	lda     _tick_count
	jsr     pusha0
	ldy     #$03
	lda     (sp),y
	jsr     tosumoda0
	cpx     #$00
	bne     L0465
	cmp     #$00
	bne     L0465
;
; cur_block.y += 1;
;
	inc     _cur_block+1
;
; hit = 0;
;
L0465:	lda     #$00
	ldy     #$02
	sta     (sp),y
;
; if (is_cluster_colliding())
;
	jsr     _is_cluster_colliding
	tax
	beq     L046C
;
; cur_block.y -= 1;
;
	dec     _cur_block+1
;
; hit = 1;
;
	lda     #$01
	ldy     #$02
	sta     (sp),y
;
; if (hit)
;
L046C:	ldy     #$02
	lda     (sp),y
	beq     L0472
;
; put_cur_cluster();
;
	jsr     _put_cur_cluster
;
; spawn_new_cluster();
;
	jsr     _spawn_new_cluster
;
; }
;
L0472:	jmp     incsp3

.endproc

; ---------------------------------------------------------------
; void __near__ set_block (unsigned char, unsigned char, unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_set_block: near

.segment	"CODE"

;
; {
;
	jsr     pusha
;
; address = get_ppu_addr(0, (x << 3) + BOARD_START_X_PX, (y << 3) + BOARD_START_Y_PX);
;
	jsr     decsp4
	lda     #$00
	ldy     #$01
	sta     (sp),y
	ldy     #$06
	lda     (sp),y
	asl     a
	asl     a
	asl     a
	clc
	adc     #$60
	ldy     #$00
	sta     (sp),y
	ldy     #$05
	lda     (sp),y
	asl     a
	asl     a
	asl     a
	clc
	adc     #$20
	jsr     _get_ppu_addr
	jsr     stax0sp
;
; one_vram_buffer(id, address);
;
	ldy     #$02
	lda     (sp),y
	jsr     pusha
	ldy     #$02
	lda     (sp),y
	tax
	dey
	lda     (sp),y
	jsr     _one_vram_buffer
;
; game_board[PIXEL_TO_BOARD_INDEX(x,y)] = id;
;
	ldy     #$03
	ldx     #$00
	lda     (sp),y
	jsr     mulax10
	sta     ptr1
	stx     ptr1+1
	iny
	lda     (sp),y
	clc
	adc     ptr1
	ldx     ptr1+1
	bcc     L0557
	inx
	clc
L0557:	adc     #<(_game_board)
	sta     ptr1
	txa
	adc     #>(_game_board)
	sta     ptr1+1
	ldy     #$02
	lda     (sp),y
	ldy     #$00
	sta     (ptr1),y
;
; }
;
	jmp     incsp5

.endproc

; ---------------------------------------------------------------
; void __near__ clear_block (unsigned char, unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_clear_block: near

.segment	"CODE"

;
; {
;
	jsr     pusha
;
; set_block(x, y, 0);
;
	jsr     decsp2
	ldy     #$03
	lda     (sp),y
	ldy     #$01
	sta     (sp),y
	iny
	lda     (sp),y
	ldy     #$00
	sta     (sp),y
	tya
	jsr     _set_block
;
; }
;
	jmp     incsp2

.endproc

; ---------------------------------------------------------------
; void __near__ put_cur_cluster (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_put_cur_cluster: near

.segment	"CODE"

;
; do_line_check = 1;
;
	jsr     decsp2
	lda     #$01
	sta     _do_line_check
;
; for (iy = 0; iy < 4; ++iy)
;
	lda     #$00
	tay
L055B:	sta     (sp),y
	cmp     #$04
	bcs     L0492
;
; for (ix = 0; ix < 4; ++ix)
;
	tya
	iny
L055A:	sta     (sp),y
	cmp     #$04
	bcs     L0493
;
; unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4
;
	dey
	lda     (sp),y
	asl     a
	asl     a
	sta     ptr1
	iny
	lda     (sp),y
	and     #$03
	clc
	adc     ptr1
	jsr     pusha
;
; if (cur_cluster.layout & (0x8000 >> bit))
;
	ldy     #$00
	lda     (sp),y
	tay
	lda     #$00
	ldx     #$80
	jsr     shraxy
	and     _cur_cluster
	pha
	txa
	and     _cur_cluster+1
	sta     tmp1
	pla
	ora     tmp1
	beq     L04A5
;
; set_block(cur_block.x + ix, cur_block.y + iy, cur_cluster.sprite);
;
	jsr     decsp2
	ldy     #$04
	lda     (sp),y
	clc
	adc     _cur_block
	ldy     #$01
	sta     (sp),y
	ldy     #$03
	lda     (sp),y
	clc
	adc     _cur_block+1
	ldy     #$00
	sta     (sp),y
	lda     _cur_cluster+4
	jsr     _set_block
;
; }
;
L04A5:	jsr     incsp1
;
; for (ix = 0; ix < 4; ++ix)
;
	ldy     #$01
	clc
	tya
	adc     (sp),y
	jmp     L055A
;
; for (iy = 0; iy < 4; ++iy)
;
L0493:	dey
	clc
	lda     #$01
	adc     (sp),y
	jmp     L055B
;
; }
;
L0492:	jmp     incsp2

.endproc

; ---------------------------------------------------------------
; unsigned char __near__ get_block (unsigned char, unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_get_block: near

.segment	"CODE"

;
; {
;
	jsr     pusha
;
; return game_board[PIXEL_TO_BOARD_INDEX(x,y)];
;
	ldx     #$00
	lda     (sp,x)
	jsr     mulax10
	sta     ptr1
	stx     ptr1+1
	ldy     #$01
	lda     (sp),y
	clc
	adc     ptr1
	ldx     ptr1+1
	bcc     L055D
	inx
L055D:	sta     ptr1
	txa
	clc
	adc     #>(_game_board)
	sta     ptr1+1
	ldy     #<(_game_board)
	ldx     #$00
	lda     (ptr1),y
;
; }
;
	jmp     incsp2

.endproc

; ---------------------------------------------------------------
; unsigned char __near__ is_block_free (unsigned char, unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_is_block_free: near

.segment	"CODE"

;
; {
;
	jsr     pusha
;
; if (y > BOARD_END_Y_PX_BOARD || x > BOARD_END_X_PX_BOARD)
;
	ldy     #$00
	lda     (sp),y
	cmp     #$14
	bcs     L055E
	iny
	lda     (sp),y
	cmp     #$0A
	bcc     L055F
;
; return 0;
;
L055E:	ldx     #$00
	txa
	jmp     incsp2
;
; return get_block(x, y) == 0;
;
L055F:	lda     (sp),y
	jsr     pusha
	ldy     #$01
	lda     (sp),y
	jsr     _get_block
	cmp     #$00
	jsr     booleq
;
; }
;
	jmp     incsp2

.endproc

; ---------------------------------------------------------------
; unsigned char __near__ is_cluster_colliding (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_is_cluster_colliding: near

.segment	"CODE"

;
; for (iy = 0; iy < 4; ++iy)
;
	jsr     decsp2
	lda     #$00
	tay
L0563:	sta     (sp),y
	ldx     #$00
	lda     (sp),y
	cmp     #$04
	bcs     L0564
;
; for (ix = 0; ix < 4; ++ix)
;
	txa
	iny
L0562:	sta     (sp),y
	cmp     #$04
	bcs     L04BD
;
; unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4
;
	dey
	lda     (sp),y
	asl     a
	asl     a
	sta     ptr1
	iny
	lda     (sp),y
	and     #$03
	clc
	adc     ptr1
	jsr     pusha
;
; if (cur_cluster.layout & (0x8000 >> bit))
;
	ldy     #$00
	lda     (sp),y
	tay
	lda     #$00
	ldx     #$80
	jsr     shraxy
	and     _cur_cluster
	pha
	txa
	and     _cur_cluster+1
	sta     tmp1
	pla
	ora     tmp1
	beq     L04D2
;
; if (!is_block_free(cur_block.x + ix, cur_block.y + iy))
;
	ldy     #$02
	lda     (sp),y
	clc
	adc     _cur_block
	jsr     pusha
	ldy     #$02
	lda     (sp),y
	clc
	adc     _cur_block+1
	jsr     _is_block_free
	tax
	bne     L04D2
;
; return 1;
;
	lda     #$01
	jsr     incsp1
	jmp     incsp2
;
; }
;
L04D2:	jsr     incsp1
;
; for (ix = 0; ix < 4; ++ix)
;
	ldy     #$01
	clc
	tya
	adc     (sp),y
	jmp     L0562
;
; for (iy = 0; iy < 4; ++iy)
;
L04BD:	dey
	clc
	lda     #$01
	adc     (sp),y
	jmp     L0563
;
; return 0;
;
L0564:	txa
;
; }
;
	jmp     incsp2

.endproc

; ---------------------------------------------------------------
; void __near__ spawn_new_cluster (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_spawn_new_cluster: near

.segment	"CODE"

;
; cur_block.x = 3; //(BOARD_END_Y_PX_BOARD >> 1);
;
	jsr     decsp1
	lda     #$03
	sta     _cur_block
;
; cur_block.y = 0;
;
	lda     #$00
	sta     _cur_block+1
;
; cur_rot = 0;
;
	sta     _cur_rot
;
; cur_cluster.def = next_cluster.def;
;
	lda     _next_cluster+2
	ldx     _next_cluster+2+1
	sta     _cur_cluster+2
	stx     _cur_cluster+2+1
;
; cur_cluster.layout = cur_cluster.def[0];
;
	sta     ptr1
	stx     ptr1+1
	ldy     #$01
	lda     (ptr1),y
	sta     _cur_cluster+1
	dey
	lda     (ptr1),y
	sta     _cur_cluster
;
; cur_cluster.sprite = next_cluster.sprite;
;
	lda     _next_cluster+4
	sta     _cur_cluster+4
;
; id = rand8() % NUM_CLUSTERS;
;
	jsr     _rand8
	jsr     pushax
	lda     #$07
	jsr     tosumoda0
	ldy     #$00
	sta     (sp),y
;
; next_cluster.def = cluster_defs[id]; // def_z_rev_clust;
;
	ldx     #$00
	lda     (sp),y
	asl     a
	bcc     L0567
	inx
	clc
L0567:	adc     #<(_cluster_defs)
	sta     ptr1
	txa
	adc     #>(_cluster_defs)
	sta     ptr1+1
	iny
	lda     (ptr1),y
	tax
	dey
	lda     (ptr1),y
	sta     _next_cluster+2
	stx     _next_cluster+2+1
;
; next_cluster.layout = next_cluster.def[0];
;
	sta     ptr1
	stx     ptr1+1
	iny
	lda     (ptr1),y
	sta     _next_cluster+1
	dey
	lda     (ptr1),y
	sta     _next_cluster
;
; next_cluster.sprite = cluster_sprites[id];
;
	lda     (sp),y
	tay
	lda     _cluster_sprites,y
	sta     _next_cluster+4
;
; }
;
	jmp     incsp1

.endproc

; ---------------------------------------------------------------
; void __near__ rotate_cur_cluster (unsigned char)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_rotate_cur_cluster: near

.segment	"CODE"

;
; {
;
	jsr     pusha
;
; old_rot = cur_rot;
;
	jsr     decsp1
	lda     _cur_rot
	ldy     #$00
	sta     (sp),y
;
; cur_rot = (cur_rot + dir) & 3; // % 4
;
	iny
	lda     (sp),y
	clc
	adc     _cur_rot
	and     #$03
	sta     _cur_rot
;
; cur_cluster.layout = cur_cluster.def[cur_rot];
;
	ldx     #$00
	lda     _cur_rot
	asl     a
	bcc     L056B
	inx
	clc
L056B:	adc     _cur_cluster+2
	sta     ptr1
	txa
	adc     _cur_cluster+2+1
	sta     ptr1+1
	lda     (ptr1),y
	sta     _cur_cluster+1
	dey
	lda     (ptr1),y
	sta     _cur_cluster
;
; if (is_cluster_colliding())
;
	jsr     _is_cluster_colliding
	tax
	beq     L04FB
;
; cur_rot = old_rot;
;
	ldy     #$00
	lda     (sp),y
	sta     _cur_rot
;
; cur_cluster.layout = cur_cluster.def[cur_rot];
;
	ldx     #$00
	lda     _cur_rot
	asl     a
	bcc     L056C
	inx
	clc
L056C:	adc     _cur_cluster+2
	sta     ptr1
	txa
	adc     _cur_cluster+2+1
	sta     ptr1+1
	iny
	lda     (ptr1),y
	sta     _cur_cluster+1
	dey
	lda     (ptr1),y
	sta     _cur_cluster
;
; }
;
L04FB:	jmp     incsp2

.endproc

; ---------------------------------------------------------------
; void __near__ debug_fill_nametables (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_debug_fill_nametables: near

.segment	"CODE"

;
; vram_adr(NTADR_A(0,0));
;
	ldx     #$20
	lda     #$00
	jsr     _vram_adr
;
; vram_fill('a', NAMETABLE_PATTERN_SIZE);
;
	lda     #$61
	jsr     pusha
	ldx     #$03
	lda     #$C0
	jsr     _vram_fill
;
; vram_adr(NTADR_B(0,0));
;
	ldx     #$24
	lda     #$00
	jsr     _vram_adr
;
; vram_fill('b', NAMETABLE_PATTERN_SIZE);
;
	lda     #$62
	jsr     pusha
	ldx     #$03
	lda     #$C0
	jsr     _vram_fill
;
; vram_adr(NTADR_B(0,0));
;
	ldx     #$24
	lda     #$00
	jsr     _vram_adr
;
; vram_fill('c', NAMETABLE_PATTERN_SIZE);
;
	lda     #$63
	jsr     pusha
	ldx     #$03
	lda     #$C0
	jsr     _vram_fill
;
; vram_adr(NTADR_D(0,0));
;
	ldx     #$2C
	lda     #$00
	jsr     _vram_adr
;
; vram_fill('d', NAMETABLE_PATTERN_SIZE);
;
	lda     #$64
	jsr     pusha
	ldx     #$03
	lda     #$C0
	jmp     _vram_fill

.endproc

; ---------------------------------------------------------------
; void __near__ debug_draw_board_area (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_debug_draw_board_area: near

.segment	"CODE"

;
; oam_spr(BOARD_START_X_PX, BOARD_START_Y_PX, 0x01, 0);
;
	jsr     decsp3
	lda     #$60
	ldy     #$02
	sta     (sp),y
	lda     #$20
	dey
	sta     (sp),y
	tya
	dey
	sta     (sp),y
	tya
	jsr     _oam_spr
;
; oam_spr(BOARD_END_X_PX, BOARD_START_Y_PX, 0x01, 0);
;
	jsr     decsp3
	lda     #$A8
	ldy     #$02
	sta     (sp),y
	lda     #$20
	dey
	sta     (sp),y
	tya
	dey
	sta     (sp),y
	tya
	jsr     _oam_spr
;
; oam_spr(BOARD_START_X_PX, BOARD_END_Y_PX, 0x01, 0);
;
	jsr     decsp3
	lda     #$60
	ldy     #$02
	sta     (sp),y
	lda     #$B8
	dey
	sta     (sp),y
	tya
	dey
	sta     (sp),y
	tya
	jsr     _oam_spr
;
; oam_spr(BOARD_END_X_PX, BOARD_END_Y_PX, 0x01, 0);
;
	jsr     decsp3
	lda     #$A8
	ldy     #$02
	sta     (sp),y
	lda     #$B8
	dey
	sta     (sp),y
	tya
	dey
	sta     (sp),y
	tya
	jmp     _oam_spr

.endproc

; ---------------------------------------------------------------
; void __near__ main (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_main: near

.segment	"CODE"

;
; ppu_off(); // screen off
;
	jsr     decsp3
	jsr     _ppu_off
;
; pal_bg(palette_bg);
;
	lda     #<(_palette_bg)
	ldx     #>(_palette_bg)
	jsr     _pal_bg
;
; pal_spr(palette_sp);
;
	lda     #<(_palette_sp)
	ldx     #>(_palette_sp)
	jsr     _pal_spr
;
; bank_spr(1);
;
	lda     #$01
	jsr     _bank_spr
;
; set_vram_buffer(); // do at least once, sets a pointer to a buffer
;
	jsr     _set_vram_buffer
;
; clear_vram_buffer();
;
	jsr     _clear_vram_buffer
;
; vram_adr(NTADR_A(16-(sizeof(text)>>1),20));
;
	ldx     #$22
	lda     #$88
	jsr     _vram_adr
;
; vram_write(text, sizeof(text)-1); // -1 null term
;
	lda     #<(_text)
	ldx     #>(_text)
	jsr     pushax
	ldx     #$00
	lda     #$0F
	jsr     _vram_write
;
; scroll(0, 239); // shift the bg down 1 pixel
;
	jsr     push0
	lda     #$EF
	jsr     _scroll
;
; ppu_on_all(); // turn on screen
;
	jsr     _ppu_on_all
;
; ppu_wait_nmi(); // wait till beginning of the frame
;
L036E:	jsr     _ppu_wait_nmi
;
; tick_count++;
;
	inc     _tick_count
;
; pad1 = pad_poll(0); // read the first controller
;
	lda     #$00
	jsr     _pad_poll
	sta     _pad1
;
; pad1_new = get_pad_new(0); // newly pressed button. do pad_poll first
;
	lda     #$00
	jsr     _get_pad_new
	sta     _pad1_new
;
; clear_vram_buffer(); // do at the beginning of each frame
;
	jsr     _clear_vram_buffer
;
; switch(state)
;
	lda     _state
;
; }
;
	beq     L0571
	cmp     #$01
	jeq     L03B5
	cmp     #$02
	beq     L036E
	jmp     L036E
;
; if (pad1_new & PAD_START)
;
L0571:	lda     _pad1_new
	and     #$10
	beq     L036E
;
; seed_rng();
;
	jsr     _seed_rng
;
; ppu_off(); // screen off
;
	jsr     _ppu_off
;
; vram_adr(NTADR_A(0,0));
;
	ldx     #$20
	lda     #$00
	jsr     _vram_adr
;
; vram_fill(0, NAMETABLE_SIZE);
;
	lda     #$00
	jsr     pusha
	ldx     #$04
	jsr     _vram_fill
;
; vram_adr(NTADR_A(0,0));
;
	ldx     #$20
	lda     #$00
	jsr     _vram_adr
;
; vram_unrle(game_area);
;
	lda     #<(_game_area)
	ldx     #>(_game_area)
	jsr     _vram_unrle
;
; for (iy = 10; iy <= BOARD_END_Y_PX_BOARD; ++iy)
;
	lda     #$0A
	ldy     #$01
L056D:	sta     (sp),y
	cmp     #$14
	bcs     L0398
;
; vram_adr(NTADR_A(BOARD_START_X_PX >> 3, (BOARD_START_Y_PX >> 3) + iy));
;
	ldx     #$00
	lda     (sp),y
	clc
	adc     #$04
	bcc     L03A6
	inx
L03A6:	jsr     shlax4
	stx     tmp1
	asl     a
	rol     tmp1
	ora     #$0C
	pha
	lda     tmp1
	ora     #$20
	tax
	pla
	jsr     _vram_adr
;
; vram_fill(1, 10);
;
	lda     #$01
	jsr     pusha
	ldx     #$00
	lda     #$0A
	jsr     _vram_fill
;
; for (iy = 10; iy <= BOARD_END_Y_PX_BOARD; ++iy)
;
	ldy     #$01
	clc
	tya
	adc     (sp),y
	jmp     L056D
;
; memfill(game_board + 100, 1, 100);
;
L0398:	jsr     decsp3
	lda     #<(_game_board+100)
	sta     (sp),y
	iny
	lda     #>(_game_board+100)
	sta     (sp),y
	lda     #$01
	ldy     #$00
	sta     (sp),y
	ldx     #$00
	lda     #$64
	jsr     _memfill
;
; ppu_on_all(); // turn on screen
;
	jsr     _ppu_on_all
;
; spawn_new_cluster();
;
	jsr     _spawn_new_cluster
;
; spawn_new_cluster();
;
	jsr     _spawn_new_cluster
;
; state = STATE_GAME;
;
	lda     #$01
	sta     _state
;
; break;
;
	jmp     L036E
;
; if (do_line_check)
;
L03B5:	lda     _do_line_check
	beq     L0573
;
; do_line_check = 0;
;
	lda     #$00
	sta     _do_line_check
;
; for (iy = BOARD_END_Y_PX_BOARD; iy > 0; --iy)
;
	lda     #$13
	ldy     #$01
L056F:	sta     (sp),y
	lda     (sp),y
	beq     L0573
;
; line_complete = 1;
;
	tya
	dey
	sta     (sp),y
;
; for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
;
	tya
	ldy     #$02
L056E:	sta     (sp),y
	cmp     #$0A
	bcs     L03C5
;
; if (is_block_free(ix, iy))
;
	lda     (sp),y
	jsr     pusha
	ldy     #$02
	lda     (sp),y
	jsr     _is_block_free
	tax
	beq     L03C6
;
; line_complete = 0;
;
	lda     #$00
	tay
	sta     (sp),y
;
; break;
;
	jmp     L0572
;
; for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
;
L03C6:	ldy     #$02
	clc
	lda     #$01
	adc     (sp),y
	jmp     L056E
;
; if (line_complete)
;
L03C5:	ldy     #$00
L0572:	lda     (sp),y
	beq     L03BC
;
; line_crush_y = iy;
;
	iny
	lda     (sp),y
	sta     _line_crush_y
;
; break;
;
	jmp     L0573
;
; for (iy = BOARD_END_Y_PX_BOARD; iy > 0; --iy)
;
L03BC:	iny
	lda     (sp),y
	sec
	sbc     #$01
	jmp     L056F
;
; if (line_crush_y > 0)
;
L0573:	lda     _line_crush_y
	beq     L03D6
;
; for(ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
;
	lda     #$00
	ldy     #$02
L0570:	sta     (sp),y
	cmp     #$0A
	bcs     L0574
;
; set_block(ix, line_crush_y, get_block(ix, line_crush_y-1));
;
	jsr     decsp2
	ldy     #$04
	lda     (sp),y
	ldy     #$01
	sta     (sp),y
	lda     _line_crush_y
	dey
	sta     (sp),y
	ldy     #$04
	lda     (sp),y
	jsr     pusha
	lda     _line_crush_y
	sec
	sbc     #$01
	jsr     _get_block
	jsr     _set_block
;
; for(ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
;
	ldy     #$02
	clc
	lda     #$01
	adc     (sp),y
	jmp     L0570
;
; --line_crush_y;
;
L0574:	dec     _line_crush_y
;
; if (line_crush_y == 0)
;
	bne     L03EC
;
; do_line_check = 1;
;
	lda     #$01
	sta     _do_line_check
;
; else
;
	jmp     L03EC
;
; movement();
;
L03D6:	jsr     _movement
;
; draw_sprites();
;
L03EC:	jsr     _draw_sprites
;
; break;
;
	jmp     L036E

.endproc

