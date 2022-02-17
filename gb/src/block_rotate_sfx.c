/*

	block_rotate_sfx

	Sound Effect File.

	Info:
		Length			:	3
		Bank			:	0
		Priority		:	0
		Channels used	:	Duty channel 2

	This file was generated by hammer2cbt

*/
#pragma bank 5
#define block_rotate_sfx_Length 3
#define block_rotate_sfx_Bank 0
#define block_rotate_sfx_CH_Used 240
#define block_rotate_sfx_Priority 0
#define CBTFX_PLAY_block_rotate_sfx CBTFX_init(&block_rotate_sfx[0][0], 3, 0, 240)
#include "cbtfx.h"

const unsigned char block_rotate_sfx[3][CBTFX_LENGTH] = {
	CBTFX_FRAME(1, 17, 7, 1, FX_Ds3, 0, 0, 0),
	CBTFX_FRAME(1, 17, 7, 1, FX_As3, 0, 0, 0)
};