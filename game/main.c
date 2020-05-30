/** (C) Matt Hughson 2020 */

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "Sprites.h" // holds our metasprite data
#include "BG/game_area.h"
#include "main.h"

void main (void) 
{	
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
	
	// TODO: This is actually the gameplay setup.
	off_nt = 0;
	cur_nt = 2;

	// TODO: Use get_ppu functions with nt id.
	vram_adr(NTADR_A(16-(sizeof(text)>>1),20));
	vram_write(text, sizeof(text)-1); // -1 null term
	
	scroll(0, 0x1df); // shift the bg down 1 pixel
	//set_scroll_y(0xff);

	
	ppu_on_all(); // turn on screen

	// infinite loop
	while (1)
	{
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

				movement();
				draw_sprites();
				//copy_board_to_nt();

#if DEBUG_ENABLED
				if (pad1_new & PAD_START)
				{
					debug_copy_board_data_to_nt();
					//go_to_state(STATE_OVER);
				}
#endif

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

	// 255 means hide.
	if (cur_block.y != 255)
	{
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
	hit = 0;
	temp_fall_rate = 0;
	old_x = 0;

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

	if (fall_frame_counter % temp_fall_rate == 0 || temp_fall_rate == 0)
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


	// TODO: Is this too slow?
	game_board[TILE_TO_BOARD_INDEX(x,y)] = id;
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
	
	game_board[TILE_TO_BOARD_INDEX(x,y)] = id;
}

void clear_block(unsigned char x, unsigned char y)
{
	set_block(x, y, 0);
}

void put_cur_cluster()
{
	unsigned char ix;
	unsigned char iy;


	max_y = 0;
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
		// hide the sprite while we work.
		cur_block.y = 255;
		draw_sprites();

		clear_rows_in_data(max_y);
	}
	
}

unsigned char get_block(unsigned char x, unsigned char y)
{
	return game_board[TILE_TO_BOARD_INDEX(x,y)];
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
	id = 0;
	
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
			//vram_adr(NTADR_A(0,0));
			//vram_unrle(game_area);

			// TODO: Use get_ppu functions with nt id.
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
#if DEBUG_ENABLED
			// leave a spot open.
			for (i=0; i < BOARD_END_X_PX_BOARD; ++i)
			{
				set_block(i, BOARD_END_Y_PX_BOARD, 1);
				set_block(i, BOARD_END_Y_PX_BOARD - 1, 1);
				set_block(i, BOARD_END_Y_PX_BOARD - 2, 1);
				set_block(i, BOARD_END_Y_PX_BOARD - 3, 1);
				delay(1);
				clear_vram_buffer();
				set_block(i, BOARD_END_Y_PX_BOARD - 4, 1);
				set_block(i, BOARD_END_Y_PX_BOARD - 5, 1);
				set_block(i, BOARD_END_Y_PX_BOARD - 6, 1);
				set_block(i, BOARD_END_Y_PX_BOARD - 7, 1);
				delay(1);
				clear_vram_buffer();
			}
#endif //DEBUG_ENABLED
			//debug_display_number(123, 0);
			//debug_display_number(45, 1);
			//debug_display_number(6, 2);

			// Spawn "next"
			spawn_new_cluster();
			// "Next" becomes current, and a new next is defined.
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

// START OF ROW CLEAR SEQUENCE!

void clear_rows_in_data(unsigned char start_y)
{
	unsigned char ix;
	unsigned char iy;
	unsigned char line_complete;
	unsigned char i = 0;

	// 0xff used to indicate unused.
	memfill(lines_cleared_y, 0xff, 4);

	// Start at the bottom of the board, and work our way up.
	for (iy = start_y; iy > BOARD_OOB_END; --iy)
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

			// Fill the row will empty data.
			memcpy(&game_board[TILE_TO_BOARD_INDEX(0, iy)], empty_row, 10);

			// Keep track of rows that we cleared so that they can be quickly
			// collapsed later.
			lines_cleared_y[i] = iy;

			// Remember that we cleared some lines.
			++i;
		}
	}

	// If any lines we cleared, time to move to the next phase...
	if (i > 0)
	{
		reveal_empty_rows_to_nt();
	}
}

void reveal_empty_rows_to_nt()
{
	//multi_vram_buffer_vert(const char * data, unsigned char len, int ppu_address);
	
	// Start in the middle of th board, and reveal outwards:
	// 4,5 -> 3,6 -> 2,7 -> 1,8 -> 0,9
	signed char ix = 4;
	unsigned char iy;

	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.
	delay(1);
	clear_vram_buffer();	

	// Reveal from the center out.
	for (; ix >= 0; --ix)
	{
		// LEFT SIDE

		// copy a column into an array.
		for (iy = 0; iy < BOARD_HEIGHT; ++iy)
		{
			copy_board_data[iy] = game_board[TILE_TO_BOARD_INDEX(ix, iy + BOARD_OOB_END + 1)];
		}

		multi_vram_buffer_vert(
			copy_board_data, 
			BOARD_HEIGHT, 
			get_ppu_addr(
				cur_nt, 
				BOARD_START_X_PX + (ix << 3), 
				BOARD_START_Y_PX + ((BOARD_OOB_END + 1) << 3)));

		
		// RIGHT SIDE


		for (iy = 0; iy < BOARD_HEIGHT; ++iy)
		{
			copy_board_data[iy] = game_board[TILE_TO_BOARD_INDEX(BOARD_END_X_PX_BOARD - ix, iy + BOARD_OOB_END + 1)];
		}

		multi_vram_buffer_vert(
			copy_board_data, 
			BOARD_HEIGHT, 
			get_ppu_addr(
				cur_nt, 
				BOARD_START_X_PX + ((BOARD_END_X_PX_BOARD - ix) << 3), 
				BOARD_START_Y_PX + ((BOARD_OOB_END + 1) << 3)));				

		// Reveal these 2 new columns, and then move to the next one.
		delay(5);
		clear_vram_buffer();				
	}

	// Move on to the next phase...
	try_collapse_empty_row_data();
}

void try_collapse_empty_row_data(void)
{
	unsigned char iy;
	signed char i;

	// Start at the bottom of the board, and work our way up.
	for (i = 3; i >= 0; --i)
	{
		// Collapse the game board by copying the top of the board down to above
		// where the line was cleared, to 1 line below the top of the board.

		iy = lines_cleared_y[i];
		if (iy != 0xff)
		{
			// We need to make a copy of game_board, because memcpy can not copy over itself.
			// memmove would be function to use, but it does not exist in this library.
			memcpy(game_board_temp, game_board, sizeof(game_board));
			// index 10 is the start of the 2nd row.
			memcpy(&game_board[10], game_board_temp, iy * 10);
		}
	}


	// TODO NEXT: Chunky update nt.
	copy_board_to_nt();


}

void copy_board_to_nt()
{
	unsigned char ix;
	unsigned char iy;

	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.

	delay(1);
	clear_vram_buffer();	

	for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
	{
		// copy a column into an array.
		for (iy = 0; iy < BOARD_HEIGHT; ++iy)
		{
			copy_board_data[iy] = game_board[TILE_TO_BOARD_INDEX(ix, iy + BOARD_OOB_END + 1)];
		}

		multi_vram_buffer_vert(
			copy_board_data, 
			BOARD_HEIGHT, 
			get_ppu_addr(
				cur_nt, 
				BOARD_START_X_PX + (ix << 3), 
				BOARD_START_Y_PX + ((BOARD_OOB_END + 1) << 3)));

		// delay often enough to avoid buffer overrun.
		if (ix % 4 == 0)
		{
			delay(1);
			clear_vram_buffer();				
		}
	}
}

// DEBUG
#if DEBUG_ENABLED
void debug_fill_nametables(void)
{
	vram_adr(NTADR_A(0,0));
	vram_fill('a', NAMETABLE_PATTERN_SIZE);
	vram_adr(NTADR_B(0,0));
	vram_fill('b', NAMETABLE_PATTERN_SIZE);
	vram_adr(NTADR_B(0,0));
	vram_fill('c', NAMETABLE_PATTERN_SIZE);
	vram_adr(NTADR_D(0,0));
	vram_fill('d', NAMETABLE_PATTERN_SIZE);
}

void debug_draw_board_area(void)
{
	oam_spr(BOARD_START_X_PX, BOARD_START_Y_PX, 0x01, 0);
	oam_spr(BOARD_END_X_PX, BOARD_START_Y_PX, 0x01, 0);
	oam_spr(BOARD_START_X_PX, BOARD_END_Y_PX, 0x01, 0);
	oam_spr(BOARD_END_X_PX, BOARD_END_Y_PX, 0x01, 0);
}

void debug_copy_board_data_to_nt(void)
{
	//multi_vram_buffer_vert(const char * data, unsigned char len, int ppu_address);
	
	unsigned char ix;
	unsigned char iy;

	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.

	delay(1);
	clear_vram_buffer();	

	for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
	{
		// copy a column into an array.
		for (iy = 0; iy < BOARD_HEIGHT; ++iy)
		{
			copy_board_data[iy] = '0' + game_board[TILE_TO_BOARD_INDEX(ix, iy + BOARD_OOB_END + 1)];
		}

		multi_vram_buffer_vert(
			copy_board_data, 
			BOARD_HEIGHT, 
			get_ppu_addr(
				cur_nt, 
				BOARD_START_X_PX + (ix << 3), 
				BOARD_START_Y_PX + ((BOARD_OOB_END + 1) << 3)));

		// delay often enough to avoid buffer overrun.
		if (ix % 4 == 0)
		{
			delay(1);
			clear_vram_buffer();				
		}
	}
}

void debug_display_number(unsigned char num, unsigned char index)
{

	char arr[3] = { 0, 0, 0 };
	if (num > 100)
	{
		arr[2] = '0' + num % 10;
		num /= 10;
	}
	if (num > 10)
	{
		arr[1] = '0' + num % 10;
		num /= 10;
	}
	arr[0] = '0' + num % 10;

	multi_vram_buffer_horz(arr, 3, get_ppu_addr(cur_nt, 0, 232 - (index << 3)));
	delay(1);
	clear_vram_buffer();
}
#endif //DEBUG_ENABLED