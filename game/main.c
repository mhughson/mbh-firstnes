/** (C) Matt Hughson 2020 */

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "Sprites.h" // holds our metasprite data
#include "main.h"

void main (void) {
	
	unsigned char p;
	char p_dir = -1;
	unsigned char tick;
	p = 4;

	ppu_off(); // screen off
	
	// load the palettes
	pal_bg(palette_bg);
	pal_spr(palette_sp);
	
	// use the second set of tiles for sprites
	// both bg and sprites are set to 0 by default
	bank_spr(1);

	set_vram_buffer(); // do at least once, sets a pointer to a buffer
	clear_vram_buffer();
	
	//load_room();

	// in tiles.
	// center x
	// lower third y.
	vram_adr(NTADR_A(17-(sizeof(text)>>1),20));
	vram_write(text, sizeof(text));
	
	set_scroll_y(0xff); // shift the bg down 1 pixel
	
	ppu_on_all(); // turn on screen
	

	while (1){
		// infinite loop
		ppu_wait_nmi(); // wait till beginning of the frame

		++tick;
		tick = tick & 0x7;
		
		pad1 = pad_poll(0); // read the first controller
		pad1_new = get_pad_new(0); // newly pressed button. do pad_poll first
		
		clear_vram_buffer(); // do at the beginning of each frame

		pal_bright(p);

		if (tick == 0)
		{
			p += p_dir;
			if (p >= 4)
			{
				p_dir = -1;
			}
			else if (p <= 0)
			{
				p_dir = 1;
			}
		}
		
		//movement();
		//draw_sprites();
	}
}

void draw_sprites(void)
{
	// clear all sprites from sprite buffer
	oam_clear();
}

void movement(void)
{

}