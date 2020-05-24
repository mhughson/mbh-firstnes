/** (C) Matt Hughson 2020 */

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "Sprites.h" // holds our metasprite data
#include "BG/game_area.h"
#include "main.h"

void main (void) 
{	
	unsigned char ix;
	unsigned char iy;
	unsigned char line_complete;
	//int address;

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

	//debug_fill_nametables();

	vram_adr(NTADR_A(16-(sizeof(text)>>1),20));
	vram_write(text, sizeof(text)-1); // -1 null term
	
	scroll(0, 239); // shift the bg down 1 pixel
	//set_scroll_y(0xff);
	
	ppu_on_all(); // turn on screen

	while (1)
	{
		// infinite loop
		ppu_wait_nmi(); // wait till beginning of the frame

		tick_count++;
		
		pad1 = pad_poll(0); // read the first controller
		pad1_new = get_pad_new(0); // newly pressed button. do pad_poll first
		
		clear_vram_buffer(); // do at the beginning of each frame

		switch(state)
		{
			case STATE_MENU:
			{
				if (pad1_new & PAD_START)
				{
					seed_rng();

					ppu_off(); // screen off

					// clear the nametable and attributes.
					vram_adr(NTADR_A(0,0));
					vram_fill(0, NAMETABLE_SIZE);
					vram_adr(NTADR_A(0,0));
					vram_unrle(game_area);
					ppu_on_all(); // turn on screen

					//cur_block.x = BOARD_START_X_PX;
					//cur_block.y = BOARD_START_Y_PX;

					//put_block(0, 0 + 152);
					/*
					put_block(rand8() % BOARD_END_X_PX_BOARD, rand8() % BOARD_END_Y_PX_BOARD);
					put_block(rand8() % BOARD_END_X_PX_BOARD, rand8() % BOARD_END_Y_PX_BOARD);
					put_block(rand8() % BOARD_END_X_PX_BOARD, rand8() % BOARD_END_Y_PX_BOARD);
					put_block(rand8() % BOARD_END_X_PX_BOARD, rand8() % BOARD_END_Y_PX_BOARD);
					put_block(rand8() % BOARD_END_X_PX_BOARD, rand8() % BOARD_END_Y_PX_BOARD);
					put_block(rand8() % BOARD_END_X_PX_BOARD, rand8() % BOARD_END_Y_PX_BOARD);
					put_block(rand8() % BOARD_END_X_PX_BOARD, rand8() % BOARD_END_Y_PX_BOARD);
					*/

					/*
					put_block(0, BOARD_END_Y_PX_BOARD);
					put_block(1, BOARD_END_Y_PX_BOARD);
					put_block(2, BOARD_END_Y_PX_BOARD);
					put_block(3, BOARD_END_Y_PX_BOARD);
					put_block(4, BOARD_END_Y_PX_BOARD);
					put_block(5, BOARD_END_Y_PX_BOARD);
					put_block(6, BOARD_END_Y_PX_BOARD);
					put_block(7, BOARD_END_Y_PX_BOARD);
					put_block(8, BOARD_END_Y_PX_BOARD);
					put_block(9, BOARD_END_Y_PX_BOARD);
					*/

					spawn_new_cluster();

					state = STATE_GAME;
				}
				break;
			}

			case STATE_GAME:
			{
								// clear out lines.
				if (do_line_check)
				{
					do_line_check = 0;
					for (iy = BOARD_END_Y_PX_BOARD; iy > 1; --iy)
					{	
						line_complete = 1;
						for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
						{
							if (is_block_free(ix, iy))
							{
								line_complete = 0;
								break;
							}
						}

						if (line_complete)
						{
							line_crush_y = iy;
							break;
						}

						// found a line so there might be more.
						//do_line_check = 1;
					}
				}

				if (line_crush_y > 0)
				{
					for(ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
					{
						set_block(ix, line_crush_y, get_block(ix, line_crush_y-1));
					}
					--line_crush_y;

					// Finished this pass, check again incase this was a multi-line
					// kill.
					if (line_crush_y == 0)
					{
						do_line_check = 1;
					}
				}
				else
				{
					movement();
				}
				
				draw_sprites();

				break;
			}

			case STATE_OVER:
			{
				break;
			}
		}
	}
}

void draw_sprites(void)
{
	unsigned char start_x;
	unsigned char start_y;
	unsigned char ix;
	unsigned char iy;

	// clear all sprites from sprite buffer
	oam_clear();

	// push a single sprite
	// oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned char attr);
	// use tile #0, palette #0

	start_x = (cur_block.x << 3) + BOARD_START_X_PX;
	start_y = (cur_block.y << 3) + BOARD_START_Y_PX;

	for (iy = 0; iy < 4; ++iy)
	{	
		for (ix = 0; ix < 4; ++ix)
		{
			// essentially an index into a bit array.
			unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4

			if (cur_cluster.layout & (0x8000 >> bit))
			{
				oam_spr(start_x + (ix << 3), start_y + (iy << 3), cur_cluster.sprite, 1);
			}
			// else
			// {
			// 	oam_spr(start_x + (ix << 3), start_y + (iy << 3), 0x01, 0);
			// }
			
		}
	}

	//debug_draw_board_area();
}

void movement(void)
{
	char hit;
	unsigned char temp_fall_rate;
	unsigned char old_x;

#if DEBUG_ENABLED
	if (pad1_new & PAD_SELECT)
	{
		spawn_new_cluster();
	}
#endif // DEBUG_ENABLED

	// INPUT

	if (pad1_new & PAD_A)
	{
		rotate_cur_cluster(1);
	}
	else if (pad1_new & PAD_B)
	{
		rotate_cur_cluster(-1);
	}

	// TODO: Allow repeats
	if (/*(pad1 & PAD_RIGHT && (tick_count % 4 == 0)) ||*/ pad1_new & PAD_RIGHT)
	{
		// if (cur_block.x < BOARD_END_X_PX_BOARD)
		// {
			old_x = cur_block.x;
			cur_block.x += 1;
		// }
		// else
		// {
		// 	cur_block.x = BOARD_END_X_PX_BOARD;
		// }
		
	}
	else if (/*(pad1 & PAD_LEFT && (tick_count % 4 == 0)) ||*/ pad1_new & PAD_LEFT)
	{
		// handle here since x is unsigned.
		// if (cur_block.x > 0)
		// {
			old_x = cur_block.x;
			cur_block.x -= 1; // note: wrap around
		// }
		// else
		// {
		// 	cur_block.x = 0;
		// }
		
	}

	if (is_cluster_colliding())
	{
		cur_block.x = old_x;
	}

	// FALLING

	// TODO: There is some inconsistencies with this approach.
	//		 A frame after pressing Down might turn out to be
	//		 the normal fall rate frame, causing 2 frames in a row
	//		 to fall.
	//		 Similarly, shortly after releasing Down, we might hit
	//		 a natural fall frame, causing it to seem like down was
	//		 held a little longer.
	//		 To fix, I think tick_count should be tracked seperate for
	//		 this function so that it can be manipulated (eg. when release
	//	     reset the tick count).
	temp_fall_rate = fall_rate;
	if (pad1_new & PAD_DOWN || pad1 & PAD_UP)
	{
		// fall this frame.
		temp_fall_rate = tick_count;
	}
	else if (pad1 & PAD_DOWN)
	{
		// fall 16 times as often.
		temp_fall_rate >>= 4;
	}

	if (tick_count % temp_fall_rate == 0)
	{
		cur_block.y += 1;
	}


	hit = 0;
	
	// Offset from the bottom.
	if (is_cluster_colliding())
	{

		// Clamped to tile space, then multiplied back to pixel space
		//cur_block.y = (cur_block.y >> 3) << 3;

		// Move it above the collided tile.
		cur_block.y -= 1;

		hit = 1;
	}

	if (hit)
	{
		put_cur_cluster();
		
		// Spawn a new block.
		spawn_new_cluster();
	}

}

void put_block(unsigned char x, unsigned char y)
{
	set_block(x, y, 1);
}

void set_block(unsigned char x, unsigned char y, unsigned char id)
{
	int address;

	// w = 10 tiles,  80 px
	// h = 20 tiles, 160 px

	// Update the logic array as well as the nametable to reflect it.

	address = get_ppu_addr(0, (x << 3) + BOARD_START_X_PX, (y << 3) + BOARD_START_Y_PX);
	one_vram_buffer(id, address);

	//x = x >> 3; // div 8
	//y = y >> 3; // div 8

	// TODO: Is this too slow?
	game_board[PIXEL_TO_BOARD_INDEX(x,y)] = id;
}

void clear_block(unsigned char x, unsigned char y)
{
	set_block(x, y, 0);
}

void put_cur_cluster()
{
	unsigned char ix;
	unsigned char iy;
	//unsigned char iy2;
	//unsigned char line_complete;
	//unsigned char top;
	//unsigned char bottom;
	//int address;

	do_line_check = 1;

	for (iy = 0; iy < 4; ++iy)
	{	
		for (ix = 0; ix < 4; ++ix)
		{
			// essentially an index into a bit array.
			unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4

			// solid bit.
			if (cur_cluster.layout & (0x8000 >> bit))
			{
				put_block(cur_block.x + ix, cur_block.y + iy);
			}			
		}
	}
}

unsigned char get_block(unsigned char x, unsigned char y)
{
	return game_board[PIXEL_TO_BOARD_INDEX(x,y)];
}

unsigned char is_block_free(unsigned char x, unsigned char y)
{
	if (y > BOARD_END_Y_PX_BOARD || x > BOARD_END_X_PX_BOARD)
	{
		// consider this blocked.
		return 0;
	}

	return get_block(x, y) == 0;
}

unsigned char is_cluster_colliding()
{
	unsigned char ix;
	unsigned char iy;

	for (iy = 0; iy < 4; ++iy)
	{	
		for (ix = 0; ix < 4; ++ix)
		{
			// essentially an index into a bit array.
			unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4

			// solid bit.
			if (cur_cluster.layout & (0x8000 >> bit))
			{
				if (!is_block_free(cur_block.x + ix, cur_block.y + iy))
				{ 
					return 1;
				}
			}			
		}
	}

	return 0;
}

void spawn_new_cluster()
{
	unsigned char id;
	// Spawn a new block.
	cur_block.x = 3; //(BOARD_END_Y_PX_BOARD >> 1);
	cur_block.y = 0;

	cur_rot = 0;
	id = rand8() % NUM_CLUSTERS;
	cur_cluster.def = cluster_defs[id]; // def_z_rev_clust;
	cur_cluster.layout = cur_cluster.def[0];
	cur_cluster.sprite = cluster_sprites[id];
}

void rotate_cur_cluster(char dir)
{
	unsigned char old_rot;

	old_rot = cur_rot;

	cur_rot = (cur_rot + dir) & 3; // % 4
	cur_cluster.layout = cur_cluster.def[cur_rot];

	// if after rotating, we are now colliding we something, revert the rotation.
	if (is_cluster_colliding())
	{
		cur_rot = old_rot;
		cur_cluster.layout = cur_cluster.def[cur_rot];
	}

	/*
	// iterator through x and y of layout.
	unsigned char ix;
	unsigned char iy;

	// destination x and y that we want to copy to.
	unsigned char dx;
	unsigned char dy;

	// The bit index we want to copy from.
	unsigned char tbit;
	// The value of that bit in the current layout (off or on).
	unsigned char tval;

	// The rotated destination.
	unsigned char dbit;

	// Temp layout to copy into without impacting current layout.
	unsigned short ret;

	for (iy = 0; iy < 4; ++iy)
	{	
		for (ix = 0; ix < 4; ++ix)
		{
			// Essentially an index into a bit array.
			// The data we are copying.
			tbit = ((iy * 4) + (ix % 4));
			tval = cur_cluster.layout & (0x8000 >> tbit);

			// The destination.
			dx = 3 - iy;
			dy = ix;
			dbit = ((dy * 4) + (dx % 4));

			if (tval)
			{
				ret |= (0x8000 >> dbit);
			}
		}
	}

	cur_cluster.layout = ret;
	*/
}

// DEBUG

void debug_fill_nametables(void)
{
#if DEBUG_ENABLED
	vram_adr(NTADR_A(0,0));
	vram_fill('a', NAMETABLE_PATTERN_SIZE);
	vram_adr(NTADR_B(0,0));
	vram_fill('b', NAMETABLE_PATTERN_SIZE);
	vram_adr(NTADR_B(0,0));
	vram_fill('c', NAMETABLE_PATTERN_SIZE);
	vram_adr(NTADR_D(0,0));
	vram_fill('d', NAMETABLE_PATTERN_SIZE);
#endif //DEBUG_ENABLED
}

void debug_draw_board_area(void)
{
#if DEBUG_ENABLED
	oam_spr(BOARD_START_X_PX, BOARD_START_Y_PX, 0x01, 0);
	oam_spr(BOARD_END_X_PX, BOARD_START_Y_PX, 0x01, 0);
	oam_spr(BOARD_START_X_PX, BOARD_END_Y_PX, 0x01, 0);
	oam_spr(BOARD_END_X_PX, BOARD_END_Y_PX, 0x01, 0);
#endif//DEBUG_ENABLED
}