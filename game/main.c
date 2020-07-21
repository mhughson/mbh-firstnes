/** (C) Matt Hughson 2020 */

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "BG/game_area.h"
#include "BG/title_screen.h"
#include "BG/options_screen.h"
#include "BG/boot_screen.h"
#include "BG/sound_screen.h"
#include "main.h"

/*

..::TODO::..

FEATURES:

//must have

//should have
* Lock-delay settings (off, 10 frames, 20 frames)
* Option to disable hard-drop (or require a slight hold to trigger hard drop.)
* Reset on A+B+SEL+START

//nice to have
* Delay at start of match (maybe only high levels?)
* Options on the Pause screen (quit, music, sfx).
* Store blocks. (classic only?)
* Update mode order and names to be (will require more space):
	* Kraken, Classic, Kraken Alt* Description of modes in option screen.
* Game over screen (polished).
* When on Level 29, display MAX instead.
* Points kicker
* Trigger sound test on Konami Code.

//investigate
* Number of rows that hit the tentacle adds a delay to next attack.
* See if tentacles can be made to work with name tables.
* Screen shake on hit. (others say this is annoying)
* Hard drop trails. (likely too much cpu)

//sound
* Kraken hit.
* Kraken hitx4
* landing should be louder


COMPLETE:
* Add ARE (spawn delay) Maybe 5 frames, and switch lock delay to 15.
* Clean up sound test.
* Hi-score display (per mode).
	* Couldn't find a good spot during gameplay, so currently only in the options.
* Option to turn off SFX.
* Update to use NES block layouts.
	* Update to have all blocks start face down. (was just T block actually).
* Score for Classic mode.
	* 1 lines:		2 lines:		3 lines:		4 lines:
	* 40 * (n + 1)	100 * (n + 1) 	300 * (n + 1) 	1200 * (n + 1)
	* + 1 point for ever row soft/hard dropped over. [cut]
* Clean up options:
	* Remove credits. [done]
	* Remove "block type". [done]
	* Re-add tower. [cut]
	* Starting level. [done]
* Credits on boot.
* More clear path out of options.
	* Change flow to go through options on the way to gameplay state.
* Safe zone issues.
* Faster start.
* Fast music when tentacle is maxed out.
* Better lock delay (consistent).
* Wall kicks
* Multiple tentacles. - possibly if when reaching top they go into name table.
* Pal swap based on time of day/night.
* Option to return to main menu on game over.
* Non-Nintendo blocks. (not fun :()
* Hard drop (up on d-pad).
* Option screen.
* Credits screen.
* Time based tentacle movement (rather than on landing).
* Option to choose between Kraken and Klassic gameplay. (classic may require score, ability to go back to main menu).
* Push start flashing text.

CUT:
* Last chance move on hard drop (maybe optional).
	* Feels weird. See commented out code in movement().
* Sound on hit tentacle.
	* Really need mutliple sounds for 4 line hit too.
* Sound on drop and then lock.
	* Don't like that it won't be consistent between lockdelay and just slow falling.

BUGS:
* Bad wall kick: http://harddrop.com/fumen/?m115@fhB8NemL2SAy3WeD0488AwkUNEjhd5DzoBAAvhA+qu?AA
* After quiting to main menu, previous match "next" block continues to show.

COMPLETE:
* S and Z are too high when flat (or too low when vert)
* If starting on level 10+, level up is happening on wrong level (happens as
  soon as player hits 110 lines, regardles of starting level).
* Next block is hidden during line clear.
* When hitting game over, final sprite switches.
	* I think this is because the vram buffer is cleared at the start of game over, 
	  before it has a chance to copy over the new block to the nametable.
* CNR - Horz input has to be pressed again if line is cleared.
* Hitch when tentacle retracts on hitting max (because of delays).
* At level 29, the blocks never trigger game over.
* Music isn't playing on main menu.
* Moving tentacle keeps moving after reaching max (possibly fixed with multi-tentacle attack).
* Tentacles are not budgeted.
* Graphical corruption on Game Over (rarely)
* Hard drop puts blocks 1 tile too far (rarely).
* Sprite flicker when blocks land.

CUT:
* Sprites do not draw when transitioning between name tables.
	* Don't care enough to redo how transitions work.

SCRIPT IDEAS:

* LAND - Make every move count! Rewards deliberate play. Punishes sloppy.
* TIME - Race to get as much done as possible before the Kraken attacks. Rewards fast play. Punishes overthinking.
* CLASSIC - The traditional block dropping puzzle mechanics you know and love.

* Story -

The Kraken cometh...

From the deepest trenches of the ocean, the Kraken has come to lay waste to your city!

The archers have slung every arrow.
The burning tar has run dry.
Every piece of military weaponary has been dispatch.

And yet... the Kraken moves forward, climbing the towering walls of your seaside fortress.

With no ammunition left, the city itself moves from a barricade, to weapon!

The stones of the walls are broken off, and hurdled down at hidious creature. And it just might be enough to
slow it down till morning, when surely help will arrive...

* Manual:
- Have a page with text book lore about the Kraken, with some cool pictures.
- A page for each mode.
- A making of section about how the game was created.

*/

void main (void)
{
	ppu_off(); // screen off

	// load the palettes
	//pal_bg(palette_bg);
	//pal_spr(palette_sp);

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

	attack_style = ATTACK_ON_TIME;// ATTACK_ON_LAND;
	music_on = 1;
	sfx_on = 1;
	block_style = BLOCK_STYLE_CLASSIC;
	state = 0xff; // uninitialized so that we don't trigger a "leaving state".
	cur_garbage_type = 0;

	pal_bright(0);
	go_to_state(STATE_BOOT);
	fade_from_black();

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
			case STATE_BOOT:
			{
				if (tick_count == 120 || pad1_new & PAD_START)
				{
					fade_to_black();
					go_to_state(STATE_MENU);
					fade_from_black();
				}
				break;
			}
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
						fade_to_black();
						go_to_state(STATE_OPTIONS);
						fade_from_black();
					}
				}
				break;
			}

			case STATE_OPTIONS:
			{
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
					music_stop();
					SFX_PLAY_WRAPPER(SOUND_START);

					fade_to_black();
					ppu_off();
					vram_adr(NTADR_A(0,0));
					vram_unrle(title_screen);
					ppu_on_all();
					fade_from_black();

					// little cheap to start at very high levels.
					if (pad1 & PAD_A)
					{
						cur_level += 10;
					}
					go_to_state(STATE_GAME);
				}

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

					case 0: // starting level

						if (cur_level < 9 )
						{
							++cur_level;
						}
						else 
						{
							cur_level = 0;
						}
						break;

					case 1: // Attack style

						//attack_style = (attack_style + 1) % ATTACK_NUM;

						if (attack_style < ATTACK_NUM - 1)
						{
							++attack_style;
							display_highscore();
						}
						break;

					case 2: // Music off/on

						//music_on = (music_on + 1) % 2;

						if (music_on == 0)
						{
							music_on = 1;
							MUSIC_PLAY_WRAPPER(MUSIC_TITLE);
							music_pause(0);
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

					case 3: // sound fx on/off
					{
						if (sfx_on == 0)
						{
							sfx_on = 1;
						}
						break;
					}

					default:
						break;
					}

					SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);
					display_options();
				}
				else if (pad1_new & PAD_LEFT)
				{
					switch (cur_option)
					{

					case 0: // starting level

						if (cur_level != 0)
						{
							--cur_level;
						}
						else
						{
							cur_level = 9;
						}
						break;

					case 1: // Attack style
						// if (attack_style == 0)
						// {
						// 	attack_style = ATTACK_NUM;
						// }
						// attack_style = (attack_style - 1) % ATTACK_NUM;

						if (attack_style != 0)
						{
							--attack_style;
							display_highscore();
						}

						break;

					case 2: // Music off/on
						// if (music_on == 0)
						// {
						// 	music_on = 2;
						// }
						// music_on = (music_on - 1) % 2;

						if (music_on != 0)
						{
							music_on = 0;
							music_pause(1);
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

					case 3: // sound fx off/on
					{
						if (sfx_on != 0)
						{
							sfx_on = 0;
						}
						break;
					}

					default:
						break;
					}

					SFX_PLAY_WRAPPER(SOUND_MENU_LOW);
					display_options();
				}
				else if (pad1_new & PAD_DOWN)
				{
					cur_option = (cur_option + 1) % NUM_OPTIONS;
					SFX_PLAY_WRAPPER(SOUND_MENU_LOW);
					display_options();
				}
				else if (pad1_new & PAD_UP)
				{
					if (cur_option == 0)
					{
						cur_option = NUM_OPTIONS;
					}
					cur_option = (cur_option - 1) % NUM_OPTIONS;
					SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);
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

				if (row_to_clear >= 0)
				{

					--attack_row_status[row_to_clear];
					if (attack_row_status[row_to_clear] == 0)
					{
						row_to_clear = -1;
					}
				}

//PROFILE_POKE(0x5f); // green
				// delay a frame for perf.
				if (attack_style != ATTACK_NEVER && attack_queued)
				{
					// TODO: Perf - Very expensive.
					add_block_at_bottom();

					clear_rows_in_data(BOARD_END_Y_PX_BOARD);
					attack_queued = 0;
					attack_queue_ticks_remaining = attack_delay;
				}

//PROFILE_POKE(0x9f); //blue

				if (delay_spawn_remaining != -1)
				{
					// This would normally be done in movement(), but since that isn't being
					// called we want to make sure charge DAS works.
					if (horz_button_delay > 0)
					{
						--horz_button_delay;
					}
					--delay_spawn_remaining;
					if (delay_spawn_remaining == 0)
					{
						spawn_new_cluster();
						delay_lock_remaining = -1;
					}
				}
				else
				{
					movement();
				}
				

//PROFILE_POKE(0x3f); // red
				draw_gameplay_sprites();
//PROFILE_POKE(0x1f); // white
				if (attack_style == ATTACK_ON_TIME && attack_queue_ticks_remaining != 0)
				{
					--attack_queue_ticks_remaining;

					if (attack_queue_ticks_remaining == 0)
					{
						attack_queued = 1;
						attack_queue_ticks_remaining = attack_delay;
					}
				}

				// STRESS MUSIC!

				local_t = 0;
				for (local_iy = 0; local_iy < STRESS_MUSIC_LEVEL * 10; ++local_iy)
				{
					if (game_board[local_iy + ((BOARD_OOB_END + 1) * 10)] != 0)
					{
						// music is stressed even if it doesn't start playing this frame.
						local_t = 1;

						if (cur_gameplay_music == MUSIC_GAMEPLAY)
						{
							cur_gameplay_music = MUSIC_STRESS;
							MUSIC_PLAY_WRAPPER(MUSIC_STRESS);
							break;
						}
					}
				}

				if (local_t == 0 && cur_gameplay_music == MUSIC_STRESS)
				{
					cur_gameplay_music = MUSIC_GAMEPLAY;
					MUSIC_PLAY_WRAPPER(MUSIC_GAMEPLAY);
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
//PROFILE_POKE(0x1e); // white
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
				if (pad1_new & PAD_B)
				{
					//go_to_state(STATE_GAME);
					go_to_state(STATE_MENU);
				}
				if (pad1_new & PAD_A)
				{
					//go_to_state(STATE_GAME);
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
						// ignore settings.
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
					// Intentionally not using wrapper so this plays regardless of settings.
					sfx_play(test_sound, 0);
				}

				// EXIT
				//

				if (pad1_new & PAD_SELECT || pad1_new & PAD_START)
				{
					go_to_state(STATE_MENU);
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
	static char shake_offset;
	static unsigned char speed;
//PROFILE_POKE(0x5f); // green
	// clear all sprites from sprite buffer
	oam_clear();
//PROFILE_POKE(0x9f); // blue
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

		// Next is now in NameTable.
		// if (next_cluster.layout & mask)
		// {
		// 	oam_spr((15 << 3) + (local_ix << 3), (1 << 3) + (local_iy << 3), next_cluster.sprite, 0);
		// }

		++local_ix;
		if (local_ix >= 4)
		{
			local_ix = 0;
			++local_iy;
		}
	}

//PROFILE_POKE(0x1f); // white

	if (attack_style != ATTACK_NEVER)
	{
		shake_offset = 0;
		if (attack_style == ATTACK_ON_TIME)
		{
			if (attack_queue_ticks_remaining < 120)
			{
				speed = tick_count >> 2;
			}
			else if (attack_queue_ticks_remaining < 300)
			{
				speed = tick_count >> 3;
			}
			else
			{
				speed = tick_count >> 5;
			}
		}
		else
		{
			speed = tick_count >> 4;
		}

		// Loop through the attack columns and draw the off board portion as sprites.
		for (local_ix = 0; local_ix < BOARD_WIDTH; ++local_ix)
		{
			local_row_status = attack_row_status[local_ix];
			if (local_row_status > 0)
			{
				for (local_iy = 0; local_iy < local_row_status /*&& local_iy < ATTACK_QUEUE_SIZE*/; ++local_iy)
				{
					//if (attack_queue_ticks_remaining < 120)
					{
						//if (attack_queue_ticks_remaining % 8 == 0) // %8
						{
							//shake_offset = ((local_iy + (attack_queue_ticks_remaining << 4)) % 3) - 1;
						}
					}

					shake_offset = tenatcle_offsets[((local_iy + speed) & 3)]; // &3 = %4 = number of entries in array.

					// gross. Try to detect if this is the last piece, and also the end of the arm.
					if (local_iy == local_row_status - 1)
					{
					oam_spr(
						BOARD_START_X_PX + (local_ix << 3) + shake_offset,
						(BOARD_END_Y_PX) + (ATTACK_QUEUE_SIZE << 3) - (local_iy << 3),
						0xf9,
						1);
					}
					else
					{
					oam_spr(
						BOARD_START_X_PX + (local_ix << 3) + shake_offset,
						(BOARD_END_Y_PX) + (ATTACK_QUEUE_SIZE << 3) - (local_iy << 3),
						0xf8,
						1);
					}

				}
			}
		}
	}

	// oam_spr((22)<<3, 8<<3, 0x21, 3);
	// for (local_ix = 0; local_ix < 6; ++local_ix)
	// {
	// 	oam_spr((23+local_ix)<<3, 8<<3, '1' + local_ix, 3);
	// }

//PROFILE_POKE(0x3f); // red

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
		if (attack_style == ATTACK_NEVER)
		{
			// sleeping
			oam_spr(3 << 3, 25 << 3, 0x63, 1);
			oam_spr(3 << 3, 26 << 3, 0x73, 1);
		}
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
	// static unsigned char ix;
	// static unsigned char iy;
	// static unsigned int bit;
	// static unsigned int res;

	hit = 0;
	temp_fall_rate = 0;
	old_x = 0;
	delay_lock_skip = 0;

	++fall_frame_counter;

	if (pad1_new & PAD_SELECT)
	{
		//hit_reaction_remaining = 60;
		//  inc_lines_cleared();
		//  delay(1);
		//  inc_lines_cleared();
		//  delay(1);
		//  inc_lines_cleared();
		//  delay(1);
		//  inc_lines_cleared();
		//  delay(1);
		//  inc_lines_cleared();
		//  delay(1);
		//  inc_lines_cleared();
		//  delay(1);
		//  inc_lines_cleared();
		//  delay(1);
		//  inc_lines_cleared();
		//  delay(1);
		//  inc_lines_cleared();
		//  delay(1);
		//lines_cleared_one = 9;
		//inc_lines_cleared();
		//add_block_at_bottom();
		//spawn_new_cluster();

		// Don't allow forcing the tentacle up while it is on the way down.
		// Not too serious, but looks weird when the height in increases 
		// while the sprites are not moving up.
		if (row_to_clear == -1)
		{
			attack_queued = 1;
		}
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

	if (horz_button_delay > 0)
	{
		--horz_button_delay;
	}

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
			// normal delay * 8
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
			
			// ix = 0;
			// iy = 0;
			// for (bit = 0x8000; bit; bit >>= 1)
			// {
			// 	res = cur_cluster.layout & bit;

			// 	// solid bit.
			// 	if (res)
			// 	{

			// 		in_x = cur_block.x + ix;
			// 		in_y = cur_block.y + iy;
			// 		in_id = 5; //cur_cluster.sprite;
			// 		set_block( );
			// 	}

			// 	++ix;
			// 	if (ix >= 4)
			// 	{
			// 		ix = 0;
			// 		++iy;
			// 	}
			// }

			++cur_block.y;

			// delay(1);
			// clear_vram_buffer();
		}

		// No delay lock on hard drops.
		delay_lock_skip = 1;

		// UNCOMMENT FOR LAST CHANCE MOVE ON HARD DROP
		// cur_block.y -= 1;
		// fall_frame_counter = 1;
	}
	else
	{
		// Hard drop skips all this to avoid dropping to the bottom
		// and then dropping again because it happens to be
		// the natural fall frame.
		if (pad1_new & PAD_DOWN || delay_lock_remaining != -1)
		{
			if (pad1_new & PAD_DOWN)
			{
				// if a new press was made this frame, skip the delay lock.
				delay_lock_skip = 1;
			}
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

	// If the fall_rate is greater than the delay lock, the player has already waited
	// long enough and doesn't need a delay lock.
	if (fall_rate > DELAY_LOCK_LEN)
	{
		delay_lock_skip = 1;
	}

	//debug_display_number(delay_lock_remaining,0);

	hit = 0;

//PROFILE_POKE(0x3f); //red
	// Offset from the bottom.
	if (is_cluster_colliding())
	{
		if (delay_lock_remaining == -1)
		{
			delay_lock_remaining = DELAY_LOCK_LEN - fall_rate;
		}
		// TODO: doesn't this mean that the delay lock is only decrementing every time that 
		//		 the block tries to move down, meaning lower g levels will have a longer
		//		 delay lock?
		// No, because when delay_lock_remaing is != -1, it triggers a "down" press (see above).
		--delay_lock_remaining;

		// Clamped to tile space, then multiplied back to pixel space
		//cur_block.y = (cur_block.y >> 3) << 3;

		// Move it above the collided tile.
		cur_block.y -= 1;
		if (delay_lock_remaining == 0 || delay_lock_skip)
		{
			hit = 1;
			delay_lock_remaining = -1;
		}
	}
	else
	{
		delay_lock_remaining = -1;
	}
	

	if (hit)
	{
//PROFILE_POKE(0x5f); //green
		put_cur_cluster();
//PROFILE_POKE(0x9f); //blue
		// Spawn a new block.
		//spawn_new_cluster();
		delay_spawn_remaining = DELAY_SPAWN_LEN;
	}
//PROFILE_POKE(0x1e); //none
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

//PROFILE_POKE(0x5f); //green

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
//PROFILE_POKE(0x3f); //red
			set_block( );
//PROFILE_POKE(0x5f); //green
		}

		++ix;
		if (ix >= 4)
		{
			ix = 0;
			++iy;
		}
	}

//PROFILE_POKE(0x9f); //blue
	SFX_PLAY_WRAPPER(SOUND_LAND);

	if (min_y <= BOARD_OOB_END)
	{
		go_to_state(STATE_OVER);
		return;
	}
	else
	{
//PROFILE_POKE(0x3f); //red
		// hide the sprite while we work.
		cur_block.y = 255;

		// We want to hid the sprite that just landed.
		// TODO: Shouldn't this only be if we clear a line?
		oam_set(0);
		//
		oam_spr(0, 0, 0, 0);
		oam_spr(0, 0, 0, 0);
		oam_spr(0, 0, 0, 0);
		oam_spr(0, 0, 0, 0);

//PROFILE_POKE(0x9f); //blue
		if (attack_style == ATTACK_ON_LAND)
		{
			attack_queued = 1;
		}

		// if (min_y <= (BOARD_OOB_END + STRESS_MUSIC_LEVEL) && cur_gameplay_music != MUSIC_STRESS)
		// {
		// 	cur_gameplay_music = MUSIC_STRESS;
		// 	music_play(MUSIC_STRESS);
		// }

		clear_rows_in_data(max_y);

	}
//PROFILE_POKE(0x3f); //green
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
			if(game_board[TILE_TO_BOARD_INDEX(x,y)]) // != 5 && game_board[TILE_TO_BOARD_INDEX(x,y)] != 0)
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
	static unsigned int mask;
	static unsigned char next_block_vram[4];

	id = 0;

	delay_lock_remaining = -1;

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
	if (block_style == BLOCK_STYLE_MODERN)
	{
		memcpy(next_cluster.def, cluster_defs_modern[id], 4 * 2);
	}
	else
	{
		memcpy(next_cluster.def, cluster_defs_classic[id], 4 * 2);
	}

	//next_cluster.def = cluster_defs[id]; // def_z_rev_clust;
	next_cluster.layout = next_cluster.def[0];
	next_cluster.sprite = cluster_sprites[id];

//PROFILE_POKE(0x9f); //blue
	// Put the next block into the nametable.
	local_iy = 0;
	local_ix = 0;
	local_t = next_cluster.sprite;
	// We need only check the middle 2 rows because all clusters spawn horizontal which 
	// means the top and bottom rows are empty.
	for (mask = 0x800; mask > 8; mask >>= 1)
	{
		if (next_cluster.layout & mask)
		{
			next_block_vram[local_ix] = local_t;
		}
		else
		{
			next_block_vram[local_ix] = 0;
		}
		

		++local_ix;
		if (local_ix >= 4)
		{
			multi_vram_buffer_horz(next_block_vram, 4, get_ppu_addr(cur_nt, 120, 16 + (local_iy << 3)));
			local_ix = 0;
			++local_iy;
		}
	}	
//PROFILE_POKE(0x1e); //none

	// There is an edge case where a block moves down on the frame it is spawned, while
	// at the top of the board.
	// In such a case the block will be shifted back up to its starting point, which
	// is not out of bounds, and so the game will not trigger game over.
	if (state != STATE_OVER && is_cluster_colliding())
	{
		// trigger game over. First place the new block in nt, then
		// move the the game over state.
		put_cur_cluster();
		go_to_state(STATE_OVER);
	}
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
		++cur_block.x;
		if (is_cluster_colliding())
		{
			cur_block.x -= 2;
			if (is_cluster_colliding())
			{
				// Special case for line piece :(
				// TODO: May be to change side that gets checked, after updating block layouts.
				--cur_block.x;
				if (is_cluster_colliding())
				{
					// Officially no where to go. Revert back to original position
					// and rotation, and play a whaa-whaa sound.
					cur_block.x += 2;
					cur_rot = old_rot;
					cur_cluster.layout = cur_cluster.def[cur_rot];
					SFX_PLAY_WRAPPER(SOUND_BLOCKED);
					return;
				}
			}
		}
	}


	SFX_PLAY_WRAPPER(SOUND_ROTATE);

}

void go_to_state(unsigned char new_state)
{
	static int address;
	static unsigned char i;
	static unsigned char fade_delay;
	static unsigned char prev_state;

	fade_delay = 5;
	prev_state = state;

	switch (state)
	{
		case STATE_BOOT:
		case STATE_SOUND_TEST:
		{
			MUSIC_PLAY_WRAPPER(MUSIC_TITLE);
			break;
		}
		case STATE_OPTIONS:
		{
			pal_bg(palette_bg);
			saved_starting_level = cur_level;
			fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];
			row_to_clear = -1;
			display_level();
			display_score();
			break;
		}

		case STATE_PAUSE:
		{
			pal_bright(4);
			break;
		}
		
		case STATE_GAME:
		{
			// Little bit of future proofing in case we add other ways
			// to exit the game (eg. from pause).
			if (cur_score > high_scores[attack_style])
			{
				high_scores[attack_style] = cur_score;
			}
			break;
		}

	default:
		break;
	}

	state = new_state;

	switch (state)
	{
		case STATE_BOOT:
		{
			pal_bg(palette_bg_options);
			ppu_off();
			vram_adr(NTADR_A(0,0));
			vram_unrle(boot_screen);
			ppu_on_all();

			break;
		}
		case STATE_MENU:
		{
			pal_bg(palette_bg);
			pal_spr(palette_sp);
			scroll_y = 0;
			time_of_day = 0;

			if (prev_state == STATE_OPTIONS || prev_state == STATE_BOOT || prev_state == STATE_SOUND_TEST)
			{
				oam_clear();

				ppu_off();
				vram_adr(NTADR_A(0,0));
				vram_unrle(title_screen);
				ppu_on_all();
			}
			else
			{
				if (prev_state == STATE_OVER)
				{
					fade_to_black();
				}

				reset_gameplay_area();

				scroll(0, 0x1df); // shift the bg down 1 pixel
				MUSIC_PLAY_WRAPPER(MUSIC_TITLE);

				if (prev_state == STATE_OVER)
				{
					fade_from_black();
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

			// handle case where player used cheat to jump 10 levels, and then quit back
			// to the main menu.
			cur_level %= 10;
			cur_option = 0;

			ppu_on_all();

			display_options();
			// too much for 1 frame.
			delay(1);
			clear_vram_buffer();
			display_highscore();

			break;
		}

		case STATE_SOUND_TEST:
		{
			oam_clear();
			ppu_off(); // screen off

			pal_bg(palette_bg_options);
			vram_adr(NTADR_A(0,0));
			vram_unrle(sound_screen);

			ppu_on_all(); // turn on screen

			test_song = test_sound = 0;
			test_song_active = 0xff;

			display_song();
			display_sound();

			break;
		}

		case STATE_GAME:
		{


			// This gets done in the main menu too.
			if (prev_state == STATE_OVER)
			{
				reset_gameplay_area();
			}

			if (prev_state != STATE_PAUSE)
			{
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

				require_new_down_button = 1;
				if (attack_style == ATTACK_ON_TIME)
				{
					attack_queue_ticks_remaining = attack_delay;
				}
			}

			// Do this at the end of the state change so that
			// the up beat music doesn't kick in until after
			// everything transitions in.
			cur_gameplay_music = MUSIC_GAMEPLAY;
			MUSIC_PLAY_WRAPPER(MUSIC_GAMEPLAY);

			break;
		}

		case STATE_PAUSE:
		{
			pal_bright(2);
			MUSIC_PLAY_WRAPPER(MUSIC_PAUSE);
			break;
		}
		case STATE_OVER:
		{
			// fix bug where mashing up/down to quickly hit gamover would case nametable coruption.
			delay(1);
			clear_vram_buffer();

			// Without this, the "next" block won't appear for the first half of the sequence.
			draw_gameplay_sprites();

			music_stop();
			SFX_MUSIC_PLAY_WRAPPER(SOUND_GAMEOVER);

			// Without music this delay feels really odd.
			if (music_on)
			{
				delay(120);
			}

			// treat this like music, since it is a jingle.
			SFX_MUSIC_PLAY_WRAPPER(SOUND_GAMEOVER_SONG);

			// Not sure why this was here. It causes the next block to
			// vanish.
			//oam_clear();

			pal_bright(5);
			delay(fade_delay);
			pal_bright(6);
			delay(fade_delay);
			pal_bright(7);
			delay(fade_delay);
			pal_bright(8);
			delay(fade_delay);

			// for (i = 0; i < 3; ++i)
			// {
			// 	address = get_ppu_addr(cur_nt, BOARD_START_X_PX, (14+i)<<3);
			// 	multi_vram_buffer_horz(empty_row, 10, address);
			// }

			address = get_ppu_addr(cur_nt, 96, 14<<3);
			multi_vram_buffer_horz( "GAME OVER!", sizeof("GAME OVER!"), address);
			address = get_ppu_addr(cur_nt, 96, 15<<3);
			multi_vram_buffer_horz("A-RESTART ", sizeof("A-RESTART "), address);
			address = get_ppu_addr(cur_nt, 96, 16<<3);
			multi_vram_buffer_horz("B-QUIT    ", sizeof("B-QUIT    "), address);
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
	static unsigned char lines_total;
	++lines_cleared_one;

	if (lines_cleared_one == 10)
	{
		// Basically the level we should be on, by truncating the ones.
		lines_total = (lines_cleared_hundred * 10) + lines_cleared_ten;

		if (cur_level <= lines_total)
		{
			// we only handle things up to level 29.
			if (cur_level < 29 )
			{
				++cur_level;
				fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];
			}
		}

		++time_of_day;
		if (time_of_day >= NUM_TIMES_OF_DAY)
		{
			time_of_day = 0;
		}

		pal_bg(palette_bg_list[time_of_day]);

		// TODO: Use pal_col()
		memcpy(temp_pal, palette_sp, sizeof(palette_sp));
		// blocks
		temp_pal[1] = palette_bg_list[time_of_day][1];
		temp_pal[2] = palette_bg_list[time_of_day][2];
		temp_pal[3] = palette_bg_list[time_of_day][3];
		// kraken
		temp_pal[6] = palette_bg_list[time_of_day][14];
		temp_pal[7] = palette_bg_list[time_of_day][15];
		// flag bg
		temp_pal[10] = palette_bg_list[time_of_day][2];
		//temp_pal[14] = pal_changes[pal_id + 1];
		pal_spr(temp_pal);

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
	one_vram_buffer('0' + lines_cleared_hundred, get_ppu_addr(cur_nt,4<<3,3<<3));
	one_vram_buffer('0' + lines_cleared_ten, get_ppu_addr(cur_nt,5<<3,3<<3));
	one_vram_buffer('0' + lines_cleared_one, get_ppu_addr(cur_nt,6<<3,3<<3));
}

void display_score()
{
	static unsigned long temp_score;
	static unsigned char i;

	temp_score = cur_score;

	// clear out any old score.
	multi_vram_buffer_horz("      ", 6, get_ppu_addr(cur_nt, 0, 6<<3));

	i = 0;
	while(temp_score != 0)
    {
        unsigned char digit = temp_score % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(cur_nt, (6<<3) - (i << 3), 6<<3 ));

        temp_score = temp_score / 10;
		++i;
    }
}

void display_highscore()
{
	static unsigned long temp_score;
	static unsigned char i;

	temp_score = high_scores[attack_style];

	// clear out any old score.
	multi_vram_buffer_horz("0000000", 7, get_ppu_addr(0, 17<<3, 27<<3));

	i = 0;
	while(temp_score != 0)
    {
        unsigned char digit = temp_score % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(0, ((17+7)<<3) - (i << 3), 27<<3 ));

        temp_score = temp_score / 10;
		++i;
    }
}

void display_level()
{
	// We let level be displayed as zero based because it makes more sense when
	// comparing it to lines (eg. lines is 80, level is 8).
	static unsigned char temp_level;
	static unsigned char i;

	temp_level = cur_level;
	i = 0;

	if (cur_level < 10)
	{
		multi_vram_buffer_horz("00", 2, get_ppu_addr(cur_nt,5<<3,9<<3));
	}

	while(temp_level != 0)
    {
        unsigned char digit = temp_level % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(cur_nt, (6<<3) - (i << 3), 9<<3 ));

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
	static unsigned int line_score_mod;
//PROFILE_POKE(0x9f); //blue
	i = 0;
	prev_level = cur_level;

	// 0xff used to indicate unused.
	memfill(lines_cleared_y, 0xff, 4);
//PROFILE_POKE(0x3f); //red
	// Start at the bottom of the board, and work our way up.
	for (iy = start_y; iy > BOARD_OOB_END; --iy)
	{
		// Assume this row is complete unless we find an empty
		// block.
		line_complete = 1;
		for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
		{
//PROFILE_POKE(0x5f); //green
			if (game_board[TILE_TO_BOARD_INDEX(ix,iy)] == 0)
			//if (is_block_free(ix, iy))
			{
				// This block is empty, so we can stop checking this row.
				line_complete = 0;
				break;
			}
//PROFILE_POKE(0x3f); //red
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
			SFX_PLAY_WRAPPER(SOUND_LEVELUP);
		}
		else if (i == 4)
		{
			SFX_PLAY_WRAPPER(SOUND_MULTIROW);
		}
		else
		{
			SFX_PLAY_WRAPPER(SOUND_ROW);
		}

		// 40 * (n + 1)	100 * (n + 1) 	300 * (n + 1) 	1200 * (n + 1)
		switch (i)
		{
			case 1:
			{
				line_score_mod = 40;
				break;
			}

			case 2:
			{
				line_score_mod = 100;
				break;
			}

			case 3:
			{
				line_score_mod = 300;
				break;
			}
			
			case 4:
			default:
			{
				line_score_mod = 1200;
				break;
			}
		}
		cur_score += (line_score_mod * (cur_level + 1));
		display_score();

		// potential hit reaction.
		if (hit_reaction_remaining > 0)
		{
			draw_gameplay_sprites();
		}
		reveal_empty_rows_to_nt();
	}
//PROFILE_POKE(0x1e); //none
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

	// This also gets called when going back to the main menu.
	if (state == STATE_GAME)
	{
		draw_gameplay_sprites();
	}
	//delay(1);
	//clear_vram_buffer();
	//return;

	for (ix = 0; ix <= BOARD_END_X_PX_BOARD; ++ix)
	{
		// copy a column into an array.
		for (iy = 0; iy < BOARD_HEIGHT; ++iy)
		{
			copy_board_data[iy] = game_board[TILE_TO_BOARD_INDEX(ix, iy + BOARD_OOB_END + 1)];

			// if (iy <= STRESS_MUSIC_LEVEL && copy_board_data[iy] != 0)
			// {
			// 	fast_music = 1;
			// }
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
//PROFILE_POKE(0x1e); //clear so we don't screw up the visualization.
			delay(1);
			clear_vram_buffer();
		}
	}

	// if (fast_music && cur_gameplay_music != MUSIC_STRESS)
	// {
	// 	cur_gameplay_music = MUSIC_STRESS;
	// 	music_play(MUSIC_STRESS);
	// }
	// else if (!fast_music && cur_gameplay_music != MUSIC_GAMEPLAY)
	// {
	// 	cur_gameplay_music = MUSIC_GAMEPLAY;
	// 	music_play(MUSIC_GAMEPLAY);
	// }
}

void add_block_at_bottom()
{
	static signed char ix;
	static unsigned char iy;
	static unsigned char attacks;
	//static unsigned char tentacle_fill[7];

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
				// // This attack has maxed out, so remove it from the board (animated).
				// while (attack_row_status[ix] > 0)
				// {
				// 	//hit_reaction_remaining = 60;
				// 	--attack_row_status[ix];
				// 	delay(2);
				// 	draw_gameplay_sprites();
				// 	clear_vram_buffer();
				// }

				row_to_clear = ix;

				// Assuming there is only one attack at a time, we can
				// now exit this loop, and go to the part where it starts
				// the next attack.
				// Note: This assumes that the nametable does not need to
				//		 be updated, since it gets updated as it goes.
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

				game_board[TILE_TO_BOARD_INDEX(ix, BOARD_END_Y_PX_BOARD)] = garbage_types[cur_garbage_type]; //     0x60; //0xf7; //(attack_row_status[ix] == (ATTACK_QUEUE_SIZE + 1)) ? 0xf9 : 0xf8;
				++cur_garbage_type;
				if (cur_garbage_type >= NUM_GARBAGE_TYPES)
				{
					cur_garbage_type = 0;
				}
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

void reset_gameplay_area()
{
	memfill(game_board, 0, BOARD_SIZE);

	// Reset stats.
	lines_cleared_one = lines_cleared_ten = lines_cleared_hundred = cur_score = 0;
	cur_level = saved_starting_level;
	fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];
	row_to_clear = -1;
	delay_lock_remaining = -1;

	// load the palettes
	time_of_day = 0;
	pal_bg(palette_bg_list[time_of_day]);
	pal_spr(palette_sp);

	display_lines_cleared();
	display_score();
	display_level();

	oam_clear();

	// Reset the ppu for gameover case.
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
		multi_vram_buffer_horz("000", 3, get_ppu_addr(0,(4<<3),(14<<3)));
	}

	while(temp != 0)
    {
        unsigned char digit = temp % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(0, (6<<3) - (i << 3), (14<<3) ));

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
		multi_vram_buffer_horz("000", 3, get_ppu_addr(0,(25<<3),(14<<3)));
	}

	while(temp != 0)
    {
        unsigned char digit = temp % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(0, (27<<3) - (i << 3), (14<<3) ));

        temp = temp / 10;
		++i;
	}
}

void display_options()
{
	multi_vram_buffer_horz(&starting_levels[cur_level], 1, get_ppu_addr(0,17<<3,17<<3));
	multi_vram_buffer_horz(attack_style_strings[attack_style], ATTACK_STRING_LEN, get_ppu_addr(0,17<<3,19<<3));
	multi_vram_buffer_horz(off_on_string[music_on], OFF_ON_STRING_LEN, get_ppu_addr(0,17<<3,21<<3));
	multi_vram_buffer_horz(off_on_string[sfx_on], OFF_ON_STRING_LEN, get_ppu_addr(0,17<<3,23<<3));

	// NOTE: One redundant call.
	multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 8<<3, 17<<3));
	multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 8<<3, 19<<3));
	multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 8<<3, 21<<3));
	multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 8<<3, 23<<3));

	multi_vram_buffer_horz(option_icon, 2, get_ppu_addr(0, 8<<3, (17 + (cur_option<<1))<<3));
}


void fade_to_black()
{
	pal_bright(3);
	delay(2);
	pal_bright(2);
	delay(2);
	pal_bright(1);
	delay(2);
	pal_bright(0);
	delay(2);
}

void fade_from_black()
{
	pal_bright(1);
	delay(2);
	pal_bright(2);
	delay(2);
	pal_bright(3);
	delay(2);
	pal_bright(4);
	delay(2);
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