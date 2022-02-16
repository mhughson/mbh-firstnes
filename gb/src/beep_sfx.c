/*

	beep_sfx

	Sound Effect File.

	Info:
		Length			:	4
		Bank			:	0
		Priority		:	0
		Channels used	:	Duty channel 2

	This file was generated by hammer2cbt

*/
#pragma bank 5
#define beep_sfx_Length 4
#define beep_sfx_Bank 0
#define beep_sfx_CH_Used 240
#define beep_sfx_Priority 0
#define CBTFX_PLAY_beep_sfx CBTFX_init(&beep_sfx[0][0], 4, 0, 240)
#include "cbtfx.h"

const unsigned char beep_sfx[4][CBTFX_LENGTH] = {
	CBTFX_FRAME(1, 17, 5, 2, FX_As2, 0, 0, 0),
	CBTFX_FRAME(1, 17, 5, 2, FX_G_3, 0, 0, 0),
	CBTFX_FRAME(1, 17, 1, 2, FX_As2, 0, 0, 0)
};