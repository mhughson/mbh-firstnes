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
#ifndef __beep_sfx_h_INCLUDE
#define __beep_sfx_h_INCLUDE
#define beep_sfx_Length 4
#define beep_sfx_Bank 0
#define beep_sfx_CH_Used 240
#define beep_sfx_Priority 0
#define CBTFX_PLAY_beep_sfx CBTFX_init(&beep_sfx[0][0], 4, 0, 240)
extern const unsigned char beep_sfx[4][CBTFX_LENGTH];
#endif