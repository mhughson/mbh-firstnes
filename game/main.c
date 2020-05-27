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
	int i;
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

	off_nt = 0;
	cur_nt = 2;

	vram_adr(NTADR_A(16-(sizeof(text)>>1),20));
	vram_write(text, sizeof(text)-1); // -1 null term
	
	scroll(0, 0x1df); // shift the bg down 1 pixel
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

					go_to_state(STATE_GAME);
				}
				break;
			}

			case STATE_GAME:
			{
				// Search for full rows to clear out.
				if (do_line_check)
				{
					// Stop searching for lines unless we find one this frame.
					do_line_check = 0;

					// Start at the bottom of the board, and work our way up.
					for (iy = line_check_start; iy > BOARD_OOB_END; --iy)
					{
						// Assume this row is complete unless we find an empty
						// block.
						line_complete = 1;
						for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
						{
							if (is_block_free(ix, iy))
							{
								// This block is empty, so we can stop checking this row.
								line_complete = 0;
								break;
							}
						}

						// If this row was filled, we need to remove it and crush
						// the rows above it into its place.
						if (line_complete)
						{
							inc_lines_cleared();
							display_lines_cleared();

							// Store line to crush.
							line_crush_y = iy;

							// hide the primary nt.
							scroll(0, 0x1df); // shift the bg down 1 pixel
							cur_nt = 0;
							off_nt = 2;

							break;
						}

						// found a line so there might be more.
						//do_line_check = 1;
					}

					// If we have finished updating the screen and are ready to move back to the primary
					// nt, we can start updating the offscreen nt.
					if (line_complete == 0)
					{
						refresh_offscreen_nt = BOARD_END_Y_PX_BOARD;
					}
				}
			

				// if (line_crush_y > BOARD_OOB_END)
				// {
				// 	// Set each block in this row to the value in the row above it.
				// 	for(ix = 0; ix <= 3; ++ix)
				// 	{
				// 		multi_vram_buffer_vert(
				// 			full_col, 
				// 			19,
				// 			get_ppu_addr(0, BOARD_START_X_PX + (ix << 3), (BOARD_OOB_END + 1) << 3));
				// 	}
				// }
				// // Are we currently shifting rows down?
				// else 
				if (line_crush_y > BOARD_OOB_END)
				{
					//for (i = 0; i < 2 && line_crush_y > BOARD_OOB_END; ++i)
					//{
					// Set each block in this row to the value in the row above it.
					for(ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
					{
						set_block_nt(ix, line_crush_y, get_block(ix, line_crush_y-1), off_nt);
					}

					// Next frame do the same on the line above.
					--line_crush_y;
					//}
					// Finished this pass, check again incase this was a multi-line
					// kill.
					if (line_crush_y == BOARD_OOB_END)
					{
						do_line_check = 1;
					}
				}
				else
				{
					// show the primary nt.
					cur_nt = 2;
					off_nt = 0;				
					scroll(0, 255 - 16);

					if (refresh_offscreen_nt)
					{
						for(ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
						{
							set_block_nt(ix, refresh_offscreen_nt, 
								get_block(ix, refresh_offscreen_nt), off_nt);
						}

						// Next frame do the same on the line above.
						--refresh_offscreen_nt;

						// Finished this pass, check again incase this was a multi-line
						// kill.
						if (refresh_offscreen_nt == BOARD_OOB_END)
						{
							refresh_offscreen_nt = 0;
						}
					}
					//else
					{
						movement();
					}
				}
				
				draw_sprites();

				if (pad1_new & PAD_START)
				{
					go_to_state(STATE_OVER);
				}

				break;
			}

			case STATE_OVER:
			{
				if (pad1_new & PAD_START)
				{
					go_to_state(STATE_GAME);
				}
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
				// Don't draw the current cluster if it is above the top of the board.
				// We want it to be able to function and move up there, but should not
				// be visible.
				if (start_y + (iy << 3) > (BOARD_START_Y_PX + (BOARD_OOB_END << 3)))
				{
					oam_spr(start_x + (ix << 3), start_y + (iy << 3), cur_cluster.sprite, 0);
				}
			}
			// else
			// {
			// 	oam_spr(start_x + (ix << 3), start_y + (iy << 3), 0x01, 0);
			// }
			
		}
	}

	start_x = 15 << 3;
	start_y = 0 << 3;

	for (iy = 0; iy < 4; ++iy)
	{	
		for (ix = 0; ix < 4; ++ix)
		{
			// essentially an index into a bit array.
			unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4

			if (next_cluster.layout & (0x8000 >> bit))
			{
				oam_spr(start_x + (ix << 3), start_y + (iy << 3), next_cluster.sprite, 0);
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

	++fall_frame_counter;

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

	--horz_button_delay;

	old_x = cur_block.x;
	if (((pad1 & PAD_RIGHT) && horz_button_delay == 0) || (pad1_new & PAD_RIGHT))
	{
		horz_button_delay = button_delay;
		if ((pad1_new & PAD_RIGHT))
		{
			horz_button_delay <<= 1;
		}

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
	else if (((pad1 & PAD_LEFT) && horz_button_delay == 0) || pad1_new & PAD_LEFT)
	{
		horz_button_delay = button_delay;
		if ((pad1_new & PAD_LEFT))
		{
			horz_button_delay <<= 1;
		}
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

	// Only check for collision if we actually moved horz.
	// Otherwise spawning into another tile will cause horz
	// correction.
	if (cur_block.x != old_x && is_cluster_colliding())
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
	if (pad1_new & PAD_DOWN)
	{
		require_new_down_button = 0;

		// fall this frame.
		temp_fall_rate = fall_frame_counter;
	}
	else if ((pad1 & PAD_DOWN) && require_new_down_button == 0)
	{
		// fall 16 times as often.
		temp_fall_rate >>= 4;
	}

	if (fall_frame_counter % temp_fall_rate == 0)
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

void set_block(unsigned char x, unsigned char y, unsigned char id)
{
	int address;

	// w = 10 tiles,  80 px
	// h = 20 tiles, 160 px

	// Update the logic array as well as the nametable to reflect it.

	if (y <= BOARD_OOB_END)
	{
		// Don't place stuff out of bounds.
		return;
	}

	address = get_ppu_addr(cur_nt, (x << 3) + BOARD_START_X_PX, (y << 3) + BOARD_START_Y_PX);
	one_vram_buffer(id, address);	
	address = get_ppu_addr(off_nt, (x << 3) + BOARD_START_X_PX, (y << 3) + BOARD_START_Y_PX);
	one_vram_buffer(id, address);

	//x = x >> 3; // div 8
	//y = y >> 3; // div 8

	// TODO: Is this too slow?
	game_board[PIXEL_TO_BOARD_INDEX(x,y)] = id;
}

void set_block_nt(unsigned char x, unsigned char y, unsigned char id, unsigned char nt)
{
	int address;
	if (y <= BOARD_OOB_END)
	{
		// Don't place stuff out of bounds.
		return;
	}

	address = get_ppu_addr(nt, (x << 3) + BOARD_START_X_PX, (y << 3) + BOARD_START_Y_PX);
	one_vram_buffer(id, address);
	
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
	unsigned char min_y;
	unsigned char max_y;
	//unsigned char iy2;
	//unsigned char line_complete;
	//unsigned char top;
	//unsigned char bottom;
	//int address;

	min_y = 0xff; // max

	for (iy = 0; iy < 4; ++iy)
	{	
		for (ix = 0; ix < 4; ++ix)
		{
			// essentially an index into a bit array.
			unsigned char bit = ((iy * 4) + (ix & 3)); // &3 = %4

			// solid bit.
			if (cur_cluster.layout & (0x8000 >> bit))
			{
				// This is basically always going to be the first thing drawn,
				// but i couldn't think of a clever way to do this once.
				if (cur_block.y + iy < min_y)
				{
					min_y = cur_block.y + iy;
				}
				if (cur_block.y + iy > max_y)
				{
					max_y = cur_block.y + iy;
				}
				set_block(cur_block.x + ix, cur_block.y + iy, cur_cluster.sprite);
			}			
		}
	}

	if (min_y <= BOARD_OOB_END)
	{
		go_to_state(STATE_OVER);
		return;
	}
	else
	{
		line_check_start = max_y;
		do_line_check = 1;
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

	require_new_down_button = 1;
	fall_frame_counter = 0;

	cur_rot = 0;

	// Copy the next cluster to the current one.
	cur_cluster.def = next_cluster.def;
	cur_cluster.layout = cur_cluster.def[0];
	cur_cluster.sprite = next_cluster.sprite;
	cur_cluster.id = next_cluster.id;

	// Reset the block.
	cur_block.x = 3; //(BOARD_END_Y_PX_BOARD >> 1);
	cur_block.y = cluster_offsets[cur_cluster.id];

	// If the block is colliding right out of the game, move it up so that
	// we get a cleaner game over.
	if (is_cluster_colliding())
	{
		--cur_block.y;
	}
	// if (is_cluster_colliding())
	// {
	// 	--cur_block.y;
	// }

	id = rand8() % NUM_CLUSTERS;
	next_cluster.id = id;
	next_cluster.def = cluster_defs[id]; // def_z_rev_clust;
	next_cluster.layout = next_cluster.def[0];
	next_cluster.sprite = cluster_sprites[id];
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

void go_to_state(unsigned char new_state)
{
	int address;
	unsigned char i;
	unsigned char fade_from_bright;
	unsigned char fade_delay;
	fade_delay = 5;

	switch (state)
	{
		case STATE_OVER:
		{
			// We would have faded up to white entering game over, so now
			// we need to fade back down.
			fade_from_bright = 1;
			break;
		}
	
	default:
		break;
	}

	state = new_state;

	switch (state)
	{
		case STATE_MENU:
		{
			break;
		}

		case STATE_GAME:
		{
			ppu_off(); // screen off

			// clear the nametable and attributes.
			//vram_adr(NTADR_A(0,0));
			//vram_fill(0, NAMETABLE_SIZE);
			vram_adr(NTADR_A(0,0));
			vram_unrle(game_area);
			vram_adr(NTADR_C(0,0));
			vram_unrle(game_area);

			ppu_on_all(); // turn on screen

			// for (iy = 0; iy <= BOARD_END_Y_PX_BOARD; ++iy)
			// {
			// 	vram_adr(NTADR_A(BOARD_START_X_PX >> 3, (BOARD_START_Y_PX >> 3) + iy));
			// 	vram_fill(0, 10);
			// }

			memfill(game_board, 0, BOARD_SIZE);

			// shift up 1
			scroll(0, 255 - 16);

			display_lines_cleared();

			// leave a spot open.
			for (i=0; i < BOARD_END_X_PX_BOARD; ++i)
			{
				set_block(i, BOARD_END_Y_PX_BOARD, 1);
				//delay(1);
				//set_block(i, BOARD_END_Y_PX_BOARD - 1, 1);
				//delay(1);
				//set_block(i, BOARD_END_Y_PX_BOARD - 2, 1);
				//delay(1);
				//set_block(i, BOARD_END_Y_PX_BOARD - 3, 1);
				//delay(1);
			}

			spawn_new_cluster();
			spawn_new_cluster();

			if (fade_from_bright)
			{
				pal_bright(7);
				delay(fade_delay);
				pal_bright(6);
				delay(fade_delay);
				pal_bright(5);
				delay(fade_delay);
				pal_bright(4);
				delay(fade_delay);
			}

			require_new_down_button = 1;

			break;
		}

		case STATE_OVER:
		{
			delay(60);

			oam_clear();

			pal_bright(5);
			delay(fade_delay);
			pal_bright(6);
			delay(fade_delay);
			pal_bright(7);
			delay(fade_delay);
			pal_bright(8);
			delay(fade_delay);
			address = get_ppu_addr(cur_nt, 96, 112);
			multi_vram_buffer_horz("GAME OVER!", 10, address);
			pal_bright(7);
			delay(fade_delay);
			pal_bright(6);
			delay(fade_delay);
			pal_bright(5);
			delay(fade_delay);
			pal_bright(4);
			delay(fade_delay);
			break;
		}

		default:
		{
			break;
		}
	}
}

void inc_lines_cleared()
{
	++lines_cleared_one;
	if (lines_cleared_one == 10)
	{
		lines_cleared_one = 0;
		++lines_cleared_ten;
		if (lines_cleared_ten == 10)
		{
			lines_cleared_ten = 0;
			++lines_cleared_hundred;
		}
	}
}

void display_lines_cleared()
{
	one_vram_buffer('0' + lines_cleared_hundred, get_ppu_addr(cur_nt,0,0));
	one_vram_buffer('0' + lines_cleared_ten, get_ppu_addr(cur_nt,8,0));
	one_vram_buffer('0' + lines_cleared_one, get_ppu_addr(cur_nt,16,0));

	one_vram_buffer('0' + lines_cleared_hundred, get_ppu_addr(off_nt,0,0));
	one_vram_buffer('0' + lines_cleared_ten, get_ppu_addr(off_nt,8,0));
	one_vram_buffer('0' + lines_cleared_one, get_ppu_addr(off_nt,16,0));
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