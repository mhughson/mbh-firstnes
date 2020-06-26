/** (C) Matt Hughson 2020 */

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "Sprites.h" // holds our metasprite data
#include "BG/game_area.h"
#include "BG/title_screen.h"
#include "BG/options_screen.h"
#include "main.h"

/*

..::TODO::..

FEATURES:

* Sound on hit tentacle.
* Option to return to main menu on game over.
* Game over screen (polished).

* Number of rows that hit the tentacle adds a delay to next attack.
* See if tentacles can be made to work with name tables.
* Pal swap based on time of day/night.
* Score.
* Multiple tentacles.
* Fast music when tentacle is maxed out.
* Hard drop trails
* Store blocks.

COMPLETE:

* Hard drop (up on d-pad).
* Option screen.
* Credits screen.
* Time based tentacle movement (rather than on landing).
* Option to choose between Kraken and Klassic gameplay. (classic may require score, ability to go back to main menu).
* Push start flashing text.

CUT:
* Last chance move on hard drop (maybe optional).
	* Feels weird. See commented out code in movement().

--

BUGS:

* Sprites do not draw when transitioning between name tables.
* Sprite flicker when blocks land.
* Horz input has to be pressed again if line is cleared.
* When hitting game over, final sprite switches.

COMPLETE:

* Tentacles are not budgeted.
* Graphical corruption on Game Over (rarely)
* Hard drop puts blocks 1 tile too far (rarely).

*/

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
	
	// TODO: This is actually the gameplay setup.
	off_nt = 0;
	cur_nt = 2;

	vram_adr(NTADR_A(0,0));
	vram_unrle(title_screen);
	vram_adr(NTADR_C(0,0));
	vram_unrle(game_area);
	
	scroll(0, 0x1df); // shift the bg down 1 pixel
	//set_scroll_y(0xff);
	
	ppu_on_all(); // turn on screen

	//music_play(0);

	attack_style = ATTACK_ON_LAND;
	music_on = 1;

	go_to_state(STATE_MENU);

	// infinite loop
	while (1)
	{
		ppu_wait_nmi(); // wait till beginning of the frame

		//set_music_speed(1);

		++tick_count;
		++tick_count_large;
		
		pad1 = pad_poll(0); // read the first controller
		pad1_new = get_pad_new(0); // newly pressed button. do pad_poll first
		
		clear_vram_buffer(); // do at the beginning of each frame

		switch(state)
		{
			case STATE_MENU:
			{
				draw_menu_sprites();			

				if (tick_count % 128 == 0)
				{
					multi_vram_buffer_horz(text_push_start, sizeof(text_push_start)-1, get_ppu_addr(0, 12<<3, 12<<3));
				}
				else if (tick_count % 128 == 96)
				{
					multi_vram_buffer_horz(clear_push_start, sizeof(clear_push_start)-1, get_ppu_addr(0, 12<<3, 12<<3));
				}

				if (pad1_new & PAD_START)
				{
					seed_rng();

					if (pad1 & PAD_A)
					{
						music_stop();
						go_to_state(STATE_SOUND_TEST);
					}
					else
					{
						music_stop();
						sfx_play(SOUND_START, 0);

						delay(4 * 0x18);

						go_to_state(STATE_GAME);
					}
				}
				else if (pad1_new & PAD_SELECT)
				{
					fade_to_black();
					go_to_state(STATE_OPTIONS);
					fade_from_black();
				}
				break;
			}

			case STATE_OPTIONS:
			{
				if (pad1_new & PAD_B)
				{
					fade_to_black();
					go_to_state(STATE_MENU);
					fade_from_black();
				}
				else if (pad1_new & PAD_RIGHT)
				{
					switch (cur_option)
					{
					
					case 0: // Attack style

						//attack_style = (attack_style + 1) % ATTACK_NUM;

						if (attack_style < ATTACK_NUM - 1)
						{
							++attack_style;
						}
						break;

					case 1: // Music off/on

						//music_on = (music_on + 1) % 2;

						if (music_on == 0)
						{
							music_on = 1;
							music_play(MUSIC_TITLE);
						}

						// if (music_on == 0)
						// {
						// 	music_stop();
						// }
						// else
						// {
						// 	music_play(MUSIC_TITLE);
						// }
						break;

					default:
						break;
					}

					sfx_play(SOUND_MENU_HIGH, 0);
					display_options();
				}
				else if (pad1_new & PAD_LEFT)
				{
					switch (cur_option)
					{
					
					case 0: // Attack style
						// if (attack_style == 0)
						// {
						// 	attack_style = ATTACK_NUM;
						// }
						// attack_style = (attack_style - 1) % ATTACK_NUM;

						if (attack_style != 0)
						{
							--attack_style;
						}

						break;

					case 1: // Music off/on
						// if (music_on == 0)
						// {
						// 	music_on = 2;
						// }
						// music_on = (music_on - 1) % 2;

						if (music_on != 0)
						{
							music_on = 0;
							music_stop();
						}

						// if (music_on == 0)
						// {
						// 	music_stop();
						// }
						// else
						// {
						// 	music_play(MUSIC_TITLE);
						// }

						break;

					default:
						break;
					}

					sfx_play(SOUND_MENU_LOW, 0);
					display_options();
				}
				else if (pad1_new & PAD_DOWN)
				{
					cur_option = (cur_option + 1) % NUM_OPTIONS;
					sfx_play(SOUND_MENU_LOW, 0);
					display_options();
				}
				else if (pad1_new & PAD_UP)
				{
					if (cur_option == 0)
					{
						cur_option = NUM_OPTIONS;
					}
					cur_option = (cur_option - 1) % NUM_OPTIONS;
					sfx_play(SOUND_MENU_HIGH, 0);
					display_options();
				}
				break;
			}

			case STATE_GAME:
			{
				if (hit_reaction_remaining > 0)
				{
					--hit_reaction_remaining;
				}

				
				// delay a frame for perf.
				if (attack_queued)
				{
					// TODO: Perf - Very expensive.
					add_block_at_bottom();
					
					clear_rows_in_data(BOARD_END_Y_PX_BOARD);
					attack_queued = 0;
				}

//POKE(0x2001,0x9f); //blue
				movement();
//POKE(0x2001,0x3f); // red
				draw_gameplay_sprites();
//POKE(0x2001,0x1e); // white

//POKE(0x2001,0x9f); // blue
				if (attack_style == ATTACK_ON_TIME && attack_queue_ticks_remaining != 0)
				{
					--attack_queue_ticks_remaining;

					if (attack_queue_ticks_remaining == 0)
					{
						attack_queued = 1;
						attack_queue_ticks_remaining = attack_delay;
					}
				}

				if (pad1_new & PAD_START)
				{
					go_to_state(STATE_PAUSE);
				}

#if DEBUG_ENABLED
				// if (pad1_new & PAD_START)
				// {

				// 	memcpy(temp_pal, palette_bg, sizeof(palette_bg));
				// 	pal_id += 2;
				// 	pal_id = pal_id % 20;
				// 	temp_pal[1] = pal_changes[pal_id];
				// 	temp_pal[2] = pal_changes[pal_id + 1];
				// 	pal_bg(temp_pal);
				// 	debug_display_number(pal_id >> 1, 0);

				// 	//debug_copy_board_data_to_nt();
				// 	//go_to_state(STATE_OVER);
				// }
#endif
				//POKE(0x2001,0x1e);//0x3E);
				break;
			}

			case STATE_PAUSE:
			{
				oam_clear();

				// if ((tick_count % 60) < 30)
				// {
				// 	draw_sprites();
				// }
				// else
				// {
				// 	oam_clear();
				// }
				

				if (pad1_new & PAD_START)
				{
					go_to_state(STATE_GAME);
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

			case STATE_SOUND_TEST:
			{
				// MUSIC
				//

				if (pad1_new & PAD_DOWN && test_song < 15)
				{
					++test_song;
					display_song();
				}
				else if (pad1_new & PAD_UP && test_song > 0)
				{
					--test_song;
					display_song();
				}

				if (pad1_new & PAD_B)
				{
					if (test_song == test_song_active)
					{
						test_song_active = 0xff;
						music_stop();
					}
					else
					{
						test_song_active = test_song;
						music_play(test_song);
					}
				}

				
				// SOUND
				//

				if (pad1_new & PAD_RIGHT && test_sound < 31)
				{
					++test_sound;
					display_sound();
				}
				else if (pad1_new & PAD_LEFT && test_sound > 0)
				{
					--test_sound;
					display_sound();
				}

				if (pad1_new & PAD_A)
				{
					//sfx_play(unsigned char sound,unsigned char channel);
					sfx_play(test_sound, 0);
				}
				break;
			}
		}
	}
}

void draw_menu_sprites(void)
{
	static unsigned char ix;
	static unsigned int t;

	// clear all sprites from sprite buffer
	oam_clear();

	// FLAGS
	t = tick_count_large % 60;
	if (t > 45)
	{
		ix = 0x69;
	}
	else if (t > 30)
	{
		ix = 0x68;
	}
	else if (t > 15)
	{
		ix = 0x67;
	}
	else
	{
		ix = 0x66;
	}

	oam_spr(10 << 3, 23 << 3, ix, 0);
	oam_spr(22 << 3, 23 << 3, ix, 0);

	// TENTACLES
	oam_spr(19 << 3, 14 << 3, 0x60, 1);
	oam_spr(20 << 3, 14 << 3, 0x61, 1);

	oam_spr(19 << 3, 15 << 3, 0x70, 1);
	oam_spr(20 << 3, 15 << 3, 0x71, 1);

	oam_spr(19 << 3, 16 << 3, 0x80, 1);
	oam_spr(20 << 3, 16 << 3, 0x81, 1);

	oam_spr(19 << 3, 17 << 3, 0x90, 1);
	oam_spr(20 << 3, 17 << 3, 0x91, 1);
}

void draw_gameplay_sprites(void)
{
	static unsigned int mask;
//POKE(0x2001,0x5f); // green
	// clear all sprites from sprite buffer
	oam_clear();
//POKE(0x2001,0x9f); // blue
	// push a single sprite
	// oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned char attr);
	// use tile #0, palette #0

	local_start_x = (cur_block.x << 3) + BOARD_START_X_PX;
	local_start_y = (cur_block.y << 3) + BOARD_START_Y_PX;

	local_iy = 0;
	local_ix = 0;
	for (mask = 0x8000; mask; mask >>= 1)
	{
		// 255 means hide.
		if (cur_block.y != 255)
		{
			if (cur_cluster.layout & mask)
			{
				// Don't draw the current cluster if it is above the top of the board.
				// We want it to be able to function and move up there, but should not
				// be visible.
				if (local_start_y + (local_iy << 3) > OOB_TOP)
				{
					oam_spr(local_start_x + (local_ix << 3), local_start_y + (local_iy << 3), cur_cluster.sprite, 0);
				}
			}
		}

		if (next_cluster.layout & mask)
		{
			oam_spr((15 << 3) + (local_ix << 3), (1 << 3) + (local_iy << 3), next_cluster.sprite, 0);
		}

		++local_ix;
		if (local_ix >= 4)
		{
			local_ix = 0;
			++local_iy;
		}
	}

//POKE(0x2001,0x1f); // white
	// Loop through the attack columns and draw the off board portion as sprites.
	for (local_ix = 0; local_ix < BOARD_WIDTH; ++local_ix)
	{
		local_row_status = attack_row_status[local_ix];
		if (local_row_status > 0)
		{
			for (local_iy = 0; local_iy < local_row_status /*&& local_iy < ATTACK_QUEUE_SIZE*/; ++local_iy)
			{
				// gross. Try to detect if this is the last piece, and also the end of the arm.
				if (local_iy == local_row_status - 1)
				{
				oam_spr(
					BOARD_START_X_PX + (local_ix << 3), 
					(BOARD_END_Y_PX) + (ATTACK_QUEUE_SIZE << 3) - (local_iy << 3),
					0xf9, 
					1);
				}
				else
				{
				oam_spr(
					BOARD_START_X_PX + (local_ix << 3), 
					(BOARD_END_Y_PX) + (ATTACK_QUEUE_SIZE << 3) - (local_iy << 3),
					0xf8, 
					1);
				}
				
			}
		}
	}

//POKE(0x2001,0x3f); // red

	// HIT REACTION
	if (hit_reaction_remaining > 0)
	{
		// -1, 0, 1
		//r = (rand8() % 3) - 1;
		oam_spr((3 << 3) /*+ r*/, (24 << 3), 0x65, 1);
		oam_spr(3 << 3, 25 << 3, 0x64, 1);
		oam_spr(3 << 3, 26 << 3, 0x74, 1);

		// memcpy(temp_pal, palette_sp, sizeof(palette_sp));
		// // blocks
		// //temp_pal[4] = 0x15;
		// //temp_pal[5] = 0x15;
		// temp_pal[6] = 0x16;
		// //temp_pal[7] = 0x16;
		// pal_spr(temp_pal);		

		// memcpy(temp_pal, palette_bg, sizeof(palette_bg));
		// // blocks
		// //temp_pal[12] = 0x15;
		// //temp_pal[13] = 0x15;
		// temp_pal[14] = 0x16;
		// //temp_pal[15] = 0x16;
		// pal_bg(temp_pal);		
	}
	// BLINKING
	else
	{
		//pal_spr(palette_sp);
		//pal_bg(palette_bg);
		local_t = tick_count_large % BLINK_LEN;

		if (local_t > BLINK_LEN - 5)
		{
			oam_spr(3 << 3, 25 << 3, 0x62, 1);
			oam_spr(3 << 3, 26 << 3, 0x72, 1);
		}
		else if (local_t > (BLINK_LEN - 10))
		{
			oam_spr(3 << 3, 25 << 3, 0x63, 1);
			oam_spr(3 << 3, 26 << 3, 0x73, 1);
		}
		else if (local_t > BLINK_LEN - 15)
		{
			oam_spr(3 << 3, 25 << 3, 0x62, 1);
			oam_spr(3 << 3, 26 << 3, 0x72, 1);
		}
	}

	// FLAGS
	local_t = tick_count & 63;
	if (local_t > 48)
	{
		local_ix = 0x69;
	}
	else if (local_t > 32)
	{
		local_ix = 0x68;
	}
	else if (local_t > 16)
	{
		local_ix = 0x67;
	}
	else
	{
		local_ix = 0x66;
	}

	oam_spr(8 << 3, 1 << 3, local_ix, 2);
	oam_spr(24 << 3, 1 << 3, local_ix, 2);
	oam_spr(3 << 3, 10 << 3, local_ix, 0);
	oam_spr(27 << 3, 10 << 3, local_ix, 0);

	//debug_draw_board_area();
}

void movement(void)
{
	hit = 0;
	temp_fall_rate = 0;
	old_x = 0;

	++fall_frame_counter;

	if (pad1_new & PAD_SELECT)
	{
		//hit_reaction_remaining = 60;
		// inc_lines_cleared();
		// inc_lines_cleared();
		// inc_lines_cleared();
		// inc_lines_cleared();
		// inc_lines_cleared();
		// inc_lines_cleared();
		// inc_lines_cleared();
		// inc_lines_cleared();
		// inc_lines_cleared();
		// inc_lines_cleared();
		add_block_at_bottom();
		//spawn_new_cluster();
	}

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
	if (pad1_new & PAD_UP)
	{
		// TODO: Causes hitch.
		while (!is_cluster_colliding())
		{
			++cur_block.y;
		}

		// UNCOMMENT FOR LAST CHANCE MOVE ON HARD DROP
		// cur_block.y -= 1;
		// fall_frame_counter = 1;
	}
	else
	{
		// Hard drop skips all this to avoid dropping to the bottom
		// and then dropping again because it happens to be
		// the natural fall frame.
		if (pad1_new & PAD_DOWN)
		{
			require_new_down_button = 0;

			// fall this frame.
			temp_fall_rate = fall_frame_counter;
		}
		else if ((pad1 & PAD_DOWN) && require_new_down_button == 0)
		{
			// fall every other frame.
			temp_fall_rate = MIN(temp_fall_rate, 2);
		}

		if (fall_frame_counter % temp_fall_rate == 0 || temp_fall_rate == 0)
		{
			cur_block.y += 1;
		}
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

void set_block(/*unsigned char x, unsigned char y, unsigned char id*/)
{
	static int address;

	// w = 10 tiles,  80 px
	// h = 20 tiles, 160 px

	// Update the logic array as well as the nametable to reflect it.

	if (in_y <= BOARD_OOB_END)
	{
		// Don't place stuff out of bounds.
		return;
	}

	address = get_ppu_addr(cur_nt, (in_x << 3) + BOARD_START_X_PX, (in_y << 3) + BOARD_START_Y_PX);
	one_vram_buffer(in_id, address);

	// TODO: Is this too slow?
	game_board[TILE_TO_BOARD_INDEX(in_x, in_y)] = in_id;	
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
	in_x = x;
	in_y = y;
	in_id = 0;
	set_block();
}

void put_cur_cluster()
{
	static unsigned char ix;
	static unsigned char iy;
	static unsigned int bit;
	static unsigned int res;

	max_y = 0;
	min_y = 0xff; // max

	ix = 0;
	iy = 0;
	for (bit = 0x8000; bit; bit >>= 1)
	{		
		res = cur_cluster.layout & bit;
	
		// solid bit.
		if (res)
		{

			in_x = cur_block.x + ix;
			in_y = cur_block.y + iy;
			in_id = cur_cluster.sprite;

			// This is basically always going to be the first thing drawn,
			// but i couldn't think of a clever way to do this once.
			if (in_y < min_y)
			{
				min_y = in_y;
			}
			if (in_y > max_y)
			{
				max_y = in_y;
			}

			set_block( );	
		}

		++ix;
		if (ix >= 4)
		{
			ix = 0;
			++iy;
		}
	}

	sfx_play(SOUND_LAND, 0);

	if (min_y <= BOARD_OOB_END)
	{
		go_to_state(STATE_OVER);
		return;
	}
	else
	{
		// hide the sprite while we work.
		cur_block.y = 255;
		// draw sprites again so newly hidden sprite vanishes. expensive for something
		// so small.
		draw_gameplay_sprites();

		if (attack_style == ATTACK_ON_LAND)
		{
			attack_queued = 1;
		}
		
		clear_rows_in_data(max_y);
		
	}
	
}

unsigned char is_block_free(unsigned char x, unsigned char y)
{
	if (y > BOARD_END_Y_PX_BOARD || x > BOARD_END_X_PX_BOARD)
	{
		// consider this blocked.
		return 0;
	}

	//return get_block(x, y) == 0;
	return game_board[TILE_TO_BOARD_INDEX(x,y)] == 0;
}

unsigned char is_cluster_colliding()
{
	static unsigned char ix;
	static unsigned char iy;
	static unsigned int bit;

	static unsigned char x;
	static unsigned char y;

	ix = 0;
	iy = 0;
	for (bit = 0x8000; bit; bit >>= 1)
	{	
		// solid bit.
		if (cur_cluster.layout & bit)
		{
			
			x = cur_block.x + ix;
			y = cur_block.y + iy;

			if (y > BOARD_END_Y_PX_BOARD || x > BOARD_END_X_PX_BOARD)
			{
				// consider this blocked.
				return 1;
			}

			//return get_block(x, y) == 0;
			if(game_board[TILE_TO_BOARD_INDEX(x,y)])
			{ 
				return 1;
			}
		}

		++ix;
		if (ix >= 4)
		{
			ix = 0;
			++iy;
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
	memcpy(cur_cluster.def, next_cluster.def, 4 * 2);
	//cur_cluster.def = next_cluster.def;
	cur_cluster.layout = cur_cluster.def[0];
	cur_cluster.sprite = next_cluster.sprite;
	cur_cluster.id = next_cluster.id;

	// Reset the block.
	cur_block.x = 3; //(BOARD_END_Y_PX_BOARD >> 1);
	cur_block.y = cluster_offsets[cur_cluster.id];

	// If the block is colliding right out of the game, move it up so that
	// we get a cleaner game over.
	// REMOVED FOR PERF
	// if (is_cluster_colliding())
	// {
	// 	--cur_block.y;
	// }

	// By checking twice we go from 1 in 7 chance of a dupe to
	// 1 in 49 chance.
	id = rand8() % NUM_CLUSTERS;
	if (id == cur_cluster.id)
	{
		id = rand8() % NUM_CLUSTERS;
	}
	next_cluster.id = id;
	memcpy(next_cluster.def, cluster_defs[id], 4 * 2);
	//next_cluster.def = cluster_defs[id]; // def_z_rev_clust;
	next_cluster.layout = next_cluster.def[0];
	next_cluster.sprite = cluster_sprites[id];
}

void rotate_cur_cluster(char dir)
{
	static unsigned char old_rot;

	old_rot = cur_rot;

	cur_rot = (cur_rot + dir) & 3; // % 4
	cur_cluster.layout = cur_cluster.def[cur_rot];

	// if after rotating, we are now colliding we something, revert the rotation.
	if (is_cluster_colliding())
	{
		cur_rot = old_rot;
		cur_cluster.layout = cur_cluster.def[cur_rot];
		sfx_play(SOUND_BLOCKED, 0);
	}
	else
	{
		sfx_play(SOUND_ROTATE, 0);
	}
	
}

void go_to_state(unsigned char new_state)
{
	static int address;
	static unsigned char i;
	static unsigned char fade_from_bright;
	static unsigned char fade_delay;
	static unsigned char prev_state;

	fade_from_bright = 0;
	fade_delay = 5;
	prev_state = state;

	switch (state)
	{
		case STATE_OVER:
		{
			// We would have faded up to white entering game over, so now
			// we need to fade back down.
			fade_from_bright = 1;
			break;
		}

		case STATE_OPTIONS:
		{
			pal_bg(palette_bg);
			break;
		}

		case STATE_PAUSE:
		{
			pal_bright(4);
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
			if (prev_state == STATE_OPTIONS)
			{
				oam_clear();

				ppu_off();
				vram_adr(NTADR_A(0,0));
				vram_unrle(title_screen);
				ppu_on_all();
			}
			else
			{
				if (music_on)
				{
					music_play(MUSIC_TITLE);
				}
			}
			
			break;
		}

		case STATE_OPTIONS:
		{
			oam_clear();

			ppu_off();

			pal_bg(palette_bg_options);

			//go_to_state(STATE_SOUND_TEST);
			vram_adr(NTADR_A(0,0));
			vram_unrle(options_screen);

			// vram_adr(NTADR_A(16,19));
			// vram_write(attack_style_strings[attack_style], ATTACK_STRING_LEN);

			// vram_adr(NTADR_A(16,21));
			// vram_write(off_on_string[music_on], OFF_ON_STRING_LEN);

			ppu_on_all();

			cur_option = 0;

			display_options();

			break;
		}

		case STATE_SOUND_TEST:
		{
			ppu_off(); // screen off
			vram_adr(NTADR_A(0,0));
			vram_fill(0, NAMETABLE_SIZE);

			i = 4;
			vram_adr(NTADR_A(16-(sizeof("SOUND TEST")>>1),i));
			vram_write("SOUND TEST", sizeof("SOUND TEST")-1); // -1 null term

			i += 2;
			vram_adr(NTADR_A(16-(sizeof("B - PLAY/STOP SONG")>>1),i));
			vram_write("B - PLAY/STOP SONG", sizeof("B - PLAY/STOP SONG")-1); // -1 null term

			++i;
			vram_adr(NTADR_A(16-(sizeof("A - PLAY SFX")>>1),i));
			vram_write("A - PLAY SFX", sizeof("A - PLAY SFX")-1); // -1 null term

			i += 4;
			vram_adr(NTADR_A(8-(sizeof("SONG #")>>1),i));
			vram_write("SONG #", sizeof("SONG #")-1); // -1 null term

			vram_adr(NTADR_A(24-(sizeof("SFX #")>>1),i));
			vram_write("SFX #", sizeof("SFX #")-1); // -1 null term

			i += 10;
			vram_adr(NTADR_A(8-(sizeof("UP/DOWN")>>1),i));
			vram_write("UP/DOWN", sizeof("UP/DOWN")-1); // -1 null term

			vram_adr(NTADR_A(24-(sizeof("LEFT/RIGHT")>>1),i));
			vram_write("LEFT/RIGHT", sizeof("LEFT/RIGHT")-1); // -1 null term			

			ppu_on_all(); // turn on screen

			test_song = test_sound = 0;
			test_song_active = 0xff;
			
			display_song();
			display_sound();

			break;
		}

		case STATE_GAME:
		{
			if (music_on)
			{
				music_play(MUSIC_GAMEPLAY);
			}

			if (prev_state != STATE_PAUSE)
			{
				//ppu_off(); // screen off

				// clear the nametable and attributes.
				//vram_adr(NTADR_A(0,0));
				//vram_fill(0, NAMETABLE_SIZE);
				//vram_adr(NTADR_A(0,0));
				//vram_unrle(game_area);

				// TODO: Use get_ppu functions with nt id.
				//vram_adr(NTADR_C(0,0));
				//vram_unrle(game_area);

				//ppu_on_all(); // turn on screen

				// for (iy = 0; iy <= BOARD_END_Y_PX_BOARD; ++iy)
				// {
				// 	vram_adr(NTADR_A(BOARD_START_X_PX >> 3, (BOARD_START_Y_PX >> 3) + iy));
				// 	vram_fill(0, 10);
				// }

				memfill(game_board, 0, BOARD_SIZE);

				// Reset stats.
				lines_cleared_one = lines_cleared_ten = lines_cleared_hundred = cur_level = 0;
				fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];
				
				// load the palettes
				pal_bg(palette_bg);
				pal_spr(palette_sp);

				//cur_level = 99;
				//fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];

				// shift up 1
				//scroll(0, 255 - 16);

				display_lines_cleared();
				display_level();
#if DEBUG_ENABLED
				// leave a spot open.
				// for (i=0; i < BOARD_END_X_PX_BOARD; ++i)
				// {
				// 	set_block(i, BOARD_END_Y_PX_BOARD, 1);
				// 	set_block(i, BOARD_END_Y_PX_BOARD - 1, 1);
				// 	set_block(i, BOARD_END_Y_PX_BOARD - 2, 1);
				// 	set_block(i, BOARD_END_Y_PX_BOARD - 3, 1);
				// 	delay(1);
				// 	clear_vram_buffer();
				// 	set_block(i, BOARD_END_Y_PX_BOARD - 4, 1);
				// 	set_block(i, BOARD_END_Y_PX_BOARD - 5, 1);
				// 	set_block(i, BOARD_END_Y_PX_BOARD - 6, 1);
				// 	set_block(i, BOARD_END_Y_PX_BOARD - 7, 1);
				// 	delay(1);
				// 	clear_vram_buffer();
				// }
#endif //DEBUG_ENABLED
				//debug_display_number(123, 0);
				//debug_display_number(45, 1);
				//debug_display_number(6, 2);

				oam_clear();
				while (scroll_y < 240)				
				{
					scroll(0, scroll_y);
					delay(1);
					scroll_y += 4;
				}
				scroll(0, 239);

				// Spawn "next"
				spawn_new_cluster();
				// "Next" becomes current, and a new next is defined.
				spawn_new_cluster();

				memfill(attack_row_status, 0, BOARD_WIDTH);

				// where to start the attack!
				i = rand8() % BOARD_WIDTH;
				attack_row_status[i] = 1;

				// Reset the ppu for gameover case.
				copy_board_to_nt();

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
				if (attack_style == ATTACK_ON_TIME)
				{
					attack_queue_ticks_remaining = attack_delay;
				}
			}

			break;
		}

		case STATE_PAUSE:
		{
			pal_bright(2);
			if (music_on)
			{
				music_play(MUSIC_PAUSE);
			}
			break;
		}
		case STATE_OVER:
		{
			// fix bug where mashing up/down to quickly hit gamover would case nametable coruption.
			clear_vram_buffer();

			music_stop();
			sfx_play(SOUND_GAMEOVER, 0);

			delay(120);

			sfx_play(SOUND_GAMEOVER_SONG, 0);

			oam_clear();

			pal_bright(5);
			delay(fade_delay);
			pal_bright(6);
			delay(fade_delay);
			pal_bright(7);
			delay(fade_delay);
			pal_bright(8);
			delay(fade_delay);

			for (i = 0; i < 3; ++i)
			{
				address = get_ppu_addr(cur_nt, BOARD_START_X_PX, (14+i)<<3);
				multi_vram_buffer_horz(empty_row, 10, address);
			}

			address = get_ppu_addr(cur_nt, 96, 15<<3);
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
	static unsigned char pal_id;

	++lines_cleared_one;

	if (lines_cleared_one == 10)
	{
		++cur_level;

		fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];

		memcpy(temp_pal, palette_bg, sizeof(palette_bg));
		pal_id = (cur_level % 10) << 1; // array is pairs of 2
		// blocks
		temp_pal[1] = pal_changes[pal_id];
		temp_pal[2] = pal_changes[pal_id + 1];
		// kraken
		temp_pal[13] = pal_changes[pal_id];
		//temp_pal[14] = pal_changes[pal_id + 1];
		pal_bg(temp_pal);

		memcpy(temp_pal, palette_sp, sizeof(palette_sp));
		// blocks
		temp_pal[1] = pal_changes[pal_id];
		temp_pal[2] = pal_changes[pal_id + 1];
		// flag bg
		temp_pal[10] = pal_changes[pal_id + 1];
		//temp_pal[14] = pal_changes[pal_id + 1];
		pal_spr(temp_pal);		
		
#if DEBUG_ENABLED
		//debug_display_number(fall_rate, 0);
#endif //DEBUG_ENABLED
		display_level();

		lines_cleared_one = 0;
		++lines_cleared_ten;
		if (lines_cleared_ten == 10)
		{
			lines_cleared_ten = 0;
			++lines_cleared_hundred;
		}
	}
	display_lines_cleared();
}

void display_lines_cleared()
{
	one_vram_buffer('0' + lines_cleared_hundred, get_ppu_addr(cur_nt,3<<3,2<<3));
	one_vram_buffer('0' + lines_cleared_ten, get_ppu_addr(cur_nt,4<<3,2<<3));
	one_vram_buffer('0' + lines_cleared_one, get_ppu_addr(cur_nt,5<<3,2<<3));
}

void display_level()
{
	// We let level be displayed as zero based because it makes more sense when
	// comparing it to lines (eg. lines is 80, level is 8).
	static unsigned char temp_level;
	static unsigned char i;

	temp_level = cur_level;
	i = 0;

	if (cur_level < 100)
	{
		multi_vram_buffer_horz("000", 3, get_ppu_addr(cur_nt,28<<3,2<<3));
	}

	while(temp_level != 0)
    {
        unsigned char digit = temp_level % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(cur_nt, (30<<3) - (i << 3), 2<<3 ));

        temp_level = temp_level / 10;
		++i;
    }
}

// START OF ROW CLEAR SEQUENCE!

void clear_rows_in_data(unsigned char start_y)
{
	static unsigned char ix;
	static unsigned char iy;
	static unsigned char line_complete;
	static unsigned char i;
	static unsigned char prev_level;

	i = 0;
	prev_level = cur_level;

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
			// for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
			// {
			// 	// search for attacks to clear.
			// 	z = get_block(ix, iy);

			// 	if (z == 0xf9 || z == 0xf8 || z == 0xf7)
			// 	{
			// 		attack_row_status[ix] = 0;
			// 	}
			// }

			// early detection for hit, so that we can play hit reaction during clear animation.
			for (ix = 0; ix < BOARD_WIDTH; ++ix)
			{
				if (attack_row_status[ix] > ATTACK_QUEUE_SIZE && attack_row_status[ix] - (ATTACK_QUEUE_SIZE+1) >= (BOARD_END_Y_PX_BOARD - iy))
				{
					hit_reaction_remaining = 60;
					//debug_display_number(hit_reaction_remaining, 1);
				}
			}

			inc_lines_cleared();

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
		if (prev_level != cur_level)
		{
			sfx_play(SOUND_LEVELUP, 0);
		}
		else if (i == 4)
		{
			sfx_play(SOUND_MULTIROW, 0);
		}
		else
		{
			sfx_play(SOUND_ROW, 0);
		}
		
		// potential hit reaction.
		if (hit_reaction_remaining > 0)
		{
			draw_gameplay_sprites();
		}
		reveal_empty_rows_to_nt();
	}
}

void reveal_empty_rows_to_nt()
{
	//multi_vram_buffer_vert(const char * data, unsigned char len, int ppu_address);
	
	// Start in the middle of th board, and reveal outwards:
	// 4,5 -> 3,6 -> 2,7 -> 1,8 -> 0,9
	static signed char ix;
	static unsigned char iy;

	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.
	delay(1);
	clear_vram_buffer();	

	// Reveal from the center out.
	for (ix = 4; ix >= 0; --ix)
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
	static unsigned char ix;
	static unsigned char iy;
	static signed char i;

	// first one is the bottom.
	// TODO: Off by one?
	iy = BOARD_END_Y_PX_BOARD - lines_cleared_y[0];

	//debug_display_number(iy, 0);

	for (ix = 0; ix < BOARD_WIDTH; ++ix)
	{
		if (attack_row_status[ix] > ATTACK_QUEUE_SIZE && attack_row_status[ix] - (ATTACK_QUEUE_SIZE+1) >= (iy))
		{
			while (attack_row_status[ix] > 0)
			{
				//hit_reaction_remaining = 60;
				--attack_row_status[ix];
				delay(1);
				draw_gameplay_sprites();
				clear_vram_buffer();	
			}
			
		}
	}

	// Start at the top of the board, and work our way down.
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
	static unsigned char ix;
	static unsigned char iy;

	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.

	draw_gameplay_sprites();
	//delay(1);
	//clear_vram_buffer();
	//return;

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
			// calling this again here isn't needed, as time will not have advanced, so
			// drawing the sprites again will do nothing.
			//draw_gameplay_sprites();
			delay(1);
			clear_vram_buffer();				
		}
	}
}

void add_block_at_bottom()
{
	static signed char ix;
	static unsigned char iy;
	static unsigned char attacks;

	attacks = 0;

	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.
	//delay(1);
	//clear_vram_buffer();	

	for (ix = 0; ix < BOARD_WIDTH; ++ix)
	{
		if (attack_row_status[ix] > 0)
		{
			if (attack_row_status[ix] >= ATTACK_MAX)
			{
				// fake an attack so that we don't try to create a new tentacle.
				++attacks;
				break;
			}
			++attacks;
			++attack_row_status[ix];

			if (attack_row_status[ix] > ATTACK_QUEUE_SIZE)
			{
				for (iy = BOARD_END_Y_PX_BOARD; iy > BOARD_OOB_END; --iy)
				{
					// travel till we hit the first empty spot, which is where we will copy up to.
					if (game_board[TILE_TO_BOARD_INDEX(ix, iy)] == 0)
					{
						// Now work our way back down, copying upwards as we go.
						for (; iy <= BOARD_END_Y_PX_BOARD; ++iy)
						{
							game_board[TILE_TO_BOARD_INDEX(ix, iy)] = game_board[TILE_TO_BOARD_INDEX(ix, iy + 1)];
						}

						break;
					}
				}

				game_board[TILE_TO_BOARD_INDEX(ix, BOARD_END_Y_PX_BOARD)] = 0xf7; //(attack_row_status[ix] == (ATTACK_QUEUE_SIZE + 1)) ? 0xf9 : 0xf8;

				// stay at 1 larger than the queue size to avoid overrun.
				//attack_row_status[ix] = ATTACK_QUEUE_SIZE + 1;
			}
		}
	}

	// TODO: Compare to level expectations.
	if (attacks == 0)
	{
		// where to start the attack!
		attack_row_status[rand8() % BOARD_WIDTH] = 1;
	}

	// TODO: Only if changed above.
	copy_board_to_nt();
}

void display_song()
{
	static unsigned char temp;
	static unsigned char i;

	temp = test_song;
	i = 0;

	if (test_song < 100)
	{
		multi_vram_buffer_horz("000", 3, get_ppu_addr(0,56-(2<<3),112));
	}

	while(temp != 0)
    {
        unsigned char digit = temp % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(0, 56 - (i << 3), 112 ));

        temp = temp / 10;
		++i;
    }	
}

void display_sound()
{

	static unsigned char temp;
	static unsigned char i;

	temp = test_sound;
	i = 0;

	if (test_song < 100)
	{
		multi_vram_buffer_horz("000", 3, get_ppu_addr(0,200-(2<<3),112));
	}

	while(temp != 0)
    {
        unsigned char digit = temp % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(0, 200 - (i << 3), 112 ));

        temp = temp / 10;
		++i;
	}
}

void display_options()
{
	multi_vram_buffer_horz(attack_style_strings[attack_style], ATTACK_STRING_LEN, get_ppu_addr(0,17<<3,19<<3));
	multi_vram_buffer_horz(off_on_string[music_on], OFF_ON_STRING_LEN, get_ppu_addr(0,17<<3,21<<3));

	multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 8<<3, (19 + ((1-cur_option)<<1))<<3));
	multi_vram_buffer_horz(option_icon, 2, get_ppu_addr(0, 8<<3, (19 + (cur_option<<1))<<3));
}


void fade_to_black()
{
	pal_bright(3);
	delay(1);
	pal_bright(2);
	delay(1);
	pal_bright(1);
	delay(1);
	pal_bright(0);
	delay(1);
}

void fade_from_black()
{
	pal_bright(1);
	delay(1);
	pal_bright(2);
	delay(1);
	pal_bright(3);
	delay(1);
	pal_bright(4);
	delay(1);
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
	
	static unsigned char ix;
	static unsigned char iy;

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
	// We let level be displayed as zero based because it makes more sense when
	// comparing it to lines (eg. lines is 80, level is 8).
	static unsigned char temp;
	static unsigned char i;

	temp = num;
	i = 0;

	if (temp < 100)
	{
		multi_vram_buffer_horz("000", 3, get_ppu_addr(cur_nt,28<<3,232 - (index << 3)));
	}

	while(temp != 0)
    {
        unsigned char digit = temp % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(cur_nt, (30<<3) - (i << 3), 232 - (index << 3) ));

        temp = temp / 10;
		++i;
    }

	//multi_vram_buffer_horz(arr, 3, get_ppu_addr(cur_nt, 0, 232 - (index << 3)));
	delay(1);
	clear_vram_buffer();
}
#endif //DEBUG_ENABLED