/** (C) Matt Hughson 2020 */

#define PLAT_GB 1

#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#if PLAT_NES
#include "BG/game_area.h"
#if VS_SYS_ENABLED
#include "BG/vs_boot_screen.h"
#include "BG/vs_settings_difficulty.h"
#include "BG/vs_settings_mode.h"
#include "BG/vs_title_screen.h"
#include "BG/high_score_screen.h"
#else
#include "BG/title_screen.h"
#include "BG/boot_screen.h"
#include "BG/options_screen.h"
#endif
#include "BG/sound_screen.h"
#include "BG/ty_screen.h"
#include "../include/stdlib.h"
//#include "../include/string.h"
#include "main.h"
#endif

#if PLAT_GB
#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "Fade.h"
#include "String.h"
#include "Print.h"
#include "rand.h"
#include "SGB.h"
#include "Music.h"
#include "BankManager.h"
#include "Palette.h"
#include "OAMManager.h"
#include "SGBHelpers.h"
#include "savegame.h"

#include "cbtfx.h"
#include "beep_sfx.h"
#include "block_land_sfx.h"
#include "block_rotate_sfx.h"
#include "bop_sfx.h"
#include "level_up_4_sfx.h"
#include "level_up_sfx.h"
#include "multi_row_destroyed_sfx.h"
#include "row_destroyed_sfx.h"
#include "start_game_sfx.h"
#include "unable_rotate_sfx.h"

#define SOUND_ROTATE			CBTFX_PLAY_block_rotate_sfx
#define SOUND_LAND				CBTFX_PLAY_block_land_sfx
#define SOUND_ROW				CBTFX_PLAY_row_destroyed_sfx
#define SOUND_MULTIROW			CBTFX_PLAY_multi_row_destroyed_sfx
#define SOUND_START				CBTFX_PLAY_start_game_sfx
#define SOUND_BLOCKED			CBTFX_PLAY_unable_rotate_sfx
#define SOUND_LEVELUP			CBTFX_PLAY_level_up_sfx
#define SOUND_LEVELUP_MULTI		CBTFX_PLAY_level_up_4_sfx
#define SOUND_MENU_HIGH			CBTFX_PLAY_beep_sfx
#define SOUND_MENU_LOW			CBTFX_PLAY_bop_sfx

#include <gbdk/platform.h>
#include <stdint.h>
#include <stdlib.h>

#include "StateFromBelow.h"
// TEMP
// stdlib.h
void srand(unsigned int seed) { initrand(seed); }

// https://github.com/Zal0/ZGB/commit/6bedcf160723a95595d21f70ae8d229d34373584
// memcmp while it is being added into GBDK
int memcmp(const char *s1, const char *s2, int n) {
	UINT8 i;
	char* c1 = (char*)s1;
	char* c2 = (char*)s2;
	for(i = 0; i != n; ++i, ++c1, ++c2) {
		if(*c1 != *c2){
			return (*c1 > *c2) ? 1 : -1;
		}
	}
	return 0;
}


// defined in ZGB main.c. Need to be able to remove it.
extern void LCD_isr() NONBANKED;
extern void UPDATE_TILE(INT16 x, INT16 y, UINT8* t, UINT8* c);

//unsigned char title_screen[] = { 0 };
//unsigned char game_area[] = { 0 };
//unsigned char boot_screen[] = { 0 };
unsigned char sound_screen[] = { 0 };
//unsigned char options_screen[] = { 0 };
//unsigned char ty_screen[] = { 0 };

IMPORT_MAP(gb_border);
IMPORT_MAP(game_area);
IMPORT_MAP(title_screen);
IMPORT_MAP(title_screen_dmg);
IMPORT_MAP(boot_screen);
IMPORT_MAP(options_screen);
IMPORT_MAP(options_screen_dmg);
IMPORT_MAP(ty_screen);
IMPORT_MAP(gameplay_map);

IMPORT_TILES(font);
IMPORT_TILES(font_on_black);
IMPORT_TILES(font_options_bright);
IMPORT_TILES(font_gameplay);

DECLARE_MUSIC(TitleMusic);
DECLARE_MUSIC(GameplayMusic);
DECLARE_MUSIC(GameplayStressMusic);
DECLARE_MUSIC(PauseMusic);
DECLARE_MUSIC(GameOverIntroMusic);
DECLARE_MUSIC(GameOverOutroMusic);

const unsigned char test_bg_tile = 128;
const unsigned char* digits[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };

// Palettes to flash between when leveling up on CGB.
const UWORD palette_flash[4] = { 0x7fff, 0x7ab4, 0x65ce, 0x0 };
const UWORD palette_normal[4] = { 0x7f97, 0x7ab4, 0x65ce, 0x0 };

// current sprite index in ZGB
extern UINT8 next_oam_idx;
// pointer to OAM data in ZGB
extern UINT8* oam;
// data used for feeding a single sprite into OAM.
UINT8 sprite_data[4];

#endif // PLAT_GB

/*

..::TODO::..


--GB--

BETA:

* BUG: GAME OVER graphics are chopped at the bottom.
* BUG: Both players hit A at the same time, and player got stuck on Game Over. https://discord.com/channels/731554439055278221/974456955622031401/980959536699568149
* FEEDBACK: Should be able to pause in versus. (idea: could use extra values in row height bits to signal pause)
* FEEDBACK: Should have a "garbage incoming" sound. (waiting on SFX)
* FEEDBACK: Should have a "garbage filling" sound. (waiting on SFX)
* FEEDBACK: Isn't obvious that you options is how you Start the game.
* BUG: Clearing 4 rows while 1 incoming garbage row is queued still results in sending 4 rows (4-1=3 == 4) (idea: could use extra values in row height bits to signal tetris)


BETA FIXED:

* FEEDBACK: Flags don't look like flags. Either swap the static tile, or add sprites back in.
* FEEDBACK: Should be able to cancel incoming garbage.
* BUG: No sound fx on countdown during gameplay.
* FEEDBACK: Bring menu palette into gameplay.
* INVESTIGATE: Try tbsp color fade: https://discord.com/channels/731554439055278221/974456955622031401/976159536345935942
* FEEDBACK: Consider removing H.Drop HOLD setting. (increases hold time instead)
* FEEDBACK: Add "First to 3 wins" to SIO, as well as overall win tracking.
* BUG: Flash on intial boot.
* BUG: Doesn't work on DSi emulator.
* BUG: Sprites don't appear on mGBA core.
* BUG: Garbage well should shift after every 9th row.
* BUG: If the tentacle is pushing blocks up when you die, then you first die, the tentacle will disappear and you just see the "blocks". 
       But then, on the game over screen, just for a split second, the tentacle randomly appears again and then disappears. 


BETA CUT:

* BUG: Stress music starts 1 row too early.
* BUG: Game hangs at Credits Screen on TGB Dual core.
* FEEDBACK: Make On/Off options loop.
* FEEDBACK: Stress music is annoying.
* FEEDBACK: Score/Level/Lines are too cramped.
* FEEDBACK: Add more colors to pieces.
* INVESTIGATE: You could hack cbtfx to use channel 1 instead with a few line changes since iirc the data structure is the same between the first 2. (sounds worse)



MUST:


SHOULD:

* [SIO] More variety in garbage tiles.
* [SIO] Play sound effect/visuals when garbage incoming.
* [SIO] Player 1 mashed B on Game Over and was able to exit before the other player (I think)
* [SIO] Player 1 (CGB) mash START while transitioning between Title and Settings. Player 2 (DMG) shows 0 in top left, Player 1 shows 1.
* Bottom of well looks weird going straight into water.
* [SIO] BUG: (Emulicious Only) Sometimes after playing a SP game, and then an MP game, Host will get random "0x55" event triggering a single line to appear.
* Add bubbles to settings screen.
* [SIO] BUG: Losing on the same frame as opponent causes switch from YOU LOSE to YOU WIN! (in CGB vs CGB emulator)

PROBABLY CUT:

* [SGB] Long delay setting attributes. Can this be done with linear array?
* High contrast mode.
* [SIO] Replicate "mode" choice by host.
* [SIO] Non-host starts slightly delayed from host, causing non-host to win in AFK case. (CGB vs SGB emulator)
* Try https://github.com/untoxa/VGM2GBSFX for alternate sfx driver (test integration with music, ROM size, clicking).
* BUG: Sound Effects cut out music in an un-natural way.
* BUG: Music sometimes has an extended first note.

CNR:

* [SGB] Flash on level up looks a little weird.
* [SIO] BUG: Sometimes when the first piece lands, garbage is sent over.
* [SIO] BUG: Once match ended for one player for no reason.
* [SIO] BUG: Sometimes player exits gameover screen without input, almost immediately (likely a gameplay SIO event interpretted as a player menu choice)
* BUG: DMG doesn't seem to be waiting 120 frames for music. CNR: I think I *might* have run the wrong build?

-------------------

FEATURES:

//must have
--

//should have
--

//nice to have
* Ghost pieces.
* Lock-delay settings (off, 10 frames, 20 frames)
* Options on the Pause screen (quit, music, sfx).
* Store blocks. (classic only?)
* Update mode order and names to be (will require more space):
	* Kraken, Classic, Kraken Alt* Description of modes in option screen.
* Game over screen (polished).
* Points kicker

//investigate
* Number of rows that hit the tentacle adds a delay to next attack.
* Screen shake on hit. (others say this is annoying)
* Hard drop trails. (likely too much cpu)
* Drought reduction: have a max number of turns a piece can drought. If hit that max, for it to be played.

//sound
* Kraken hit.
* Kraken hitx4
* landing should be louder


COMPLETE:
* Trigger sound test on Konami Code.
* Special Thanks screen
* Allow players to start on any level up to 29.
* Start level 29.
	* Select + Start on level 9.
* Disable hard drop (not on hold - People either want it or not).
	* Added 3 settings: off, tap, hold.
* Test NES rotations (confirmed)
* Delay at start of match (maybe only high levels?)
* Kill screen ideas:
- Remove lock delay.
- Shrink the play area. << THIS
- 2 block drops.
* Hard drop on HOLD.
* Reset on A+B+SEL+START
* Option to disable hard-drop (or require a slight hold to trigger hard drop.)
	* No hold setting, just on or off.
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
* See if tentacles can be made to work with name tables.
	* Want to keep animation.
* When on Level 29, display MAX instead.
	* Doge suggested showing levels beyond 29.
	* Added "Kill Screen"
* Last chance move on hard drop (maybe optional).
	* Feels weird. See commented out code in movement().
* Sound on hit tentacle.
	* Really need mutliple sounds for 4 line hit too.
* Sound on drop and then lock.
	* Don't like that it won't be consistent between lockdelay and just slow falling.

BUGS:
* Flicker on hard drop.
* Can get game over when spawned piece clears a line. Wouldn't be so bad if the line didn't get cleared.
* Kraken eye is all glitches when hitting tentacle on hardware.
	* Krysio kart only. Need resistor on pins?
* "When I was in the middle of playing a game the sound test came up, i was on a pink stage but couldn't catch the level." - KittyFae

COMPLETE:
* Flickering sprites when pressing select during Kraken modes.
* Blocks are not falling fast enough on soft drop
	* Was falling every 3 frames instead of 2 frames.
* Bad wall kick: http://harddrop.com/fumen/?m115@fhB8NemL2SAy3WeD0488AwkUNEjhd5DzoBAAvhA+qu?AA
	* Discovered a couple problems on top of special case for lines is now only lines.
* SKULL doesn't clear on game over.
* Game Over eat into side of nametable (1 tile too far).
* "the way nestris does it is it declares you dead when two pieces overlap which happens when there's a piece where the next piece spawns"
* Garbage in nametable after playing "timed", quiting to main menu, and entering options and changing a few settings.
	* likely writing too many values in display_options, not sure why not 100% though.
	* suspect you can hit this change an option with a high score at the same time PUSH START is toggled on/off
* Quick fall rate sometimes: https://clips.twitch.tv/FuriousIntelligentClipzMau5
* After quiting to main menu, previous match "next" block continues to show.
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

DIP SWITCHES:
* = not yet implemented.

All defaults will be 0.

FREE PLAY				|	0	0	-	-	-	-	-	-
CREDIT/COIN	1/1			|	0	1	-	-	-	-	-	-
CREDIT/COIN	1/2			|	1	0	-	-	-	-	-	-
CREDIT/COIN	1/4			|	1	1	-	-	-	-	-	-

PPU RP2C04-0001			|	-	-	0	0	0	-	-	-
PPU RP2C04-0002			|	-	-	1	0	0	-	-	-
PPU RP2C04-0003			|	-	-	0	1	0	-	-	-
PPU RP2C04-0004			|	-	-	1	1	0	-	-	-
PPU 2C03/2C05			|	-	-	0	0	1	-	-	-
PPU 2C02 (NES)			|	-	-	1	0	1	-	-	-

DISABLE MUSIC			|	-	-	-	-	-	1	-	-
DISABLE SFX				|	-	-	-	-	-	-	1	-
DISABLE ATTACT SOUND	|	-	-	-	-	-	-	-	1


Arcade Buttons:

1: p1 select (virtual - Doesn't seem to match physical controller in Messen)
2: p2 select
3: p1 start
4: p2 start

VERSUS TODO:
* Final Artwork from Duey.
* Package up artwork and dip sheet in zip (maybe zip everything including NES?)
* [cut] Font outline.
* [cut] Consider hard drop (setting, dip, hold by default, etc). [tested - no issues]
* [cut] On gameover, continue should go to Mode select, not title screen.
* [cut] Shared leaderboard on dual system (with save).
* [cut] Hide coin display in Free Play mode.
* [done] Better initial entry UI.
* [done] Add lidnariq to thanks if possible.
* [done] Shorten attract timer to 20-30 seconds.
* [done] Arrow sprites on leaderboards.
* [done] Re-enable music (when attact sound is disable) after inserting a coin. Leave disabled for Free Play.
* [done] Better gameover display. (remove press 1)
* [done] Attract gameplay.
* [done] Auto-forward if no input on the leaderboards for too long.
* [done] Countdown timer on entering initials. (not visible to player)
* [done] PPU support (incl. NES!)
* [done] Prefer credit style of 1/2
* [done] Press Start should say Press Any Button
* [done] Updated block art.
* [done] Skull transition feels odd to still have a full timer - jump to 5 seconds or something.
* [done] Leaderboards
* [done] Title Screen with "Vs."
* [done] Game Mode Screen art.
* [done] Remaining Dip Switches.
* [done] Game over timer (force quit).
* [done] Credit display.
* [done] Matenience coin counter.
* [done] Maintenience coin feeder.
* [done] start game with A or B as well.
* [done] game over quits with any key, changed messaging.
* [done] credit display.
* [done] Catch credits in NMI, and poll value in main.
* [done] Better coin display.
* [done] coin feedback across all states (but disbled during gameplay)
* [done] Auto-advance all menus to avoid burn in.
* [done] More buttons (not just 1) for entering initials. A to enter, B go back. Maybe 1 to skip.
* [done] Button 1 and Button 2 go to leaderbaords. Should just be 2.


SRAM:

* STATUS:
	* Disabled behind VS_SRAM_ENABLED.
	* Also need to manually turn XRAM back on in config.
	* Currently only functions fully on CPU1.
	* CPU2 is partially working, but that might just be by luck, since it should be mostly disabled.
	* Next step is to move the leaderboards out of SRAM, and instead use SRAM as messaging system, and hook into IRQ.

* Used as a messaging system, not storage for leaderboards.
	* This ensures that it works on system without access to SRAM, and on CPU2 without requesting SRAM access.
* Message: [unique id for validation] ... [msg type - leaderbaord update] [mode] [difficulty] [placement] [initals x 3] [score x 4] ... [end id]
*			1 byte ... 					   1 byte						   1 byte 1 byte	   1 byte	   2 bytes       4 bytes		 1 byte

CPU1:
	* Keeps local copy of leaderboard, and message queue.
	* When making changes to leaderboard, stores changes in message queue.
	* At end of frame:
		* Request SRAM.

		* Copy message queue to SRAM.
		* Clear local message queue.
		* Trigger IRQ.
		* Assume that IRQ will be done by the end of next frame.
	* In IRQ, clear message queue?

CPU2:
	* Keeps local copy of leaderbaord, and message queue.
	* When making changes to leaderboard, stores changes in message queue.
	* In IRQ:
		* Request SRAM.
		* Copy message queue.
		* Trigger IRQ.
		* Assume that IRQ will be done by the end of next frame.	

*/

// const unsigned char test_palette_bg[16]=
// {
// 	0x0f,0x1f,0x2f,0x3f,
// 	0x0f,0x1d,0x2d,0x3d,
// 	0x0f,0x1a,0x2a,0x3a,
// 	0x0f,0x10,0x20,0x30
// };

// const unsigned char metasprite_flag[]={
// 	  0,  0,0x6e,1,
// 	  8,  0,0x6f,1,
// 	  0,  8,0x7e,1,
// 	  8,  8,0x7f,1,
// 	  0, 16,0x8e,1,
// 	  8, 16,0x8f,1,
// 	  0, 24,0x9e,1,
// 	  8, 24,0x9f,1,
// 	128
// };

void vbl_delay(UINT8 frames)
{
	static UINT8 i;
	for (i = 0; i < frames; ++i)
	{
		wait_vbl_done();
	}
}

//const unsigned char flag_anim[] = {4, 0, 1, 2, 3 };

// Sprite* FlagTopLeft;
// Sprite* FlagTopRight;

// Sprite* FlagMidLeft;
// Sprite* FlagMidRight;

// Sprite* FlagBottomLeft;
// Sprite* FlagBottomRight;

// https://discord.com/channels/790342889318252555/790346049377927168/928576481624473600
// 

//const INT8 wave_table[] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, -1, -1, -1, -1 };

uint8_t counter = 0; 
void my_interrupt() NONBANKED {

	//if (state == STATE_GAME || state == STATE_OVER || state == STATE_PAUSE)
	{
		while (STAT_REG & STATF_BUSY);
		SCY_REG++;
		LYC_REG+=7;
		if (counter == 20) {
			counter = 0;
			SCY_REG = 1; 
			LYC_REG = 5; // 6 to trim top instead of bottom
		} else counter++;
	}
	// else
	// {
	// 	while (STAT_REG & STATF_BUSY);
	// 	SCX_REG = wave_table[(counter + (tick_count>>4)) % 16];
	// 	LYC_REG+=8;
	// 	if (counter == 16) {
	// 		counter = 0;
	// 		SCX_REG = 0; 
	// 		LYC_REG = 0; // 6 to trim top instead of bottom
	// 	} else counter++;
	// }
	// ++counter;
	// move_bkg(0, counter);

	// if (counter >= 20)
	// {
	// 	counter = 0;
	// }

	// LYC_REG = counter * 7;
}

void START() 
{
	static unsigned char i;
#if VS_SYS_ENABLED
	static unsigned int temp_secs;
	static unsigned char digit;
#endif
#if VS_SRAM_ENABLED
	static unsigned char j;
	static unsigned char k;
#endif

	LOAD_SGB_BORDER(gb_border);
	sgb_init_pals();

	CRITICAL {
		remove_LCD(LCD_isr);
		LYC_REG = 0;
		add_LCD(my_interrupt);

		// LCD_isr may have left sprites in a hidden state!
		SHOW_SPRITES;
	}

	CRITICAL {
        add_SIO(nowait_int_handler);    // disable waiting VRAM state before return

		// Add SIO interrupt to the list. The other flags are copied from
		// ZGB main.
        set_interrupts(VBL_IFLAG | TIM_IFLAG | LCD_IFLAG | SIO_IFLAG);
    }

	// enable SRAM for the entire program.
	ENABLE_RAM;

	ppu_off(); // screen off

	// use the second set of tiles for sprites
	// both bg and sprites are set to 0 by default
	bank_spr(1);

	set_vram_buffer(); // do at least once, sets a pointer to a buffer
	clear_vram_buffer();

	// TODO: This is actually the gameplay setup.
	off_nt = 0;
	cur_nt = 2;

#if PLAT_GB
	//INIT_FONT(font, PRINT_BKG);
	//vram_unrle(title_and_game_area);
	//InitScroll(BANK(title_and_game_area), &title_and_game_area, 0, 0);
#else
	vram_adr(NTADR_A(0,0));
	vram_unrle(title_screen);
	vram_adr(NTADR_C(0,0));
	vram_unrle(game_area);	
#endif // PLAT_GB

#if PLAT_GB
	scroll_y_game = 0;
	scroll_x_camera = 0;
#else
	scroll_y_game = 0x1df;
#endif // PLAT_GB
	
	//scroll(0, scroll_y_game); // shift the bg down 1 pixel

	//set_scroll_y_game(0xff);

	ppu_on_all(); // turn on screen

	//music_play(0);

	// Uninitialized save game. It would have already gone through integritiy checks
	// which will zero out the struct if it's new or corrupted.
	if (savegame.version == 0)
	{
		savegame.version = 1;
		savegame.attack_style = ATTACK_ON_TIME;
		savegame.music_on = 1;
		savegame.sfx_on = 1;
		savegame.hard_drops_on = 1;
	}

	block_style = BLOCK_STYLE_CLASSIC;
	state = 0xff; // uninitialized so that we don't trigger a "leaving state".
	cur_garbage_type = 0;
#if VS_SYS_ENABLED
	credits_remaining = 0;
	free_play_enabled = (DIP1 == 0 && DIP2 == 0);
	game_cost = ((DIP1 != 0) << 1) | (DIP2 != 0);
	if (game_cost == 3)
	{
		game_cost = 4; // cost of 3 seems odd.
	}
	savegame.music_on = DIP6 == 0;
	savegame.sfx_on = DIP7 == 0;
	high_score_entry_placement = 0xff;
	// DIP	Val	PPU
	// 000 	0	RP2C04-0001
	// 001 	1	RP2C04-0002
	// 010 	2	RP2C04-0003
	// 011 	3 	RP2C04-0004
	// 100 	4 	2C03 and 2C05
	PPU_VERSION = ((DIP5!=0)<<2) | ((DIP4 != 0)<<1) | (DIP3 != 0);
#endif //#if VS_SYS_ENABLED
	pal_bright(0);
	go_to_state(STATE_BOOT);
	// removed to fix flash at boot.
	//fade_from_black();

#if VS_SRAM_ENABLED
	//one_vram_buffer('0', get_ppu_addr(0, 24, 32));
	// Is primary?
	if (IS_PRIMARY_CPU)
	{
		// // Has control.
		// one_vram_buffer('0' + ((PEEK(0x4016) & (1 << 7))>>7), get_ppu_addr(0, 24, 32));
		// // Take control.
		POKE(0x4016, 2);
		
		if (xram_test[1] != 6)
		{
			xram_test[1] = 6;
			one_vram_buffer(xram_test[1], get_ppu_addr(0, 40, 32));

			// for (i = 0; i < ATTACK_NUM; ++i)
			// //i = 0;
			// {
			// 	for (j = 0; j < 4; ++j)
			// 	//j = 0;
			// 	{
			// 		for (k = 0; k < 3; ++k)
			// 		{
			// 			high_scores_vs_initials[i][j][k][0] = '-';
			// 			high_scores_vs_initials[i][j][k][1] = '-';
			// 			high_scores_vs_initials[i][j][k][2] = '-';
			// 			high_scores_vs_value[i][j][k] = 0;
			// 		}
			// 	}
			// }
			memfill(high_scores_vs_initials, '-', sizeof(high_scores_vs_initials));
			memfill(high_scores_vs_value, 0xff, sizeof(high_scores_vs_value) * 4);

			//high_scores_vs_initials[0][0][0][0] = '-';
			// high_scores_vs_initials[0][0][0][1] = '-';
			// high_scores_vs_initials[0][0][0][2] = '-';
			//high_scores_vs_value[0][0][0] = 0;
		}

		// Reliquish control of SRAM.
		POKE(0x4016, 0);		
	}	
#endif // #if VS_SRAM_ENABLED
	// FlagTopLeft = SpriteManagerAdd(SpriteFlag, 10 << 3, 23 << 3);
	// FlagTopLeft->lim_y = 0xfff; // don't despawn
	// SetSpriteAnim(FlagTopLeft, flag_anim, 15);

	// FlagTopRight = SpriteManagerAdd(SpriteFlag, 22 << 3, 23 << 3);
	// FlagTopRight->lim_y = 0xfff; // don't despawn
	// SetSpriteAnim(FlagTopRight, flag_anim, 15);

/*
	oam_spr(8 << 3, 1 << 3, local_ix, 2);
	oam_spr(24 << 3, 1 << 3, local_ix, 2);
	oam_spr(3 << 3, 10 << 3, local_ix, 0);
	oam_spr(27 << 3, 10 << 3, local_ix, 0);
*/	

	// FlagMidLeft = SpriteManagerAdd(SpriteFlag, 8 << 3, (1 << 3) + 240);
	// FlagMidLeft->lim_y = 0xfff; // don't despawn
	// SetSpriteAnim(FlagMidLeft, flag_anim, 15);

	// FlagMidRight = SpriteManagerAdd(SpriteFlag, 24 << 3, (1 << 3) + 240);
	// FlagMidRight->lim_y = 0xfff; // don't despawn
	// SetSpriteAnim(FlagMidRight, flag_anim, 15);

	// FlagBottomLeft = SpriteManagerAdd(SpriteFlag, 3 << 3, (10 << 3) + 240);
	// FlagBottomLeft->lim_x = 0xfff; // don't despawn
	// FlagBottomLeft->lim_y = 0xfff; // don't despawn
	// SetSpriteAnim(FlagBottomLeft, flag_anim, 15);

	// FlagBottomRight = SpriteManagerAdd(SpriteFlag, 27 << 3, (10 << 3) + 240);
	// FlagBottomRight->lim_x = 0xfff; // don't despawn
	// FlagBottomRight->lim_y = 0xfff; // don't despawn
	// SetSpriteAnim(FlagBottomRight, flag_anim, 15);

	// Get sprite graphics into memory.
	SpriteManagerLoad(SpriteBlock);

	// This don't get enabled until music is actually
	// played, so force them on right at the start.
	NR52_REG = 0x80; //Enables sound, you should always setup this first
	NR51_REG = 0xFF; //Enables all channels (left and right)
	NR50_REG = 0x77; //Max volume

	queued_packet = queued_packet_required = 0;
}

void UPDATE()
{
	//static char num[16];

	ppu_wait_nmi(); // wait till beginning of the frame

	//UIntToString(ticks_in_state_large, num);
	//PRINT(0,0,num);

	//set_music_speed(1);

	++tick_count;
	++tick_count_large;
	++ticks_in_state_large;
	
	// Every frame clear the queued packet.
	queued_packet = queued_packet_required = 0;

	// pad1 = pad_poll(0); // read the first controller
	// pad1_new = get_pad_new(0); // newly pressed button. do pad_poll first

	// pad2 = pad_poll(1);
	// pad2_new = get_pad_new(1);

	// Combine both controllers into one. This is mostly for Vs system, but seems like
	// a nice enough feature for NES as well. Co-op mode!
	pad_all = pad_poll();
	pad_all_new = get_pad_new();

#if VS_SYS_ENABLED

	// Don't start counting credits until the physical counter is cleared
	// from the previous coin.
	if (maintenance_counter == 0)
	{
		// Did the user drop at least 1 credit (maybe more) since the last update?
		// Did the arcade owner press the maintenance button.
		if (CREDITS_QUEUED > 0 || ((prev_4016 & 1<<2) && !(PEEK(0x4016) & 1<<2)))
		{
			if (credits_remaining < 254)
			{
				// Reset the timer so that the "insert coin" display updates
				// this frame.
				tick_count = 0;
				++credits_remaining;

				// If attract music is disabled, the title music will not have been started.
				// If this credit is the amount needed to leave attract mode, trigger the music to
				// starts.
				if ((state == STATE_MENU || state == STATE_HIGH_SCORE_TABLE) && (DIP8 != 0 && credits_remaining == game_cost))
				{
					MUSIC_PLAY_WRAPPER(MUSIC_TITLE);
				}

				// If this was triggered with maintenance button, the dequeue might be 0, and this would
				// send it to 255, which can never be dequeued again.
				if (CREDITS_QUEUED > 0)
				{
					--CREDITS_QUEUED;

					// We need 6 frames to set the counter. 3 on and 3 off.
					maintenance_counter = 6;

					// Set the phyical counter bit.
					// NOTE: Overrides other values. Should this PEEK and OR?
					// NOTE: This seems to have no impact in emulator. The value at $4020 does not change.
					POKE(0x4020, 1);
				}

				// Don't play fx if in the gameplay state, as it could be distracting.
				if (state != STATE_GAME)
				{
					screen_shake_remaining = 5;
					SFX_PLAY_WRAPPER(SOUND_LEVELUP_MULTI);
				}
				
				if (attract_gameplay_enabled && (state == STATE_OPTIONS || state == STATE_GAME))
				{
					fade_to_black();
					go_to_state(STATE_MENU);
					fade_from_black();
				}
			}
		}
	}
	else
	{
		--maintenance_counter;
		// Half way throught the physicial counter sequence.
		// Turn off the bit, but it needs to stay off for 3 more
		// frames.
		if (maintenance_counter == 3)
		{
			POKE(0x4020, 0);
		}
	}

	// Store this for next frame so that we can detect maintenance key released.
	prev_4016 = PEEK(0x4016);
#endif //VS_SYS_ENABLED

	clear_vram_buffer(); // do at the beginning of each frame

	// Quick reset when not on the VS system. I don't think it makes sense to have a quick reset there.
#if !VS_SYS_ENABLED
	if (state != STATE_MENU)
	{
		if (pad_all & PAD_A && pad_all & PAD_B && pad_all & PAD_SELECT && pad_all & PAD_START)
		{
			fade_to_black();
			go_to_state(STATE_MENU);
			fade_from_black();
		}
	}
#endif // !VS_SYS_ENABLED

	switch(state)
	{
		case STATE_BOOT:
		{
			if (tick_count == 120 || pad_all_new & PAD_ALL_BUTTONS)
			{
				fade_to_black();
				go_to_state(STATE_TY);
				//go_to_state(STATE_MENU);
				fade_from_black();
			}
			break;
		}
		case STATE_TY:
		{
			// 120, means wait 240 frames from 120 (previous state).
			if (tick_count == 104 || pad_all_new & PAD_ALL_BUTTONS)
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

			// if (pad1 & PAD_RIGHT)
			// {
			// 	++scroll_x_camera;
			// }
			// if (pad1 & PAD_LEFT)
			// {
			// 	--scroll_x_camera;
			// }			
			// if (pad1 & PAD_DOWN)
			// {
			// 	++scroll_y_camera;
			// }
			// if (pad1 & PAD_UP)
			// {
			// 	--scroll_y_camera;
			// }

			sgb_sound_test();
			
			if (tick_count % 128 == 0)
			{
#if VS_SYS_ENABLED
				if (free_play_enabled) // free play
				{
					multi_vram_buffer_horz(text_free_play, sizeof(text_free_play)-1, get_ppu_addr(0, 8<<3, 12<<3));
				}
				else if (credits_remaining >= game_cost)
				{
					multi_vram_buffer_horz(text_push_start, sizeof(text_push_start)-1, get_ppu_addr(0, 8<<3, 12<<3));
				}
				else
				{
					if (game_cost -  credits_remaining == 1)
					{
						multi_vram_buffer_horz(text_insert_1_coin, sizeof(text_insert_1_coin)-1, get_ppu_addr(0, 8<<3, 12<<3));
					}
					else
					{
						multi_vram_buffer_horz(text_insert_2_coin, sizeof(text_insert_2_coin)-1, get_ppu_addr(0, 8<<3, 12<<3));
					}
				}
#else
				multi_vram_buffer_horz(text_push_start, sizeof(text_push_start)-1, get_ppu_addr(0, 12<<3, 12<<3));
#endif
			}
			else if (tick_count % 128 == 96)
			{
#if VS_SYS_ENABLED
				multi_vram_buffer_horz(clear_push_start, sizeof(clear_push_start)-1, get_ppu_addr(0, 8<<3, 12<<3));
#else
				multi_vram_buffer_horz(clear_push_start, sizeof(clear_push_start)-1, get_ppu_addr(0, 12<<3, 12<<3));
#endif
			}

#if !VS_SYS_ENABLED
			if (pad_all_new != 0)
			{
				if (pad_all_new & konami_code[cur_konami_index])
				{
					++cur_konami_index;
				}
				else
				{
					cur_konami_index = 0;
				}
			}
#endif //#if !VS_SYS_ENABLED
#if VS_SYS_ENABLED
			// 2
			if (pad2_new & (PAD_SELECT))
			{
					fade_to_black();
					auto_forward_leaderboards = 1;
					go_to_state(STATE_HIGH_SCORE_TABLE);
					fade_from_black();
			}
			// Any A or B, or 1, 3, 4.
			else if (((pad1_new & PAD_SELECT) || (pad_all_new & (PAD_START | PAD_A | PAD_B))) && (credits_remaining >= game_cost || free_play_enabled)) // free play
#else

			// Check to see if a connect GB hit start on the main menu.
			UINT8 host_advanced = 0;

			// If the player presses STARRT while selecting
			// the "2 player" option.
			if (pad_all_new & PAD_ALL_CONFIRM && 
				sub_state == 1 && 
				cur_option == 1)
			{
				// Send out the HOST START event. It only does something
				// if we get an ack back.
				_io_out = MP_TITLE_HOST_START;
				// fire and forget
				send_byte();
				while((_io_status == IO_SENDING));
				receive_byte();
			}

			if (_io_status != IO_RECEIVING)
			{
				// Did another sysmte try to start a SIO match?
				if (_io_in == MP_TITLE_HOST_START)
				{
					// Let them know we got it, and move to the 
					// next menu as a CLIENT.
					host_advanced = 1;
					is_host = 0;
					_io_out = MP_TITLE_HOST_ACK;
					// needs to be reliable
					// NOTE: This can likely result in repeat ACK events.
					do
					{
						send_byte();
						while((_io_status == IO_SENDING));
					} while (_io_status != IO_IDLE);
					// jump to the next start if we aren't there already so that
					// it will move to the next scren.
					sub_state = 1;
				}

				// Another device ACK'd our HOST_START event. Officially
				// become the HOST and move to the next menu.
				if (_io_in == MP_TITLE_HOST_ACK)
				{
					host_advanced = 1;
					is_host = 1;
					// jump to the next start if we aren't there already so that
					// it will move to the next scren.
					// This likely will already be 1 but there may be a button mash case.
					sub_state = 1;
				}

				// Start listening again.
				receive_byte();
			}

			if (sub_state == 0)
			{
				if (pad_all & PAD_ALL_CONFIRM)
				{					
					SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);
					sub_state = 1;

					UINT8 p1 = '\x35';
					UINT8 p2 = '\x44';
					UINT8 player = 0x36;

					if (_cpu != CGB_TYPE)	
					{
						p1 = '\x34';
						p2 = '\x43';
						player = 0x35;
					}

					// Show the player options
					UPDATE_TILE_BY_VALUE(7,6, p1, 0x10);
					UPDATE_TILE_BY_VALUE(7,7, p2, 0x10);

					for (local_start_x = 0; local_start_x < 4; ++local_start_x)
					{
						UPDATE_TILE_BY_VALUE(8 + local_start_x,6, player + local_start_x, 0x10);
						UPDATE_TILE_BY_VALUE(8 + local_start_x,7, player + local_start_x, 0x10);
					}
				}
			}
			else if (sub_state == 1) // 1/2 Player State
			{
				if (pad_all_new & PAD_UP)
				{
					if (cur_option == 1)
					{
						SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);
						cur_option = 0;
					}
				}
				else if (pad_all_new & PAD_DOWN)
				{
					if (cur_option == 0)
					{
						SFX_PLAY_WRAPPER(SOUND_MENU_LOW);
						cur_option = 1;
					}
				}

				// Cursor
				sprite_data[1] = ((6) << 3) + SCREEN_START_X - 2 - tenatcle_offsets[(tick_count / 16) % 4];
				sprite_data[0] = ((6 + cur_option) * 7) + SCREEN_START_Y - 1;
				sprite_data[2] = 16;
				sprite_data[3] = 1 | (1 << 4) | (1 << 5);
				memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
				next_oam_idx += sizeof(sprite_data) >> 2;

				// If play player presses START while selecting
				// 1-player, or if there is a host event 
				// (happens on both host and client).
				if ((pad_all_new & PAD_ALL_CONFIRM && cur_option == 0) || host_advanced)
	#endif //VS_SYS_ENABLED
				{

	#if !VS_SYS_ENABLED
					// if (cur_konami_index >= KONAMI_CODE_LEN)
					// {
					// 	SFX_PLAY_WRAPPER(SOUND_LEVELUP_MULTI);
					// 	StopMusic;
					// 	go_to_state(STATE_SOUND_TEST);
					// }
					// else
	#endif //#if !VS_SYS_ENABLED
					{
						// If we got here through an SIO event, this is an SIO game.
						if (host_advanced)
						{
							is_sio_game = 1;
						}
						else
						{
							// This is an offline game so we are always the host.
							is_host = 1;
						}

						sub_state = 2;
					}
				}
			}
			/*else*/ if (sub_state == 2)
			{
				// #define FADE_STEP_FRAMES 8
				// ++scroll_y_camera;
				// ++scroll_y_camera;
				// scroll(0, scroll_y_camera);

				// if ((scroll_y_camera % FADE_STEP_FRAMES) == 0)
				// {
				// 	FadeInStep(scroll_y_camera / FADE_STEP_FRAMES);
				// }

				// if (scroll_y_camera >= (FADE_STEP_FRAMES * 5))
				// {
				// 	//fade_to_black();
				// 	DISPLAY_OFF;
				// 	go_to_state(STATE_OPTIONS);
				// 	fade_from_black();
				// }

				
				SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);

				// Currently this substate isn't actually used. It was put in 
				// place to allow a scroll/fade out but it caused too many
				// problems so I just went back to a straight fade.
				fade_to_black();
				go_to_state(STATE_OPTIONS);
				fade_from_black();				
			}
#if VS_SYS_ENABLED
			// "attract mode" to avoid burn in. Just go back to the start.
			// Timed to be when the title track finishes for a 2nd time.
			if ((credits_remaining < game_cost) && ticks_in_state_large > (15*60*1))
			{
					fade_to_black();
					attract_gameplay_enabled = 1;
					go_to_state(STATE_OPTIONS);
					fade_from_black();
			}
#endif //#if VS_SYS_ENABLED
			break;
		}

		case STATE_OPTIONS:
		{
#if VS_SYS_ENABLED

			// do this first, so the sprites get cleared in the case of auto-advancing.
			if (ticks_in_state_large <= AUTO_FORWARD_DELAY)
			{
				oam_clear();
				temp_secs = ((AUTO_FORWARD_DELAY - ticks_in_state_large)/60);
				digit = (temp_secs) % 10;
				oam_spr(27<<3, 2<<3, '0' + digit, 0);
				temp_secs = temp_secs / 10;
				digit = (temp_secs) % 10;
				oam_spr(26<<3, 2<<3, '0' + digit, 0);
			}

			switch ((option_state))
			{
				case 0:
				{
					if (pad_all_new & PAD_RIGHT)
					{
							if (savegame.attack_style < ATTACK_NUM - 1)
							{
								for (i = 0; i < 4; ++i)
								{
									pal_col(i + (4 * savegame.attack_style), palette_vs_options_inactive[i]);
								}

								++savegame.attack_style;
								SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);

								for (i = 0; i < 4; ++i)
								{
									pal_col(i + (4 * savegame.attack_style), palette_vs_options_active[i]);
								}
							}
					}
					else if (pad_all_new & PAD_LEFT)
					{
							if (savegame.attack_style > 0)
							{
								for (i = 0; i < 4; ++i)
								{
									pal_col(i + (4 * savegame.attack_style), palette_vs_options_inactive[i]);
								}

								--savegame.attack_style;
								SFX_PLAY_WRAPPER(SOUND_MENU_LOW);

								for (i = 0; i < 4; ++i)
								{
									pal_col(i + (4 * savegame.attack_style), palette_vs_options_active[i]);
								}
							}
					}
					if (pad_all_new & (PAD_A | PAD_B | PAD_SELECT | PAD_START) || ticks_in_state_large > AUTO_FORWARD_DELAY)
					{
						fade_to_black();
						oam_clear();
						ppu_off();
						vram_adr(NTADR_A(0,0));
						vram_unrle(vs_settings_difficulty);
						ppu_on_all();
						option_state = 1;
						for (i = 0; i < 4; ++i)
						{
							pal_col(i + (4 * savegame.attack_style), palette_vs_options_inactive[i]);
						}
						for (i = 0; i < 4; ++i)
						{
							pal_col(i + (4 * cur_level_vs_setting), palette_vs_options_active[i]);
						}
						fade_from_black();
					}
					break;
				}

				case 1:
				{
					if (vs_code_index < VS_CODE_LEN)
					{
						if (pad_all_new != 0)
						{
							if (pad_all_new & vs_code[vs_code_index])
							{
								++vs_code_index;
								if (vs_code_index == VS_CODE_LEN)
								{
									StopMusic;
									SFX_PLAY_WRAPPER(SOUND_LEVELUP_MULTI)
									cur_level_vs_setting = 3;
									pal_bg(palette_vs_options_skulls);
									ticks_in_state_large = MAX(ticks_in_state_large, AUTO_FORWARD_DELAY - (5*60));
								}
							}
							else
							{
								vs_code_index = 0;
							}
						}
					}
					else
					{
						for (i = 0; i < 8; ++i)
						{
							one_vram_buffer(0x01, get_ppu_addr(0, (unsigned int)rand() % 256, (unsigned char)rand() % 240));
						}
					}

					if (vs_code_index < VS_CODE_LEN && pad_all_new & PAD_RIGHT)
					{
							if (cur_level_vs_setting < 2)
							{
								for (i = 0; i < 4; ++i)
								{
									pal_col(i + (4 * cur_level_vs_setting), palette_vs_options_inactive[i]);
								}

								++cur_level_vs_setting;
								SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);

								for (i = 0; i < 4; ++i)
								{
									pal_col(i + (4 * cur_level_vs_setting), palette_vs_options_active[i]);
								}
							}
					}
					else if (vs_code_index < VS_CODE_LEN && pad_all_new & PAD_LEFT)
					{
							if (cur_level_vs_setting > 0)
							{
								for (i = 0; i < 4; ++i)
								{
									pal_col(i + (4 * cur_level_vs_setting), palette_vs_options_inactive[i]);
								}

								--cur_level_vs_setting;
								SFX_PLAY_WRAPPER(SOUND_MENU_LOW);

								for (i = 0; i < 4; ++i)
								{
									pal_col(i + (4 * cur_level_vs_setting), palette_vs_options_active[i]);
								}
							}
					}
					if (pad_all_new & (PAD_A | PAD_B | PAD_SELECT | PAD_START) || (ticks_in_state_large > AUTO_FORWARD_DELAY))
					{
						StopMusic;
						SFX_PLAY_WRAPPER(SOUND_START);

						// 0, 9, 19, 29.
						// MAX to ensure setting 0 doesn't wrap around.
						cur_level = MAX(1, (cur_level_vs_setting * 10)) - 1;

						fade_to_black();
						ppu_off();
						// prevent vram changes showing up in gameplay.
						clear_vram_buffer();
						vram_adr(NTADR_A(0,0));
						vram_unrle(title_screen);
						ppu_on_all();
						go_to_state(STATE_GAME);
						fade_from_black();
					}

					break;
				}

				default:
					break;
			}
#else
			if (tick_count % 128 == 0)
			{
				multi_vram_buffer_horz(text_push_start, sizeof(text_push_start)-1, get_ppu_addr(0, 12<<3, 12<<3));
			}
			else if (tick_count % 128 == 96)
			{
				multi_vram_buffer_horz(clear_push_start, sizeof(clear_push_start)-1, get_ppu_addr(0, 12<<3, 12<<3));
			}

			// Check if the host advanced to gameplay.
			// TODO: This should probably require both 
			UINT8 host_advanced = 0;
			if (is_sio_game && _io_status != IO_RECEIVING)
			{
				if (_io_in & MP_OPTIONS_MENU_ADVANCE != 0)
				{
					host_advanced = 1;
				}

				// Don't start listening again until we are in the go_to_state section
				// that handle the event, otherwise we could get the next send before
				// we are ready for it.
				// receive_byte();
			}

			// Only accept "Start" because holding "A" means skip 10 levels.
			if ((pad_all_new & PAD_START && is_host) || host_advanced)
			{
				if (!host_advanced)
				{
					queued_packet |= MP_OPTIONS_MENU_ADVANCE;
					queued_packet_required = 1;
					send_queued_packet();
				}

				StopMusic;
				SFX_PLAY_WRAPPER(SOUND_START);

				//fade_to_black();
				//ppu_off();
#if PLAT_GB
				//vram_unrle(title_and_game_area);
				//InitScroll(BANK(title_and_game_area), &title_and_game_area, 0, 0);
				// for (local_ix = 8; local_ix <= 40; local_ix+=8)
				// {
				// 	scroll(local_ix, 0);
				// }
#else
				vram_adr(NTADR_A(0,0));
				vram_unrle(title_screen);
#endif // PLAT_GB
				//ppu_on_all();
				//fade_from_black();

				// little cheat to start at very high levels.
				if (cur_level == 9 && pad_all & PAD_SELECT)
				{
					cur_level = 29;
				}
				else if (pad_all & PAD_A)
				{
					cur_level += 10;
				}
				go_to_state(STATE_GAME);

				// skip the drawing at the end of the case.
				break;
			}

			// Don't allow backing out in multiplayer. This could be added, but will require
			// a packet to tell the client to back out too.
			if (pad_all_new & PAD_B && !is_sio_game)
			{
				fade_to_black();
				go_to_state(STATE_MENU);
				fade_from_black();

				// skip the drawing at the end of the case.
				break;
			}
			else if (pad_all_new & PAD_RIGHT)
			{
				switch (cur_option)
				{

				case 1: // starting level

					if (cur_level < 9 )
					{
						++cur_level;
					}
					else
					{
						cur_level = 0;
					}
					break;

				case 0: // Attack style

					//savegame.attack_style = (savegame.attack_style + 1) % ATTACK_NUM;

					if (savegame.attack_style < ATTACK_NUM - 1)
					{
						++savegame.attack_style;
						display_highscore();
					}
					break;

				case 2: // Music off/on

					//savegame.music_on = (savegame.music_on + 1) % 2;

					if (savegame.music_on == 0)
					{
						savegame.music_on = 1;
						MUSIC_PLAY_ATTRACT_WRAPPER(MUSIC_TITLE);
						music_pause(0);
					}

					// if (savegame.music_on == 0)
					// {
					// 	StopMusic;
					// }
					// else
					// {
					// 	music_play(MUSIC_TITLE);
					// }
					break;

				case 3: // sound fx on/off
				{
					if (savegame.sfx_on == 0)
					{
						savegame.sfx_on = 1;
					}
					break;
				}

				case 4: // hard drops
				{
					if (savegame.hard_drops_on < NUM_HARD_DROP_SETTINGS - 1)
					{
						++savegame.hard_drops_on;
					}
				}

				default:
					break;
				}

				SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);
				display_options();
			}
			else if (pad_all_new & PAD_LEFT)
			{
				switch (cur_option)
				{

				case 1: // starting level

					if (cur_level != 0)
					{
						--cur_level;
					}
					else
					{
						cur_level = 9;
					}
					break;

				case 0: // Attack style
					// if (savegame.attack_style == 0)
					// {
					// 	savegame.attack_style = ATTACK_NUM;
					// }
					// savegame.attack_style = (savegame.attack_style - 1) % ATTACK_NUM;

					if (savegame.attack_style != 0)
					{
						--savegame.attack_style;
						display_highscore();
					}

					break;

				case 2: // Music off/on
					// if (savegame.music_on == 0)
					// {
					// 	savegame.music_on = 2;
					// }
					// savegame.music_on = (savegame.music_on - 1) % 2;

					if (savegame.music_on != 0)
					{
						savegame.music_on = 0;
						music_pause(1);
						StopMusic;
					}

					// if (savegame.music_on == 0)
					// {
					// 	StopMusic;
					// }
					// else
					// {
					// 	music_play(MUSIC_TITLE);
					// }

					break;

				case 3: // sound fx off/on
				{
					if (savegame.sfx_on != 0)
					{
						savegame.sfx_on = 0;
					}
					break;
				}
				case 4: // hard drops
				{
					if (savegame.hard_drops_on != 0)
					{
						--savegame.hard_drops_on;
					}
					break;
				}

				default:
					break;
				}

				SFX_PLAY_WRAPPER(SOUND_MENU_LOW);
				display_options();
			}
			else if (pad_all_new & PAD_DOWN)
			{
				cur_option = (cur_option + 1) % NUM_OPTIONS;
				SFX_PLAY_WRAPPER(SOUND_MENU_LOW);
				display_options();
			}
			else if (pad_all_new & PAD_UP)
			{
				if (cur_option == 0)
				{
					cur_option = NUM_OPTIONS;
				}
				cur_option = (cur_option - 1) % NUM_OPTIONS;
				SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);
				display_options();
			}

			// mode at the top
			local_start_y = 3;

			// jump down to the other options.
			if (cur_option > 0)
			{
				local_start_y = cur_option + 6;
			}

			sprite_data[1] = (3 << 3) + SCREEN_START_X - 5 + tenatcle_offsets[(tick_count / 16) % 4];
			sprite_data[0] = (local_start_y * 7) + SCREEN_START_Y;
			sprite_data[2] = 16;
			sprite_data[3] = 1;
			memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
			next_oam_idx += sizeof(sprite_data) >> 2;

			sprite_data[1] = (4 << 3) + SCREEN_START_X - 2 - tenatcle_offsets[(tick_count / 16) % 4];
			sprite_data[0] = (local_start_y * 7) + SCREEN_START_Y;
			sprite_data[2] = 16;
			sprite_data[3] = 1 | (1 << 5);
			memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
			next_oam_idx += sizeof(sprite_data) >> 2;
#endif
			break;
		}

		case STATE_GAME:
		{
			packet_in = 0;
			UINT8 garbage_rows = 0;
			UINT8 other_lost = 0;
			UINT8 other_highwater = 0;

			if (level_up_remaining > 0)
			{
				level_up_remaining = 0;
				// if (level_up_remaining % 8 > 3)
				// {
				// 	//set_palette_entry(1, 0, 0x7fff);
				// 	set_bkg_palette(1, 1, palette_flash);
				// 	BGP_REG = PAL_DEF(1, 2, 3, 3);
				// }
				// else
				// {
				// 	//set_palette_entry(1, 0, 0x7fff);
				// 	set_bkg_palette(1, 1, palette_normal);
				// 	BGP_REG = PAL_DEF(0, 1, 2, 3);
				// }

				// --level_up_remaining;
			}

			if (is_sio_game && _io_status != IO_RECEIVING)
			{
				packet_in = _io_in;

				garbage_rows = (packet_in & MP_GAME_GARBAGE_MASK);
				other_highwater = (packet_in & MP_GAME_HIGHWATER_MASK) >> MP_GAME_HIGHWATER_SHIFT;
				other_lost = (packet_in & MP_GAME_OTHER_LOST) >> MP_GAME_OTHER_LOST_SHIFT;

				++packet_count_in;
	#if DEBUG_ENABLED
				PRINT_POS(0,4);
				Printf("I:%d %d %d", other_highwater, garbage_rows, other_lost);
	#endif // DEBUG_ENABLED
				// Start listening again...
				receive_byte();
			}

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

			// delay a frame for perf.
			if (savegame.attack_style != ATTACK_NEVER && attack_queued)
			{
//PROFILE_POKE(PROF_R);
				// TODO: Perf - Very expensive.
				add_block_at_bottom();
//PROFILE_POKE(PROF_W);
				clear_rows_in_data(BOARD_END_Y_PX_BOARD);
				attack_queued = 0;
				attack_queue_ticks_remaining = attack_delay;
			}

			if (kill_row_queued)
			{
				add_row_at_bottom();
				kill_row_queued = 0;
			}

//PROFILE_POKE(PROF_G);

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

//PROFILE_POKE(PROF_B);

			// Handle the case where we hit game over earlier in this thread.
			if (state == STATE_GAME)
			{
				draw_gameplay_sprites();
			}

//PROFILE_POKE(PROF_W);

			if (savegame.attack_style == ATTACK_ON_TIME && attack_queue_ticks_remaining != 0)
			{
				--attack_queue_ticks_remaining;

				if (attack_queue_ticks_remaining == 0)
				{
					attack_queued = 1;
					attack_queue_ticks_remaining = attack_delay;
				}
			}

			// STRESS MUSIC!

#if VS_SYS_ENABLED
			if (!attract_gameplay_enabled)
#endif // #if VS_SYS_ENABLED				
			{
				local_t = 0;
				for (local_iy = 0; local_iy < STRESS_MUSIC_LEVEL * 10; ++local_iy)
				{
					if (game_board[local_iy + ((BOARD_OOB_END + 1) * 10)] != EMPTY_TILE)
					{
						// music is stressed even if it doesn't start playing this frame.
						local_t = 1;

						if (cur_gameplay_music == GAMEPLAY_MUSIC_NORMAL)
						{
							cur_gameplay_music = GAMEPLAY_MUSIC_STRESS;
							MUSIC_PLAY_WRAPPER(MUSIC_STRESS);
							break;
						}
					}
				}

				if (local_t == 0 && cur_gameplay_music == GAMEPLAY_MUSIC_STRESS)
				{
					cur_gameplay_music = GAMEPLAY_MUSIC_NORMAL;
					MUSIC_PLAY_WRAPPER(MUSIC_GAMEPLAY);
				}
			}

// No pause in the arcade, fool!
#if !VS_SYS_ENABLED
			if (pad_all_new & PAD_START && !is_sio_game)
			{
				go_to_state(STATE_PAUSE);
			}
#endif // !VS_SYS_ENABLED

#if DEBUG_ENABLED
			// if (pad_all_new & PAD_START)
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

#if HIGHWATER_ON

			// bar goes from [8,19] up to [8, 0]
			// 19 for left side.

			// Check each row of the board, starting from the top, looking for the first,
			// non-empty row. That is the high water mark that should be sent to the other
			// player.
			if (is_sio_game)
			{
				// Is this row NOT empty?
				if (memcmp(cur_high_water_row, empty_row, BOARD_WIDTH) != 0)
				{
					// Has the high water changed?
					// NOTE: Since a value of 0 is used to indicate an "empty"
					// queued_pakcet, we use 1 more than the actual highwater.
					// It will be subtracted on the recieving end.
					if (prev_high_water != (high_water_y + 1))
					{
						prev_high_water = high_water_y + 1;
						queued_packet |= (prev_high_water << MP_GAME_HIGHWATER_SHIFT);
					}
					// PRINT_POS(0,0);
					// Printf("P1:%d", BOARD_HEIGHT - high_water_y);
					
					// Got back to the top of the board and start again.
					high_water_y = 0;
					cur_high_water_row = &game_board[((BOARD_OOB_END + 1) * BOARD_WIDTH)];
				}
				else
				{
					// This row was empty, so next frame check the row below it.
					++high_water_y;
					cur_high_water_row += BOARD_WIDTH; // pointer arithmetic

					// When we hit the end of the game board, go back to the top.
					// We don't send this as an event, so technically this means that
					// if the player was able to completely clear the play area, it will
					// not be refected by the other player, but that's almost impossible
					// and pretty unimportant.
					if (high_water_y >= BOARD_HEIGHT) 
					{ 
						cur_high_water_row = &game_board[((BOARD_OOB_END + 1) * BOARD_WIDTH)];
						high_water_y = 0; 
						prev_high_water = (BOARD_HEIGHT + 1); 
					}
				}

				// Check if the incoming packet had a high water for the other player.
				if (other_highwater != 0)
				{
					UINT8 h = BOARD_HEIGHT - (other_highwater - 1);
					// PRINT_POS(0,1);
					// Printf("P2:%d", h);

					// Loop through the column showing the high water, updating the
					// tiles to reflect it. This is brute force, and doesn't try to 
					// reduce the number of tiles that get changed.
					for (local_iy = 0; local_iy < BOARD_HEIGHT; ++local_iy)
					{
						if (local_iy < h)
						{
							UPDATE_TILE_BY_VALUE(8, 19 - local_iy, 135, 0x10);
						}
						else
						{
							UPDATE_TILE_BY_VALUE(8, 19 - local_iy, 136, 0x10);
						}
					}
				}
			}
#endif

			if (garbage_rows > 0)
			{
				// Rows counts come in as 1 minus the amount
				// cleared by the other player. This is how 
				// many garbage rows should be added, except for 
				// the case of 4 rows, which should add 4 as a
				// special bonus.
				if (garbage_rows == 3)
				{
					garbage_rows = 4;
				}
				garbage_row_queue += garbage_rows;
				
				SFX_PLAY_WRAPPER(SOUND_BLOCKED);
			}

			if (other_lost && is_sio_game)
			{
				++rounds_won;
				go_to_state(STATE_OVER);

				PRINT(10, 4, "YOU WIN!");
			}

			send_queued_packet();
//PROFILE_POKE(PROF_CLEAR);
			break;
		}

#if !VS_SYS_ENABLED		
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

			draw_pause_sprites();


			if (pad_all_new & PAD_START)
			{
				go_to_state(STATE_GAME);
			}
			break;
		}
#endif //#if !VS_SYS_ENABLED		
		case STATE_OVER:
		{
#if VS_SYS_ENABLED
			if (ticks_in_state_large > (60*10) || (pad_all_new & (PAD_B | PAD_A | PAD_SELECT | PAD_START)))
			{
				// always go through the high score table, but sometimes it will be waiting to have you enter
				// your initials.
				fade_to_black();
				go_to_state(STATE_HIGH_SCORE_TABLE);
				fade_from_black();
			}
#else

			queued_packet = queued_packet_required = 0;

			packet_in = 0;
			UINT8 menu_choice_quit = 0;
			UINT8 menu_choice_replay = 0;

			if (is_sio_game && _io_status != IO_RECEIVING)
			{
				packet_in = _io_in;

				if (packet_in & MP_OVER_QUIT)
				{
					menu_choice_quit = 1;
				}
				else if (packet_in & MP_OVER_REPLAY)
				{
					menu_choice_replay = 1;
				}

				++packet_count_in;
				// PRINT_POS(0,4);
				// Printf("I:%d %d    ", menu_choice_quit, menu_choice_replay);

				// Start listening again...
				receive_byte();
			}

			if (pad_all_new & PAD_B || menu_choice_quit)
			{
				// Don't resend the same event back.
				if (!menu_choice_quit)
				{
					queued_packet |= MP_OVER_QUIT;
					send_queued_packet();
				}
				//go_to_state(STATE_GAME);
				fade_to_black();
				go_to_state(STATE_MENU);
				fade_from_black();
			}
			if (pad_all_new & PAD_A || menu_choice_replay)
			{
				if (!menu_choice_replay)
				{
					queued_packet |= MP_OVER_REPLAY;
					send_queued_packet();
				}
				//go_to_state(STATE_GAME);
				go_to_state(STATE_GAME);
			}
#endif

			send_queued_packet();

			break;
		}

#if !VS_SYS_ENABLED		
		case STATE_SOUND_TEST:
		{
			// MUSIC
			//

			if (pad_all_new & PAD_DOWN && test_song < 5)
			{
				++test_song;
				display_song();
			}
			else if (pad_all_new & PAD_UP && test_song > 0)
			{
				--test_song;
				display_song();
			}

			if (pad_all_new & PAD_B)
			{
				if (test_song == test_song_active)
				{
					test_song_active = 0xff;
					StopMusic;
				}
				else
				{
					#define FORCE_PLAY(id) PlayMusic(id, 1)
					#define FORCE_PLAY_ONCE(id) PlayMusic(id, 0)
					test_song_active = test_song;
					// hack
					switch (test_song)
					{
					case 0:
						// ignore settings.
						FORCE_PLAY(MUSIC_TITLE);
						break;
					case 1:
						FORCE_PLAY(MUSIC_GAMEPLAY);
						break;
					case 2:
						FORCE_PLAY(MUSIC_STRESS);
						break;
					case 3:
						FORCE_PLAY(MUSIC_PAUSE);
						break;
					case 4:
						FORCE_PLAY_ONCE(MUSIC_GAMEOVER_INTRO);
						test_song_active = 0xff;
						break;
					case 5:
						FORCE_PLAY_ONCE(MUSIC_GAMEOVER_OUTRO);
						test_song_active = 0xff;
						break;
					
					default:
						break;
					}
					music_play(test_song);
				}
			}


			// SOUND
			//

			if (pad_all_new & PAD_RIGHT && test_sound < 31)
			{
				++test_sound;
				display_sound();
			}
			else if (pad_all_new & PAD_LEFT && test_sound > 0)
			{
				--test_sound;
				display_sound();
			}

			if (pad_all_new & PAD_A)
			{
				// Intentionally not using wrapper so this plays regardless of settings.
				sfx_play(test_sound, 0);
			}

			// EXIT
			//

			if (pad_all_new & PAD_SELECT || pad_all_new & PAD_START)
			{
				go_to_state(STATE_MENU);
			}
			break;
		}
#endif // #if !VS_SYS_ENABLED		

#if VS_SYS_ENABLED
		case STATE_HIGH_SCORE_TABLE:
		{
			oam_clear();
			if (IS_PRIMARY_CPU && high_score_entry_placement < 3)
			{
#if VS_SRAM_ENABLED					
				// take control of SRAM.
				POKE(0x4016, 2);
#endif // #if VS_SRAM_ENABLED
				// Save a bunch of code space, and make this easier to work with.
				temp_table = high_scores_vs_initials[savegame.attack_style][cur_level_vs_setting][high_score_entry_placement];

				if (temp_table[cur_initial_index] == '-')
				{
					temp_table[cur_initial_index] = last_initials[cur_initial_index];
				}

				// output to in_x, in_y
				difficulty_to_leaderboard_pos (cur_level_vs_setting);

				in_id = (ticks_in_state_large % 128 < 64) ? 0 : 2;

				oam_spr((in_x + 0) << 3, (in_y + high_score_entry_placement) << 3, temp_table[0],  (cur_initial_index == 0) ? in_id : 2);
				if (cur_initial_index > 0)
				{
					oam_spr((in_x + 1) << 3, (in_y + high_score_entry_placement) << 3, temp_table[1], (cur_initial_index == 1) ? in_id : 2);
				}
				if (cur_initial_index > 1)
				{
					oam_spr((in_x + 2) << 3, (in_y + high_score_entry_placement) << 3, temp_table[2], in_id);
				}

				if (pad_all_new & PAD_RIGHT)
				{
					// Reset the timer on movement, like on a modern OS.
					ticks_in_state_large = 0;

					// Increment the character. Works because A-Z is in order.
					++temp_table[cur_initial_index];

					i = temp_table[cur_initial_index];

					// NOTE: multiple ifs takes less space that if else if
					if (i > 0x5a) // passed Z
					{
						temp_table[cur_initial_index] = 0x2e; // .
					}
					if (i == 0x2f) // passed .
					{
						temp_table[cur_initial_index] = '0';
					}
					if (i == 0x3a) // passed 9
					{
						temp_table[cur_initial_index] = 'A';
					}
				}
				else if (pad_all_new & PAD_LEFT)
				{
					ticks_in_state_large = 0;
					--temp_table[cur_initial_index];

					i = temp_table[cur_initial_index];

					if (i < 0x2e) // passed .
					{
						temp_table[cur_initial_index] = 'Z';
					}
					if (i == 0x2f) // passed 0
					{
						temp_table[cur_initial_index] = 0x2e;
					}
					if (i == 0x40) // passed A
					{
						temp_table[cur_initial_index] = '9';
					}
				}
				else if ((pad_all_new & PAD_A) || (ticks_in_state_large > AUTO_FORWARD_DELAY))
				{
					ticks_in_state_large = 0;

					// If the users changes away from the default, assume they don't want to continue
					// using the previous initials.
					if (temp_table[cur_initial_index] != last_initials[cur_initial_index])
					{
						// NOTE: last_initials doesn't get populated for this entry until the end of
						//		 the name entry sequence.
						memfill(last_initials, 'A', 3);
					}

					// Overflow caught below.
					++cur_initial_index;
				}
				else if (pad_all_new & PAD_B)
				{
					if (cur_initial_index > 0)
					{
						//ticks_in_state_large = 0;
						temp_table[cur_initial_index] = '-';
						--cur_initial_index;
					}
				}

				// If the user takes to long to enter their initials auto complete it.
				// NOTE: The tick counter is reset every time they press a button, so it's really just
				//		 here to handle cases where they walk away in the middle of entering initials.
				if (cur_initial_index >= 3)
				{
					memcpy(last_initials, temp_table, 3);
					SFX_PLAY_WRAPPER(SOUND_LEVELUP);
					fade_to_black();
					auto_forward_leaderboards = 1;
					go_to_state(STATE_HIGH_SCORE_TABLE);
					fade_from_black();
				}
#if VS_SRAM_ENABLED
				// Reliquish control.
				POKE(0x4016, 0);	
#endif // #if VS_SRAM_ENABLED
			}
			else 
			{
				// arrow sprites.
				oam_spr(80 + tenatcle_offsets[(tick_count/16) % 4], 24, 14, 0);
				oam_spr(160 - tenatcle_offsets[(tick_count/16) % 4], 24, 14, 0|OAM_FLIP_H);

				if (auto_forward_leaderboards && ticks_in_state_large > (60*10))
				{
					if (savegame.attack_style > 0)
					{
						--savegame.attack_style;
					}
					else
					{
						savegame.attack_style = ATTACK_NUM - 1;
					}

					--auto_forward_leaderboards;

					if (auto_forward_leaderboards == 0)
					{
						fade_to_black();
						go_to_state(STATE_MENU);
						fade_from_black();
					}
					else
					{
						ticks_in_state_large = 0;
						fade_to_black();
						go_to_state(STATE_HIGH_SCORE_TABLE);
						fade_from_black();
					}
				}
				else if ( pad_all_new & PAD_LEFT)
				{
					if (savegame.attack_style > 0)
					{
						--savegame.attack_style;
					}
					else
					{
						savegame.attack_style = ATTACK_NUM - 1;
					}

					auto_forward_leaderboards = 1;
					fade_to_black();
					go_to_state(STATE_HIGH_SCORE_TABLE);
					fade_from_black();
				}
				else if ( pad_all_new & PAD_RIGHT)
				{
					if (savegame.attack_style < ATTACK_NUM - 1)
					{
						++savegame.attack_style;
					}
					else
					{
						savegame.attack_style = 0;
					}

					auto_forward_leaderboards = 1;
					fade_to_black();
					go_to_state(STATE_HIGH_SCORE_TABLE);
					fade_from_black();
				}
				// else if (pad_all_new & PAD_A)
				// {
				// 	// stop auto-forwarding.
				// 	auto_forward_leaderboards = 0;
				// }
				else if (pad_all_new & (PAD_A | PAD_B | PAD_SELECT | PAD_START))
				{
					fade_to_black();
					go_to_state(STATE_MENU);
					fade_from_black();
				}
			}
			break;
		}
#endif // #if VS_SYS_ENABLED
	}

//#if VS_SYS_ENABLED
			// Yeah, this game has screen shake for inserting credits?? So what!?
			// if (screen_shake_remaining > 0)
			// {
			// 	--screen_shake_remaining;
			// 	scroll(((unsigned char)rand() % 2), scroll_y_game - ((unsigned char)rand() % 2));
			// }
			// else
			// {
			// 	//++scroll_y_game;
			// 	scroll(0 + scroll_x_camera, scroll_y_game + scroll_y_camera); // shift the bg down 1 pixel
			// }

			// set_scroll_x(scroll_y_game);
			// ++scroll_y_game;
//#endif	// VS_SYS_ENABLED
}


#if PLAT_NES
void draw_menu_sprites(void)
{
	static unsigned char t;
#if VS_SYS_ENABLED
	static unsigned char d;
#endif

	// clear all sprites from sprite buffer
	oam_clear();

	// FLAGS
	t = tick_count % 64;
	if (t > 48)
	{
		local_ix = 0x69;
	}
	else if (t > 32)
	{
		local_ix = 0x68;
	}
	else if (t > 16)
	{
		local_ix = 0x67;
	}
	else
	{
		local_ix = 0x66;
	}

	oam_spr(10 << 3, 23 << 3, local_ix, 0);
	oam_spr(22 << 3, 23 << 3, local_ix, 0);

	// TENTACLES
	oam_meta_spr(19<<3, 14<<3, metasprite_tentacle_title);


#if VS_SYS_ENABLED
	// if (!free_play_enabled)
	// {
		//3<<3, 26<<3
		t = credits_remaining;
		d = (t) % 10;
		oam_spr(6<<3, 27<<3, '0' + d, 0);
		t = t / 10;
		d = (t) % 10;
		oam_spr(5<<3, 27<<3, '0' + d, 0);

		oam_spr(7 << 3, 27 << 3, 0x2F, 0);
		oam_spr(8 << 3, 27 << 3, 0x30 + game_cost, 0);
	// }

	oam_meta_spr(22<<3, 3<<3, metasprite_vs_logo);
//	oam_meta_spr(27<<3, 27<<3, metasprite_button2);

#endif //VS_SYS_ENABLED
}

#endif // PLAT_NES

void draw_gameplay_sprites(void)
{
	static char shake_offset;
	static unsigned char speed;
	static unsigned char i;
	static unsigned char j;

	// steady moving sprite to find hitches
	// sprite_data[1] = SCREEN_START_X + tick_count_large % 160;
	// sprite_data[0] = SCREEN_START_Y;
	// sprite_data[2] = 0; // put it into the sprite memory.
	// sprite_data[3] = 0;
	// memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
	// next_oam_idx += sizeof(sprite_data) >> 2;

//PROFILE_POKE(0x5f); // green
	// clear all sprites from sprite buffer
	//oam_clear();
//PROFILE_POKE(0x9f); // blue
	// push a single sprite
	// oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned char attr);
	// use tile #0, palette #0

	local_start_x = (cur_block.x << 3) + BOARD_START_X_PX;
	local_start_y = (cur_block.y * 7) + BOARD_START_Y_PX;

	// 255 means hide.
	if (cur_block.y != 255)
	{
		for (i = 0; i < 4; ++i)
		{
			// store the index into the x,y offset for each solid piece in the first rotation.
			j = cur_cluster.layout[i];

			// conver that to x,y offsets.
			local_ix = index_to_x_lookup[j];
			local_iy = index_to_y_lookup[j];

			// Don't draw the current cluster if it is above the top of the board.
			// We want it to be able to function and move up there, but should not
			// be visible.
			// TODO: * 7 via lookup table.
			if (local_start_y + (local_iy * 7) > OOB_TOP)
			{
				//oam_spr(local_start_x + (local_ix << 3), local_start_y + (local_iy << 3), cur_cluster.sprite, 0);
				sprite_data[1] = local_start_x + (local_ix << 3) + SCREEN_START_X;
				// Note sure why -30. -32 was to account for the non-visible
				// OOB area, but that resulted in sprites that looked like they
				// we offset by a few more pixels than they should be.
				// Using 30 instead looks right for some reason.
				sprite_data[0] = local_start_y + (local_iy * 7) - 29 + SCREEN_START_Y;
				sprite_data[2] = cur_cluster.sprite - 128; // put it into the sprite memory.
				sprite_data[3] = 0;
				memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
				next_oam_idx += sizeof(sprite_data) >> 2;
			}
		}
	}

//PROFILE_POKE(0x1f); // white

	if (savegame.attack_style != ATTACK_NEVER)
	{
		shake_offset = 0;
		if (savegame.attack_style == ATTACK_ON_TIME)
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
				for (local_iy = local_row_status - 1; local_iy < local_row_status /*&& local_iy < ATTACK_QUEUE_SIZE*/; --local_iy)
				{
					//if (attack_queue_ticks_remaining < 120)
					{
						//if (attack_queue_ticks_remaining % 8 == 0) // %8
						{
							//shake_offset = ((local_iy + (attack_queue_ticks_remaining << 4)) % 3) - 1;
						}
					}

					shake_offset = tenatcle_offsets[((local_iy + speed) & 3)]; // &3 = %4 = number of entries in array.

					sprite_data[1] = BOARD_START_X_PX + (local_ix << 3) + shake_offset + SCREEN_START_X;
					sprite_data[0] = (BOARD_END_Y_PX) + (ATTACK_QUEUE_SIZE * 7) - (local_iy * 7) + SCREEN_START_Y;
					sprite_data[2] = 0x7; // put it into the sprite memory.
					sprite_data[3] = 0x1;

					// gross. Try to detect if this is the last piece, and alsothe end of the arm.
					if (local_iy == local_row_status - 1)
					{
						sprite_data[2] = 0x8;
					}

					if (local_iy < 3)
					{
						sprite_data[0] -= ((2 - local_iy) * 7) + (local_iy * 2) - 2;
					}

					memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
					next_oam_idx += sizeof(sprite_data) >> 2;

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
		// oam_spr((3 << 3) /*+ r*/, (24 << 3), 0x65, 1);
		// oam_spr(3 << 3, 25 << 3, 0x64, 1);
		// oam_spr(3 << 3, 26 << 3, 0x74, 1);	

		sprite_data[1] = (3 << 3) + SCREEN_START_X;
		sprite_data[0] = (16 * 7) + (UINT8)SCREEN_START_Y - 1U;
		sprite_data[2] = 15; // put it into the sprite memory.
		sprite_data[3] = 1;
		memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
		next_oam_idx += sizeof(sprite_data) >> 2;

		sprite_data[1] = (3 << 3) + SCREEN_START_X;
		sprite_data[0] = (UINT8)(17 * 7) + (UINT8)SCREEN_START_Y - 1U;
		sprite_data[2] = 11; // put it into the sprite memory.
		sprite_data[3] = 1 | (1 << 4);
		memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
		next_oam_idx += sizeof(sprite_data) >> 2;

		sprite_data[1] = (3 << 3) + SCREEN_START_X;
		sprite_data[0] = (18 * 7) + (UINT8)SCREEN_START_Y - 1U;
		sprite_data[2] = 14; // put it into the sprite memory.
		sprite_data[3] = 1 | (1 << 4);
		memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
		next_oam_idx += sizeof(sprite_data) >> 2;		

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
		if (savegame.attack_style == ATTACK_NEVER)
		{
			// sleeping
			// oam_spr(3 << 3, 25 << 3, 0x63, 1);
			// oam_spr(3 << 3, 26 << 3, 0x73, 1);

			sprite_data[1] = (3 << 3) + SCREEN_START_X;
			sprite_data[0] = (UINT8)(17 * 7) + (UINT8)SCREEN_START_Y - 1U;
			sprite_data[2] = 10; // put it into the sprite memory.
			sprite_data[3] = 1 | (1 << 4);
			memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
			next_oam_idx += sizeof(sprite_data) >> 2;

			sprite_data[1] = (3 << 3) + SCREEN_START_X;
			sprite_data[0] = (18 * 7) + (UINT8)SCREEN_START_Y - 1U;
			sprite_data[2] = 13; // put it into the sprite memory.
			sprite_data[3] = 1 | (1 << 4);
			memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
			next_oam_idx += sizeof(sprite_data) >> 2;
		}
		else
		{

			//pal_spr(palette_sp);
			//pal_bg(palette_bg);
			local_t = tick_count_large % BLINK_LEN;

			// if (pad_all_new & PAD_RIGHT)
			// {
			// 	++scroll_x_camera;

			// 	if (scroll_x_camera > 3)
			// 	{
			// 		scroll_x_camera = 0;
			// 	}
			// }

			if (local_t > BLINK_LEN - 10)
			{
				// oam_spr(3 << 3, 25 << 3, 0x62, 1);
				// oam_spr(3 << 3, 26 << 3, 0x72, 1);
				
				sprite_data[1] = (3 << 3) + SCREEN_START_X; //12<<3
				sprite_data[0] = (17U * 7U) + SCREEN_START_Y - 1U; // (17 * 7)
				sprite_data[2] = 9; // put it into the sprite memory.
				sprite_data[3] = 1 | (1 << 4);
				memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
				next_oam_idx += sizeof(sprite_data) >> 2;

				sprite_data[1] = (3 << 3) + SCREEN_START_X;
				sprite_data[0] = (18U * 7U) + SCREEN_START_Y - 1U;
				sprite_data[2] = 12; // put it into the sprite memory.
				sprite_data[3] = 1 | (1 << 4);
				memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
				next_oam_idx += sizeof(sprite_data) >> 2;
			}
			else if (local_t > (BLINK_LEN - 20))
			{
				// oam_spr(3 << 3, 25 << 3, 0x63, 1);
				// oam_spr(3 << 3, 26 << 3, 0x73, 1);
				
				sprite_data[1] = (3 << 3) + SCREEN_START_X;
				sprite_data[0] = (17U * 7U) + SCREEN_START_Y - 1U;
				sprite_data[2] = 10; // put it into the sprite memory.
				sprite_data[3] = 1 | (1 << 4);
				memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
				next_oam_idx += sizeof(sprite_data) >> 2;

				sprite_data[1] = (3 << 3) + SCREEN_START_X;
				sprite_data[0] = (18U * 7U) + SCREEN_START_Y - 1U;
				sprite_data[2] = 13; // put it into the sprite memory.
				sprite_data[3] = 1 | (1 << 4);
				memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
				next_oam_idx += sizeof(sprite_data) >> 2;
			}
			else if (local_t > BLINK_LEN - 30)
			{
				// oam_spr(3 << 3, 25 << 3, 0x62, 1);
				// oam_spr(3 << 3, 26 << 3, 0x72, 1);
				
				sprite_data[1] = (3 << 3) + SCREEN_START_X;
				sprite_data[0] = (17U * 7U) + SCREEN_START_Y - 1U;
				sprite_data[2] = 9; // put it into the sprite memory.
				sprite_data[3] = 1 | (1 << 4);
				memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
				next_oam_idx += sizeof(sprite_data) >> 2;

				sprite_data[1] = (3 << 3) + SCREEN_START_X;
				sprite_data[0] = (18U * 7U) + SCREEN_START_Y - 1U;
				sprite_data[2] = 12; // put it into the sprite memory.
				sprite_data[3] = 1 | (1 << 4);
				memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
				next_oam_idx += sizeof(sprite_data) >> 2;
			}
		}
	}

	// // FLAGS
	// local_t = tick_count & 63;
	// if (local_t > 48)
	// {
	// 	local_ix = 0x69;
	// }
	// else if (local_t > 32)
	// {
	// 	local_ix = 0x68;
	// }
	// else if (local_t > 16)
	// {
	// 	local_ix = 0x67;
	// }
	// else
	// {
	// 	local_ix = 0x66;
	// }

	// oam_spr(8 << 3, 1 << 3, local_ix, 2);
	// oam_spr(24 << 3, 1 << 3, local_ix, 2);
	// oam_spr(3 << 3, 10 << 3, local_ix, 0);
	// oam_spr(27 << 3, 10 << 3, local_ix, 0);

	//debug_draw_board_area();

	// Blocks spitting out prototype.
	//

	// for (i = 0; i < NUM_BLOCK_PARTICLES; ++i)
	// {
	// 	block_particle_x[i] += block_particle_vel_x[i];
	// 	block_particle_vel_y[i] += 30;
	// 	block_particle_y[i] += block_particle_vel_y[i];

	// 	if (block_particle_y[i] > (200 << 8))
	// 	{
	// 		block_particle_x[i] = 0;
	// 		block_particle_y[i] = (32 << 8);
	// 		block_particle_vel_x[i] = ((unsigned char)rand() % 64U) + 128U;
	// 		block_particle_vel_y[i] = -((unsigned char)rand() << 1);
	// 		block_particle_spr[i] = ((unsigned char)rand() % 7);
	// 	}

	// 	sprite_data[1] = (block_particle_x[i] >> 8) + SCREEN_START_X; //x
	// 	sprite_data[0] = (block_particle_y[i] >> 8) + SCREEN_START_Y; //y
	// 	sprite_data[2] = block_particle_spr[i]; //sprite
	// 	sprite_data[3] = 0; //attr
	// 	memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
	// 	next_oam_idx += sizeof(sprite_data) >> 2;
	// }
}

void draw_pause_sprites(void)
{
	// "A" == 211/D3
	static const UINT8 pause_test[] = "PAUSE";
	static UINT8 i;
	static UINT8 speed;
	static UINT8 shake_offset;

	// NOTE: Loop accounts for null term.
	for(i = 0; i < 5; ++i)
	{
		speed = tick_count >> 4;
		shake_offset = tenatcle_offsets[((i + speed) & 3)]; // &3 = %4 = number of entries in array.

		sprite_data[1] = ((11 + i) << 3) + SCREEN_START_X + 5;
		sprite_data[0] = (UINT8)(9 * 7) + (UINT8)SCREEN_START_Y - 1U + shake_offset;
		sprite_data[2] = 17 + i; // put it into the sprite memory.
		sprite_data[3] = 1 | (1 << 4);
		memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
		next_oam_idx += sizeof(sprite_data) >> 2;

		// For now only draw the drop shadow on CGB, as DMG palettes will be the
		// same for the text color.
		// TODO: Switch DMG OBJ pals when entering pause.
		// NOTE: Draw order is front to back.
		// if (_cpu == CGB_TYPE) 
		// {
		// 	sprite_data[0] += 1;
		// 	// currently grey scale default.
		// 	sprite_data[3] = 2;
		// 	memcpy(oam + (next_oam_idx << 2), sprite_data, sizeof(sprite_data));
		// 	next_oam_idx += sizeof(sprite_data) >> 2;
		// }
	}
}

void movement(void)
{
	static unsigned char hard_drop_performed;

	hit = 0;
	temp_fall_frame_counter = 0;
	old_x = 0;
	delay_lock_skip = 0;

	if (start_delay_remaining == 0)
	{
		--fall_frame_counter;
	}
	else
	{
		--start_delay_remaining;
	}

#if VS_SYS_ENABLED
	if (pad_all_new & (PAD_SELECT | PAD_START))
#else
	if (pad_all_new & PAD_SELECT)
#endif
	{
		// cur_score += 100;
		// display_score();
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
		// lines_cleared_one = 9;
		// inc_lines_cleared();
		//add_block_at_bottom();
		//spawn_new_cluster();

		//add_garbage_row_at_bottom(4);

		// Don't allow forcing the tentacle up while it is on the way down.
		// Not too serious, but looks weird when the height in increases
		// while the sprites are not moving up.
		if (row_to_clear == -1)
		{
			attack_queued = 1;
		}
	}

	// INPUT

	if (pad_all_new & PAD_A)
	{
		rotate_cur_cluster(1);
	}
	else if (pad_all_new & PAD_B)
	{
		rotate_cur_cluster(-1);
	}

	if (horz_button_delay > 0)
	{
		--horz_button_delay;
	}

	old_x = cur_block.x;
	if (((pad_all & PAD_RIGHT) && horz_button_delay == 0) || (pad_all_new & PAD_RIGHT))
	{
		horz_button_delay = button_delay;
		if ((pad_all_new & PAD_RIGHT))
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
	else if (((pad_all & PAD_LEFT) && horz_button_delay == 0) || pad_all_new & PAD_LEFT)
	{
		horz_button_delay = button_delay;
		if ((pad_all_new & PAD_LEFT))
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

		// This pains me, but I am NOT implementing this fix for the NES version to avoid
		// complicating high scores ("which version did you get the score on!?").
		// Enabling the fix on GB since that feels like a different experience overall and
		// score will not be 1:1 with NES anyway.
#if VS_SYS_ENABLED || PLAT_GB
		// If we are charging into a wall, instantly clear the button delay.
		// this is because we WANT pieces to tuck into walls when charged against them.
		// Without this, it will only TRY to tuck when horz_button_delay reaches 0 with DAS,
		// Meaning it will check frame 0,10,15,20,25... which feels very wrong. It can skip
		// over large gaps when playing at high levels where the piece travels multiple spaces
		// in a 5 frames, let alone 10.
		// https://discord.com/channels/731554439055278221/731554439793606709/791440637814636634
		horz_button_delay = 0;
#endif //VS_SYS_ENABLED
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
	temp_fall_frame_counter = fall_frame_counter;

	hard_drop_performed = 0;
	if (savegame.hard_drops_on && pad_all & PAD_UP && (pad_all & (PAD_LEFT|PAD_RIGHT)) == 0)
	{
		if ((pad_all & PAD_UP && hard_drop_tap_required == 0) || pad_all_new & PAD_UP)
		{
			--hard_drop_hold_remaining;

			if (hard_drop_hold_remaining == 0)
			{
				hard_drop_performed = 1;
				hard_drop_tap_required = 1;

				// TODO: Causes hitch.
				while (!is_cluster_colliding())
				{
					++cur_block.y;
				}

				// No delay lock on hard drops.
				delay_lock_skip = 1;

				// UNCOMMENT FOR LAST CHANCE MOVE ON HARD DROP
				// cur_block.y -= 1;
				// fall_frame_counter = 1;
			}
		}
	}
	else
	{
		if ((pad_all & (PAD_LEFT|PAD_RIGHT)) == 0)
		{
			hard_drop_tap_required = 0;
		}
		if (savegame.hard_drops_on == 1) // tap
		{
			hard_drop_hold_remaining = 1;
		}
		else if (savegame.hard_drops_on == 2) // hold
		{
			hard_drop_hold_remaining = HARD_DROP_HOLD_TIME;
		}
	}


	if (hard_drop_performed == 0)
	{
		// Hard drop skips all this to avoid dropping to the bottom
		// and then dropping again because it happens to be
		// the natural fall frame.
		if (pad_all_new & PAD_DOWN || delay_lock_remaining != -1)
		{
			if (pad_all_new & PAD_DOWN)
			{
				// if a new press was made this frame, skip the delay lock.
				delay_lock_skip = 1;
			}
			require_new_down_button = 0;

			// fall this frame.
			fall_frame_counter = 0;
		}
		else if ((pad_all & PAD_DOWN) && require_new_down_button == 0)
		{
			// fall every other frame.
			fall_frame_counter = MIN(fall_frame_counter, 1);
		}

		if (fall_frame_counter == 0)
		{
			// If the player forces the opening move, skip any remaining start delay.
			// NOTE: We don't do this for Hard Drops, allowing a sort of "boosted start"
			//		 if the player wants to try a place a few pieces before they start falling.
			start_delay_remaining = 0;

			cur_block.y += 1;
			fall_frame_counter = fall_rate;
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
		put_cur_cluster();
		// Spawn a new block.
		//spawn_new_cluster();
		delay_spawn_remaining = DELAY_SPAWN_LEN;
	}
//PROFILE_POKE(0x1e); //none
}

void set_block(/*unsigned char x, unsigned char y, unsigned char id*/)
{
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

	UPDATE_TILE_BY_VALUE(in_x + (BOARD_START_X_PX >> 3), in_y- 4, in_id, 0x10);

	// TODO: Is this too slow?
	game_board[TILE_TO_BOARD_INDEX(in_x, in_y)] = in_id;
}

#if PLAT_NES

void set_block_nt(unsigned char x, unsigned char y, unsigned char id, unsigned char nt)
{
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

#endif // PLAT_NES

void put_cur_cluster()
{
	static unsigned char i;
	static unsigned char j;

//PROFILE_POKE(0x5f); //green

	max_y = 0;
	min_y = 0xff; // max

	for (i = 0; i < 4; ++i)
	{
		// store the index into the x,y offset for each solid piece in the first rotation.
		j = cur_cluster.layout[i];

		// convert that to x,y offsets.
		local_ix = index_to_x_lookup[j];
		local_iy = index_to_y_lookup[j];

		in_x = cur_block.x + local_ix;
		in_y = cur_block.y + local_iy;
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

//PROFILE_POKE(0x9f); //blue
	SFX_PLAY_WRAPPER(SOUND_LAND);

	// Don't end the game when something goes off screen (it should only end when a new piece can't be placed).
	//
	// if (min_y <= BOARD_OOB_END)
	// {
	// 	go_to_state(STATE_OVER);
	// 	return;
	// }
	// else
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
		if (savegame.attack_style == ATTACK_ON_LAND)
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


	if (garbage_row_queue)
	{
		add_garbage_row_at_bottom(garbage_row_queue);
		garbage_row_queue = 0;
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
	return game_board[TILE_TO_BOARD_INDEX(x,y)] == EMPTY_TILE;
}

unsigned char is_cluster_colliding()
{
	static unsigned char x;
	static unsigned char y;
	static unsigned char i;
	static unsigned char j;

	for (i = 0; i < 4; ++i)
	{
		// store the index into the x,y offset for each solid piece in the first rotation.
		j = cur_cluster.layout[i];

		// convert that to x,y offsets.
		local_ix = index_to_x_lookup[j];
		local_iy = index_to_y_lookup[j];

		x = cur_block.x + local_ix;
		y = cur_block.y + local_iy;

		if (y > BOARD_END_Y_PX_BOARD || x > BOARD_END_X_PX_BOARD)
		{
			// consider this blocked.
			return 1;
		}

		//return get_block(x, y) == 0;
		if(game_board[TILE_TO_BOARD_INDEX(x,y)] != EMPTY_TILE) // != 5 && game_board[TILE_TO_BOARD_INDEX(x,y)] != 0)
		{
			return 1;
		}
	}

	return 0;
}

void spawn_new_cluster()
{
	static unsigned char i;
	static unsigned char j;

	id = 0;

	delay_lock_remaining = -1;

	require_new_down_button = 1;
	fall_frame_counter = fall_rate;

	cur_rot = 0;

	// Copy the next cluster to the current one.
	memcpy(cur_cluster.def, next_cluster.def, 4 * 4);
	memcpy(cur_cluster.layout, cur_cluster.def[0], 4);
	cur_cluster.sprite = next_cluster.sprite;
	cur_cluster.id = next_cluster.id;

	// Reset the block.
	cur_block.x = 3; //(BOARD_END_Y_PX_BOARD >> 1);
	cur_block.y = cluster_offsets[cur_cluster.id];

	// By checking twice we go from 1 in 7 chance of a dupe to
	// 1 in 49 chance.
	id = (unsigned char)rand() % NUM_CLUSTERS;
	if (id == cur_cluster.id)
	{
		id = (unsigned char)rand() % NUM_CLUSTERS;
	}

//	id = 2;

	if (id >= NUM_CLUSTERS)
	{
		Printf("ERROR");
	}
	next_cluster.id = id;
	memcpy(next_cluster.def, cluster_defs_classic[id], (4 * 4));
	memcpy(next_cluster.layout, next_cluster.def[0], 4);
	next_cluster.sprite = cluster_sprites[id];

//PROFILE_POKE(0x9f); //blue
	// Put the next block into the nametable.
	local_iy = 0;
	local_ix = 0;
	local_t = next_cluster.sprite;

	// clear out the middle 2 rows of the "next piece" (all pieces spawn with only those 2 rows containing visuals).
	multi_vram_buffer_horz(empty_row, 4, get_ppu_addr(cur_nt, 120, 16));
	multi_vram_buffer_horz(empty_row, 4, get_ppu_addr(cur_nt, 120, 24));

	for (i = 0; i < 4; ++i)
	{
		UPDATE_TILE_BY_VALUE(2 + i, 1, EMPTY_TILE, 0x10);
		UPDATE_TILE_BY_VALUE(2 + i, 2, EMPTY_TILE, 0x10);
	}

	for (i = 0; i < 4; ++i)
	{
		// store the index into the x,y offset for each solid piece in the first rotation.
		j = next_cluster.layout[i];

		// convert that to x,y offsets.
		local_ix = index_to_x_lookup[j];
		local_iy = index_to_y_lookup[j];

		one_vram_buffer(local_t, get_ppu_addr(cur_nt, 120 + (local_ix << 3), 8 + (local_iy << 3)));

		UPDATE_TILE_BY_VALUE(2 + local_ix, 0 + local_iy, local_t, 0x10);
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
		queued_packet |= MP_GAME_OTHER_LOST;
		// This state change MUST get through. Everything else will self correct or
		// has error handling in other ways, but this change to game over should try
		// over and over until it gets through.
		queued_packet_required = 1;

		// Force to row 20 since it will not have time to detect it
		// with the normal flow.
		queued_packet |= (1 << MP_GAME_HIGHWATER_SHIFT);

		++rounds_lost;
		send_queued_packet();
		go_to_state(STATE_OVER);

		if (is_sio_game)
		{
			PRINT(10, 4, "YOU LOSE");
		}
	}
}

void rotate_cur_cluster(char dir)
{
	static unsigned char old_rot;

	old_rot = cur_rot;

	cur_rot = (cur_rot + dir) & 3; // % 4

	memcpy(cur_cluster.layout, cur_cluster.def[cur_rot], 4);

	// if after rotating, we are now colliding we something, revert the rotation.
	if (is_cluster_colliding())
	{
		// RIGHT 1
		++cur_block.x;
		if (is_cluster_colliding())
		{
			// LEFT 1
			cur_block.x -= 2;
			if (is_cluster_colliding())
			{
				// Special case for line piece :(
				if (cur_cluster.id == 2)
				{
					// RIGHT 2
					cur_block.x += 3;
					if (is_cluster_colliding())
					{
						// Officially no where to go. Revert back to original position
						// and rotation, and play a whaa-whaa sound.
						cur_block.x -= 2;
						cur_rot = old_rot;
						memcpy(cur_cluster.layout, cur_cluster.def[cur_rot], 4);
						SFX_PLAY_WRAPPER(SOUND_BLOCKED);
						return;
					}
				}
				else
				{
					// Officially no where to go. Revert back to original position
					// and rotation, and play a whaa-whaa sound.
					cur_block.x += 1;
					cur_rot = old_rot;
					memcpy(cur_cluster.layout, cur_cluster.def[cur_rot], 4);
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
	static unsigned char i;
	static unsigned char fade_delay;
	static unsigned char prev_state;
	static unsigned char empty_menu_tile;
#if VS_SYS_ENABLED
	static unsigned char j;
	static unsigned char k;
	static unsigned long temp_score;
	static unsigned char digit;
#endif // #if VS_SYS_ENABLED
	fade_delay = 5;
	prev_state = state;

	sub_state = 0;
	cur_option = 0;

	// Handle the case where the packet was queued prior to changing states, and then
	// may end up still getting sent at the end of this frame, but for a new
	// state!
	queued_packet = queued_packet_required = 0;

	// Clear OAM prior to drawing fresh.
	ClearOAMs();

	// Force the OAM to update with the sprites
	// now hidden.
	SwapOAMs();		

	switch (state)
	{
		//case STATE_BOOT:
		case STATE_TY:
		case STATE_SOUND_TEST:
		{
			MUSIC_PLAY_ATTRACT_WRAPPER(MUSIC_TITLE);
			break;
		}
		case STATE_OPTIONS:
		{
#if VS_SYS_ENABLED
			// swap back to normal CHR setup.
			bank_bg(0);
			bank_spr(1);
#endif
			pal_bg(palette_bg);
			savegame.saved_starting_level = cur_level;
			fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];
			row_to_clear = -1;
			start_delay_remaining = START_DELAY;
			rounds_won = 0;
			rounds_lost = 0;
			matches_won = 0;
			//display_level();
			//display_score();
			break;
		}

#if !VS_SYS_ENABLED
		case STATE_PAUSE:
		{
			pal_bright(4);
			break;
		}
#endif // #if !VS_SYS_ENABLED

		case STATE_GAME:
		{
			// Little bit of future proofing in case we add other ways
			// to exit the game (eg. from pause).
#if VS_SYS_ENABLED
			high_score_entry_placement = 0xff;
			if (attract_gameplay_enabled)
			{
				auto_forward_leaderboards = 3; // cycle through all boards.
			}
			else
			{
				auto_forward_leaderboards = 1; // auto forward after 10 seconds.
			}
			if (cur_score > 0 && IS_PRIMARY_CPU && !attract_gameplay_enabled)
			{
#if VS_SRAM_ENABLED				
				// take control of SRAM.
				POKE(0x4016, 2);
#endif // #if VS_SRAM_ENABLED
				for (i = 0; i < 3; ++i)
				{
					if (high_scores_vs_value[savegame.attack_style][cur_level_vs_setting][i] == NO_SCORE || cur_score > high_scores_vs_value[savegame.attack_style][cur_level_vs_setting][i])
					{
						high_score_entry_placement = i;
						for(j = 2; j != i; --j)
						{
							if (high_scores_vs_value[savegame.attack_style][cur_level_vs_setting][j-1] != NO_SCORE)
							{
								high_scores_vs_value[savegame.attack_style][cur_level_vs_setting][j] = high_scores_vs_value[savegame.attack_style][cur_level_vs_setting][j-1];
								memcpy(high_scores_vs_initials[savegame.attack_style][cur_level_vs_setting][j], high_scores_vs_initials[savegame.attack_style][cur_level_vs_setting][j-1], 3);
							}
						}
						high_scores_vs_value[savegame.attack_style][cur_level_vs_setting][i] = cur_score;
						memcpy(high_scores_vs_initials[savegame.attack_style][cur_level_vs_setting][i], "---", 3);
						cur_score = NO_SCORE;
						break;
					}
				}
				
				// Reliquish control of SRAM.
				POKE(0x4016, 0);

				attract_gameplay_enabled = 0;
			}
#else
			if (cur_score > savegame.high_scores[savegame.attack_style])
			{
				savegame.high_scores[savegame.attack_style] = cur_score;
			}
#endif
			break;
		}

#if VS_SYS_ENABLED
		case STATE_HIGH_SCORE_TABLE:
		{
			high_score_entry_placement = 0xff;
			break;
		}
#endif // #if VS_SYS_ENABLED
	default:
		break;
	}

	state = new_state;

	ticks_in_state_large = 0;

	switch (state)
	{
		case STATE_BOOT:
		{
			pal_bg(palette_bg_options);
			ppu_off();
#if PLAT_GB
			InitScroll(BANK(boot_screen), &boot_screen, 0, 0);
			//scroll(scroll_x_camera,0);
#else
			vram_adr(NTADR_A(0,0));
			vram_unrle(boot_screen);
#endif // PLAT_GB
			ppu_on_all();

			break;
		}
		case STATE_TY:
		{
			pal_bg(palette_bg_options);
			ppu_off();
#if PLAT_GB
			InitScroll(BANK(ty_screen), &ty_screen, 0, 0);
#else
			vram_adr(NTADR_A(0,0));
			vram_unrle(ty_screen);
#endif // PLAT_GB			
			ppu_on_all();

			break;
		}
		case STATE_MENU:
		{
			// Seems like a good point to say we are "disconnected". Require a new handshake
			// to enter an SIO match again.
			is_sio_game = 0;

			// Start looking for incoming packets the moment we enter the title
			// screen.
			receive_byte();

			pal_bg(palette_bg);
			pal_spr(palette_sp);

#if PLAT_GB
			scroll_y_game = 0;
#else
			scroll_y_game = 0x1df;
#endif // PLAT_GB

			time_of_day = 0;
			cur_konami_index = 0;
#if VS_SYS_ENABLED
			attract_gameplay_enabled = 0;
#endif // #if VS_SYS_ENABLED			

//			if (prev_state == STATE_OPTIONS || prev_state == STATE_BOOT || prev_state == STATE_TY|| prev_state == STATE_SOUND_TEST || prev_state == STATE_HIGH_SCORE_TABLE)
			{
				oam_clear();
				draw_menu_sprites();

				ppu_off();
#if PLAT_GB
				//vram_unrle(title_and_game_area);

				// Using PNG for backgrounds causes the DMG version to look bad
				// because the palettes get mapped poorly. To avoid this, we just
				// have a unique background for the DMG consoles.
				unsigned char cloud_tile = 0x13;
				if (_cpu == CGB_TYPE) 		
				{		
					InitScroll(BANK(title_screen), &title_screen, 0, 0);
					cloud_tile = 0x14;
				}
				else	
				{		
					InitScroll(BANK(title_screen_dmg), &title_screen_dmg, 0, 0);
				}

				for (i = 0; i < 5; ++i)
				{
					UPDATE_TILE_BY_VALUE(7+i, 6, cloud_tile, 0x10);
					UPDATE_TILE_BY_VALUE(7+i, 7, cloud_tile, 0x10);
				}

 				MUSIC_PLAY_ATTRACT_WRAPPER(MUSIC_TITLE);				

				sgb_init_menu();

				// Needs to be called to reset the cur_level back to the saved_level.
				// On the NES, this was done below in the else...gameover case, but that is
				// all commented out now.
				//reset_gameplay_area();
				cur_level = savegame.saved_starting_level;

				// Disabled for now. If this is needed in the end remember
				// to push the current_bank first.
				// // Titlescreen is centered slightly offset.
				// scroll(4,0);
				// // force the scroll to update before fading in.
				// wait_vbl_done();
				// SpriteManagerUpdate();

				// scroll_x_camera = 56;
				// for (local_ix = 8; local_ix <= scroll_x_camera; local_ix+=8)
				// {
				// 	scroll(local_ix, 0);
				// }				
				//UPDATE_TILE(8,8,&test_bg_tile,0);
				//PRINT(0,16,"Hello World");
#else
				vram_adr(NTADR_A(0,0));
				vram_unrle(title_screen);
#endif // PLAT_GB				
				ppu_on_all();
#if VS_SYS_ENABLED
				multi_vram_buffer_horz(clear_push_start, sizeof(clear_push_start)-1, get_ppu_addr(0, 8<<3, 12<<3));
#else
				multi_vram_buffer_horz(clear_push_start, sizeof(clear_push_start)-1, get_ppu_addr(0, 12<<3, 12<<3));
#endif
			}
// 			else
// 			{
// 				if (prev_state == STATE_OVER)
// 				{
// 					fade_to_black();
// 				}

// 				//reset_gameplay_area();

// 				draw_menu_sprites();

// #if PLAT_GB
// 				scroll_y_game = 0;
// #else
// 				scroll_y_game = 0x1df;
// #endif // PLAT_GB
// 				//scroll(scroll_x_camera, scroll_y_game); // shift the bg down 1 pixel
// 				MUSIC_PLAY_ATTRACT_WRAPPER(MUSIC_TITLE);

// #if VS_SYS_ENABLED
// 				multi_vram_buffer_horz(clear_push_start, sizeof(clear_push_start)-1, get_ppu_addr(0, 8<<3, 12<<3));
// #else
// 				multi_vram_buffer_horz(clear_push_start, sizeof(clear_push_start)-1, get_ppu_addr(0, 12<<3, 12<<3));
// #endif

// 				if (prev_state == STATE_OVER)
// 				{
// 					fade_from_black();
// 				}
// 			}

			break;
		}

		case STATE_OPTIONS:
		{
			oam_clear();

			// TODO: Needed?
			receive_byte();			

			// get rid of any queued up changes as they are no longer valid.
			// fixes bug where "press a button" is on screen in settings because it got queued up
			// on the frame that we transitioned.
			clear_vram_buffer();

			ppu_off();

#if VS_SYS_ENABLED

			cur_level_vs_setting = MIN(cur_level_vs_setting, 2);
			vs_code_index = 0;

			// For VS, we use the 2nd half of the CHR for Background, and the first half
			// for sprites.
			// NOTE: The sprite switch isn't actually needed I think, since the only sprites
			// 		 drawn are numbers which exist in both.
			bank_bg(1);
			bank_spr(0);
			pal_bg(palette_vs_options);

			// Update the palettes to show which option is currently selected.
			for (i = 0; i < 4; ++i)
			{
				pal_col(i + (4 * savegame.attack_style), palette_vs_options_active[i]);
			}
#else
			pal_bg(palette_bg_options);
#endif

			//go_to_state(STATE_SOUND_TEST);
			vram_adr(NTADR_A(0,0));
#if VS_SYS_ENABLED
			vram_unrle(vs_settings_mode);
			ppu_on_all();
			option_state = 0;
#else
#if PLAT_GB

			// Using PNG for backgrounds causes the DMG version to look bad
			// because the palettes get mapped poorly. To avoid this, we just
			// have a unique background for the DMG consoles.
			if (_cpu == CGB_TYPE) 		
			{		
				InitScroll(BANK(options_screen), &options_screen, 0, 0);
			}
			else	
			{		
				InitScroll(BANK(options_screen_dmg), &options_screen_dmg, 0, 0);
			}

			INIT_FONT(font_options_bright, PRINT_BKG);

			PRINT(5, 7,  "LEVEL");
			PRINT(5, 8,  "MUSIC");
			PRINT(5, 9,  "SOUNDS");
			PRINT(5, 10, "H.DROP");
			PRINT(6, 12, "HI-SCORE");			

			sgb_init_settings();

			// Clear out the temp tiles used to force tile index.
			// UPDATE_TILE_BY_VALUE(0,0,4,NULL);
			// UPDATE_TILE_BY_VALUE(1,0,4,NULL);
			// UPDATE_TILE_BY_VALUE(2,0,4,NULL);
			// UPDATE_TILE_BY_VALUE(3,0,4,NULL);

			// Disabled for now. Remember to add PUSH/POP bank
			//
			// // Titlescreen is centered slightly offset.
			// scroll(0,0);
			// // force the scroll to update before fading in.
			// wait_vbl_done();
			// SpriteManagerUpdate();
#else
			vram_unrle(options_screen);			
#endif // PLAT_GB			
			// vram_adr(NTADR_A(16,19));
			// vram_write(attack_style_strings[savegame.attack_style], ATTACK_STRING_LEN);

			// vram_adr(NTADR_A(16,21));
			// vram_write(off_on_string[savegame.music_on], OFF_ON_STRING_LEN);

			// handle case where player used cheat to jump 10 levels, and then quit back
			// to the main menu.
			cur_level %= 10;

			ppu_on_all();

			display_options();
			// too much for 1 frame.
			wait_vbl_done();
			clear_vram_buffer();
			display_highscore();

			if (!is_host)
			{
				PRINT(5, 1, "WAITING...");
			}
			else
			{
				PRINT(6, 1, "SETTINGS");
			}
#endif

			break;
		}

#if !VS_SYS_ENABLED		
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
#endif //#if !VS_SYS_ENABLED		

		case STATE_GAME:
		{
			// This gets done in the main menu too.
			// GB: Turned off for GB port, as we always want to 
			// reset the game score, lines, etc when entering gameplay.
			// if (prev_state == STATE_OVER)
			// {
			// 	reset_gameplay_area();
			// }

			if (prev_state != STATE_PAUSE)
			{
				oam_clear();

				// sync up a 16bit srand value...
				if (is_sio_game)
				{
					// TODO: Delay may not actually be needed now that fade was moved to after this.
					#define SAFE_DELAY (60) // 30, 2x60 works

					unsigned char x_offset = 0;
					if (prev_state == STATE_OPTIONS)
					{
						x_offset = 4;
					}

					// 10,6 -> 17,10
					for (local_ix = 10; local_ix <= 17; ++local_ix)
					{
						for (local_iy = 6; local_iy <= 10; ++local_iy)
						{
							UPDATE_TILE_BY_VALUE(local_ix - x_offset, local_iy, 0x0, 0x10);
						}
					}

					PRINT(11-x_offset,8, "READY?");
					SFX_PLAY_WRAPPER(SOUND_MENU_LOW);

					draw_pause_sprites();

					if (is_host)
					{
						// Hack to delay the send to give the Super Game Boy version a chance to start
						// receiving before we send. I thought looping of == IO_ERROR would be good enough
						// but it doesn't work.
						vbl_delay(SAFE_DELAY);
						//do
						//{
							// First send the upper byte.
							_io_out = (tick_count_large >> 8);
							send_byte();
							while(_io_status == IO_SENDING);
						//} while (_io_status == IO_ERROR);

						PRINT(11-x_offset,8, " SET? ");
						SFX_PLAY_WRAPPER(SOUND_MENU_LOW);

						// Now wait for client to get the upper byte and send
						// back and ACK.
						receive_byte();
						while(_io_status == IO_RECEIVING);

						PRINT(11-x_offset,8, "  GO! ");
						SFX_PLAY_WRAPPER(SOUND_START);

						// Client has sent ACK. Value doesn't matter, as we just
						// assume success at this point.

						// Hack
						vbl_delay(SAFE_DELAY);

						//do
						//{
							// Send lower byte.
							_io_out = (tick_count_large & 0xff);
							send_byte();
							while(_io_status == IO_SENDING);
						//} while (_io_status != IO_IDLE);

						//PRINT(13,8, "GO!");
						//SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);
						//SFX_PLAY_WRAPPER(SOUND_START);

						// Seed the RNG with this synced value.
						srand(tick_count_large);

						seed_value = tick_count_large;
					}
					else
					{
						seed_value = 0;

						// First thing that should happen is that the host will send
						// the upper byte of the seed.
						if (_io_status != IO_RECEIVING)
						{
							receive_byte();
						}
						while(_io_status == IO_RECEIVING);
						seed_value = (_io_in << 8);

						PRINT(11-x_offset,8, " SET? ");
						SFX_PLAY_WRAPPER(SOUND_MENU_LOW);

						vbl_delay(SAFE_DELAY);
						//do
						//{
							// Now that we have the first byte, let the host know we are
							// ready for the second byte.
							_io_out = 0x69;
							send_byte();
							while(_io_status == IO_SENDING);
						//} while (_io_status  != IO_IDLE);

						PRINT(11-x_offset,8, "  GO! ");
						SFX_PLAY_WRAPPER(SOUND_START);

						// The second byte should be arriving next.
						receive_byte();
						while(_io_status == IO_RECEIVING);
						seed_value |= _io_in;

						//PRINT(13,8, "GO!");
						//SFX_PLAY_WRAPPER(SOUND_MENU_HIGH);
						//SFX_PLAY_WRAPPER(SOUND_START);

						// Seed the RNG with this synced value.
						srand(seed_value);
					}

					receive_byte();
				}
				else
				{
					srand(tick_count_large);
				}

				// Fade after the SIO stuff to avoid issues.
				fade_to_black();

#if VS_SYS_ENABLED
				if (!attract_gameplay_enabled && credits_remaining >= game_cost)
				{
					credits_remaining-=game_cost;
				}
#endif// VS_SYS_ENABLED

#if !VS_SYS_ENABLED

				// During initial scroll in, the scroll_y_game is 0x1df
				// which will cause some very odd behavior when trying to scroll
				// to 240. Force it to jump to 0 (1 pixel down), and scroll from there.
				if (prev_state == STATE_OPTIONS)
				{
					// start at the top.
					scroll_y_game = 0;
				}
				// while (scroll_y_game < 306)
				// {
				// 	scroll(scroll_x_camera, scroll_y_game);
				// 	wait_vbl_done();
				// 	SpriteManagerUpdate(); 
				// 	scroll_y_game += 4;
				// }
#endif //!VS_SYS_ENABLED
				// scroll_y_game = 0;
				// scroll(0, scroll_y_game);

#if DEBUG_ENABLED
				PRINT_POS(0,4);
				Printf("SEED:%d", seed_value);
#endif // DEBUG_ENABLED

				INIT_FONT(font_gameplay, PRINT_BKG);
				InitScroll(BANK(gameplay_map), &gameplay_map, 0, 0);
				// Clear out the temp tiles used to force tile index.
				// UPDATE_TILE_BY_VALUE(0,0,3,NULL);
				// UPDATE_TILE_BY_VALUE(1,0,3,NULL);
				// UPDATE_TILE_BY_VALUE(2,0,3,NULL);
				reset_gameplay_area();

#if DEBUG_ENABLED
				PRINT_POS(0,3);
				Printf("%d %d", (seed_value>>8), (seed_value&0xff));
#endif // DEBUG_ENABLED

				//UPDATE_TILE(0,0,&test_bg_tile,0);

				// Spawn "next"
				spawn_new_cluster();
				// "Next" becomes current, and a new next is defined.
				spawn_new_cluster();

				require_new_down_button = 1;
				if (savegame.attack_style == ATTACK_ON_TIME)
				{
					attack_queue_ticks_remaining = attack_delay;
				}

				if (is_sio_game)
				{
					// Create a column for the hugh water.
					for (local_iy = 0; local_iy < BOARD_HEIGHT; ++local_iy)
					{
						UPDATE_TILE_BY_VALUE(8, 19 - local_iy, 136, 0x10);
					}

					// Pointer to the start fo the game data.
					cur_high_water_row = &game_board[(BOARD_OOB_END + 1) * BOARD_WIDTH];
				}

				sgb_init_gameplay();

				fade_from_black();
			}

#if VS_SYS_ENABLED
			if (!attract_gameplay_enabled)
#endif // #if VS_SYS_ENABLED			
			{
				// Do this at the end of the state change so that
				// the up beat music doesn't kick in until after
				// everything transitions in.
				cur_gameplay_music = GAMEPLAY_MUSIC_NORMAL;
				MUSIC_PLAY_WRAPPER(MUSIC_GAMEPLAY);
			}

			break;
		}
#if !VS_SYS_ENABLED		
		case STATE_PAUSE:
		{
			pal_bright(2);
			MUSIC_PLAY_WRAPPER(MUSIC_PAUSE);
			break;
		}
#endif //#if !VS_SYS_ENABLED				
		case STATE_OVER:
		{
			// fix bug where mashing up/down to quickly hit gamover would case nametable coruption.
			wait_vbl_done();
			clear_vram_buffer();

			// Without this, the "next" block won't appear for the first half of the sequence.
			//draw_gameplay_sprites();

			StopMusic;
			MUSIC_PLAY_WRAPPER(MUSIC_GAMEOVER_INTRO);


			// Without music this delay feels really odd.
			// However, we need to keep the SIO game in sync, so always wait in that case.
#if !VS_SYS_ENABLED
			if (savegame.music_on || is_sio_game)
			{
				vbl_delay(120);
			}
#endif // !VS_SYS_ENABLED

			// treat this like music, since it is a jingle.
			MUSIC_PLAY_WRAPPER(MUSIC_GAMEOVER_OUTRO);

			// Not sure why this was here. It causes the next block to
			// vanish.
			//oam_clear();

			// pal_bright(5);
			// delay(fade_delay);
			pal_bright(6);
			vbl_delay(fade_delay);
			// pal_bright(7);
			// delay(fade_delay);
			pal_bright(8);
			vbl_delay(fade_delay);

			fade_to_black();

#if !VS_SYS_ENABLED
			// address = get_ppu_addr(cur_nt, 96, 14<<3);
			// multi_vram_buffer_horz("\x9a\x9b\xba\xbb\x00\x9b\x96\xbb\x9d\xf7", 10, address);
			// //address = get_ppu_addr(cur_nt, 96, 15<<3);
			// address += 32;
			// multi_vram_buffer_horz("\xaa\xab\xca\xcb\x00\x3a\x68\xcb\xdf\xf9", 10, address);

			// // address = get_ppu_addr(cur_nt, 96, 14<<3);
			// // multi_vram_buffer_horz("GAME OVER!", 10, address);
			// //address = get_ppu_addr(cur_nt, 96, 15<<3);
			// address += 32;
			// multi_vram_buffer_horz("A-RESTART ", 10, address);
			// //address = get_ppu_addr(cur_nt, 96, 16<<3);
			// address += 32;
			// multi_vram_buffer_horz("B-QUIT    ", 10, address);

			//const UINT8 over_top[] = { 113, 114, 115, 116, 0, 114, 117, 116, 118, 126 };
			//const UINT8 over_bot[] = { 119, 120, 121, 122, 0, 123, 124, 122, 125, 127 };


			for (i = 0; i < 10; ++i)
			{
				for (UINT8 j = 0; j < 14; ++j)
				{
					if (j == 0 && i == 0)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 181, 0x10);
					}
					else if (j == 0 && i == 9)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 183, 0x10);
					}
					else if (j == 13 && i ==0)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 186, 0x10);
					}
					else if (j == 13 && i == 9)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 188, 0x10);
					}
					else if (j == 13)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 187, 0x10);
					}
					else if (j == 0)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 182, 0x10);
					}
					else if (i == 0)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 184, 0x10);
					}
					else if (i == 9)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 185, 0x10);
					}
					else
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 0x0, 0x10);
					}
				}
			}


			if (is_sio_game)
			{
				// clear the area used for the SIO wins/loses.
				for (i = 0; i < 10; ++i)
				{
					for (UINT8 j = 14; j < 17; ++j)
					{
						UPDATE_TILE_BY_VALUE(9 + i, 3 + j, 0x0, 0x10);
					}
				}
			}			

			// for(i = 0; i < 9; ++i)
			// {
			// 	UPDATE_TILE_BY_VALUE(9, 6 + i, 184, 0x10);
			// 	UPDATE_TILE_BY_VALUE(18, 6 + i, 185, 0x10);
			// }

			UINT8 tile = 137;
			const UINT8 x_offset = 10;
			const UINT8 y_offset = 6;

			//for (UINT8 j = 0; j < 5; ++j)
			{
				for (i = 0; i < 8; ++i)
				{
					UPDATE_TILE_BY_VALUE(x_offset + i, y_offset, tile, 0x10);
					UPDATE_TILE_BY_VALUE(x_offset + i, y_offset + 1, tile + 8, 0x10);
					UPDATE_TILE_BY_VALUE(x_offset + i, y_offset + 2, tile + 16, 0x10);
					UPDATE_TILE_BY_VALUE(x_offset + i, y_offset + 3, tile + 24, 0x10);
					UPDATE_TILE_BY_VALUE(x_offset + i, y_offset + 4, tile + 32, 0x10);
					++tile;
				}
			}

			UPDATE_TILE_BY_VALUE(10, 12, 177, 0x10);
			UPDATE_TILE_BY_VALUE(10, 14, 178, 0x10);
			PRINT(11, 12, "RESTART");
			PRINT(11, 14, "QUIT   ");

			if (is_sio_game)
			{

				for (i = 0; i < 3; ++i)
				{
					// Wins
					UPDATE_TILE_BY_VALUE(10 + i, 18, (rounds_won > i) ? 189 : 190, 0x10);
					// Loses
					UPDATE_TILE_BY_VALUE(15 + i, 18, (rounds_lost > i) ? 189 : 190, 0x10);
				}

				PRINT(13, 18, "VS");

				if(rounds_lost >= 3 || rounds_won >= 3)
				{
					if (rounds_won >= 3)
					{
						++matches_won;
					}
					rounds_lost = rounds_won = 0;
				}


				// Edges of win count text.
				// TODO: Could be flipped instead of duplicated.
				UPDATE_TILE_BY_VALUE(11,17, 0xbf, 0x10);
				UPDATE_TILE_BY_VALUE(16,17, 0xc0, 0x10);

				// Always use 2 digits. Doesn't handle >99 well.
				if (matches_won < 10)
				{
					PRINT_POS(12, 17);
					Printf("-0%d-", matches_won);
				}
				else
				{
					PRINT_POS(12, 17);
					Printf("-%d-", matches_won);
				}
			}
#else
			address = get_ppu_addr(cur_nt, 96, 14<<3);
			multi_vram_buffer_horz("\x9a\x9b\xba\xbb\x00\x9b\x96\xbb\x9d\xf7", 10, address);
			//address = get_ppu_addr(cur_nt, 96, 15<<3);
			address += 32;
			multi_vram_buffer_horz("\xaa\xab\xca\xcb\x00\x3a\x68\xcb\xdf\xf9", 10, address);
#endif //!VS_SYS_ENABLED
			// pal_bright(7);
			// delay(fade_delay);
			pal_bright(6);
			vbl_delay(fade_delay);
			// pal_bright(5);
			// delay(fade_delay);
			pal_bright(4);
			//delay(fade_delay);

			fade_from_black();
			break;
		}

#if VS_SYS_ENABLED
		case STATE_HIGH_SCORE_TABLE:
		{
			if (prev_state == STATE_OVER)
			{
				MUSIC_PLAY_ATTRACT_WRAPPER(MUSIC_TITLE);
				//before ppuoff
				reset_gameplay_area();
			}

			oam_clear();
			ppu_off(); // screen off

			cur_initial_index = 0;

#if PLAT_GB
			scroll_y_game = 0;
#else
			scroll_y_game = 0x1df;
#endif // PLAT_GB
			// force it so we see it when the fade in completes.
			scroll(0, scroll_y_game);

			pal_bg(palette_vs_highscore_table);
			pal_spr(palette_vs_highscore_table);
			vram_adr(NTADR_A(0,0));
			vram_unrle(high_score_screen);

			if (savegame.attack_style == ATTACK_NEVER)
			{
				vram_adr(NTADR_A(15 - ((sizeof(attack_style_strings[savegame.attack_style]))/2),3));
			}
			else
			{
				vram_adr(NTADR_A(16 - ((sizeof(attack_style_strings[savegame.attack_style]))/2),3));
			}

			vram_write(attack_style_strings[savegame.attack_style], sizeof(attack_style_strings[savegame.attack_style]));

			// difficulty
			for (i = 0; i < 4; ++i)
			{
				// output to in_x, in_y
				difficulty_to_leaderboard_pos(i);

				// Scores from 1st place to last place.
				for (j = 0; j < 3; ++j)
				{
					vram_adr(NTADR_A(in_x, in_y+j));

					if (IS_PRIMARY_CPU)
					{
#if VS_SRAM_ENABLED						
						// take control of SRAM.
						POKE(0x4016, 2);
#endif // #if VS_SRAM_ENABLED			
						vram_write(high_scores_vs_initials[savegame.attack_style][i][j], 3);

						// re-use cur_score. Means this can't be done during gameplay.
						//cur_score = high_scores_vs_value[savegame.attack_style][i][j];

						// vram_adr(NTADR_A(in_x + 4,in_y+j));
						// vram_put('0' + cur_score);

						temp_score = high_scores_vs_value[savegame.attack_style][i][j];
#if VS_SRAM_ENABLED						
						// Reliquish control of SRAM.
						POKE(0x4016, 0);
#endif //#if VS_SRAM_ENABLED
					}

					if (temp_score == NO_SCORE)
					{
						vram_adr(NTADR_A(in_x + 4,in_y+j));
						vram_write("-------", 7);
					}
					else
					{
						// clear out any old score.
						// multi_vram_buffer_horz("      ", 6, get_ppu_addr(cur_nt, 0, 6<<3));
						vram_adr(NTADR_A(in_x + 4,in_y+j));
						vram_write("0000000", 7);

						k = 0;
						while(temp_score != 0)
						{
							digit = temp_score % 10;
							//one_vram_buffer('0' + digit, get_ppu_addr(0, (23<<3) - (i << 3), 27<<3 ));
							vram_adr(NTADR_A((in_x + 4) + 6 - k, in_y + j));
							vram_put('0' + digit);

							temp_score = temp_score / 10;
							++k;
						}
					}
				}
			}

			ppu_on_all(); // turn on screen

			break;
		}
#endif // #if VS_SYS_ENABLED

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
			++cur_level;

			// we only handle things up to level 29.
			if (cur_level <= 29 )
			{
				fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];
			}
			else if (cur_level < 40) // raise the floor 10 levels.
			{
				if (cur_level == 30)
				{
					//one_vram_buffer(SKULL_SPRITE, get_ppu_addr(cur_nt, 4<<3, 9<<3)); // skull
					UPDATE_TILE_BY_VALUE(4, 10, SKULL_SPRITE, 0x10);
				}
				kill_row_queued = 1;
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
	//static unsigned char* text;

	//UPDATE_TILE_BY_VALUE(4,3,0,0);
	//UIntToString(lines_cleared_hundred, text);
	//PRINT(4,1,text);
	//PRINT(2,5, "LINES");
	PRINT_POS(4,6);
	Printf("%d", lines_cleared_hundred);
	PRINT_POS(5,6);
	Printf("%d", lines_cleared_ten);
	PRINT_POS(6,6);
	Printf("%d", lines_cleared_one);

	//UPDATE_TILE_BY_VALUE(4,3,'0' + lines_cleared_hundred,0);
	//UPDATE_TILE_BY_VALUE(5,3,'0' + lines_cleared_ten,0);
	//UPDATE_TILE_BY_VALUE(6,3,'0' + lines_cleared_one,0);
	// one_vram_buffer('0' + lines_cleared_hundred, get_ppu_addr(cur_nt,4<<3,3<<3));
	// one_vram_buffer('0' + lines_cleared_ten, get_ppu_addr(cur_nt,5<<3,3<<3));
	// one_vram_buffer('0' + lines_cleared_one, get_ppu_addr(cur_nt,6<<3,3<<3));
}

void display_score()
{
	static unsigned long temp_score;
	static unsigned char i;

	temp_score = cur_score;

	//PRINT(2, 7, "SCORE");

	// clear out any old score.
	multi_vram_buffer_horz("      ", 6, get_ppu_addr(cur_nt, 0, 6<<3));
	PRINT(0, 8, "0000000");

	i = 0;
	while(temp_score != 0)
    {
        unsigned char digit = temp_score % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(cur_nt, (6<<3) - (i << 3), 6<<3 ));

		PRINT(6 - i, 8, digits[digit]);

        temp_score = temp_score / 10;
		++i;
    }
}

#if !VS_SYS_ENABLED		
void display_highscore()
{
	static unsigned long temp_score;
	static unsigned char i;

	temp_score = savegame.high_scores[savegame.attack_style];

	// clear out any old score.
	multi_vram_buffer_horz("0000000", 7, get_ppu_addr(0, 17<<3, 27<<3));
	PRINT(6, 13, "00000000");

	i = 0;
	while(temp_score != 0)
    {
        unsigned char digit = temp_score % 10;
        one_vram_buffer('0' + digit, get_ppu_addr(0, (23<<3) - (i << 3), 27<<3 ));
		PRINT(13 - i, 13, digits[digit]);

        temp_score = temp_score / 10;
		++i;
    }
}
#endif //#if !VS_SYS_ENABLED

void display_level()
{
	// We let level be displayed as zero based because it makes more sense when
	// comparing it to lines (eg. lines is 80, level is 8).
	static unsigned char temp_level;
	static unsigned char i;
	static unsigned char res[2];

	//PRINT(2, 9, "LEVEL");

	temp_level = cur_level;
	i = 0;

	if (cur_level < 10)
	{
		multi_vram_buffer_horz("00", 2, get_ppu_addr(cur_nt,5<<3,9<<3));
		PRINT(5, 10, "0");
		uitoa(cur_level, res, 10);
		PRINT(6, 10, res);
	}
	else
	{
		uitoa(cur_level, res, 10);
		PRINT(5, 10, res);
	}

	// while(temp_level != 0)
    // {
    //     unsigned char digit = temp_level % 10;
    //     one_vram_buffer('0' + digit, get_ppu_addr(cur_nt, (6<<3) - (i << 3), 9<<3 ));

	// 	PRINT(7 - i, 13, '0' + digit);

    //     temp_level = temp_level / 10;
	// 	++i;
    // }
}

// START OF ROW CLEAR SEQUENCE!

void clear_rows_in_data(unsigned char start_y)
{
	static unsigned char line_complete;
	static unsigned char i;
	static unsigned char prev_level;
//PROFILE_POKE(0x9f); //blue
	i = 0;
	prev_level = cur_level;

	// 0xff used to indicate unused.
	memfill(lines_cleared_y, 0xff, 4);
//PROFILE_POKE(0x3f); //red
	// Start at the bottom of the board, and work our way up.
	for (local_iy = start_y; local_iy > BOARD_OOB_END; --local_iy)
	{
		// Assume this row is complete unless we find an empty
		// block.
		line_complete = 1;
		for (local_ix = 0; local_ix <= BOARD_END_X_PX_BOARD; ++local_ix)
		{
//PROFILE_POKE(0x5f); //green
			if (game_board[TILE_TO_BOARD_INDEX(local_ix,local_iy)] == EMPTY_TILE || game_board[TILE_TO_BOARD_INDEX(local_ix,local_iy)] == KILL_SCREEN_TILE)
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
			for (local_ix = 0; local_ix < BOARD_WIDTH; ++local_ix)
			{
				if (attack_row_status[local_ix] > ATTACK_QUEUE_SIZE && attack_row_status[local_ix] - (ATTACK_QUEUE_SIZE+1) >= (BOARD_END_Y_PX_BOARD - local_iy))
				{
					hit_reaction_remaining = 60;
					//debug_display_number(hit_reaction_remaining, 1);
				}
			}

			inc_lines_cleared();

			// Fill the row will empty data.
			memcpy(&game_board[TILE_TO_BOARD_INDEX(0, local_iy)], empty_row, 10);

			// Keep track of rows that we cleared so that they can be quickly
			// collapsed later.
			lines_cleared_y[i] = local_iy;

			// Remember that we cleared some lines.
			++i;
		}
	}

	// If any lines we cleared, time to move to the next phase...
	if (i > 0)
	{
		if (prev_level != cur_level)
		{
			if (i == 4)
			{
				//screen_shake_remaining = 5;
				SFX_PLAY_WRAPPER(SOUND_LEVELUP_MULTI);
			}
			else
			{
				SFX_PLAY_WRAPPER(SOUND_LEVELUP);
			}
				
			level_up_remaining = 60;
		}
		else if (i == 4)
		{
			// play a shake on big drops. This will happen after the rows are cleared
			// and the pieces fall.
			//screen_shake_remaining = 5;
			SFX_PLAY_WRAPPER(SOUND_MULTIROW);
		}
		else
		{
			//screen_shake_remaining = 5;
			SFX_PLAY_WRAPPER(SOUND_ROW);
		}

		// Send one less than the number of rows cleared to the
		// other player as garbage.
		if (i > 1)
		{
			unsigned char j = (i == 4) ? i : i - 1;

			if (j > garbage_row_queue)
			{

				// BUG IN CASE OF CLEARING 4 WHILE QUEUED UP IS 1: 
				// Sends 4-1=3, which gets upgraded to 4 on reciever.
				// No way to send 3 rows atm. 
				// Decided it is ok to just not let 1 row impact Tetris.

				j -= garbage_row_queue;
				garbage_row_queue = 0;

				// Add the number of rows to send to the packet.
				// j can be 4, so make sure it gets clamped to 3.
				queued_packet |= MIN(j, 3);
			}
			else if (garbage_row_queue >= j)
			{
				// The is more queued up than we cleared, so just reduce
				// the queue by the amount we would have sent.
				garbage_row_queue -= j;
			}
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
	static INT16 ix;
	static INT16 iy;

	//multi_vram_buffer_vert(const char * data, unsigned char len, int ppu_address);

	// Start in the middle of th board, and reveal outwards:
	// 4,5 -> 3,6 -> 2,7 -> 1,8 -> 0,9


	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.
	wait_vbl_done();
	clear_vram_buffer();

	// Clear OAM prior to drawing fresh.
	ClearOAMs();	
		
	// Force the sprites to hide themselves.
	draw_gameplay_sprites();

	// Force the OAM to update with the sprites
	// now hidden.
	SwapOAMs();	


	// Reveal from the center out.
	for (ix = 4; ix >= 0; --ix)
	{
		// LEFT SIDE

		// copy a column into an array.
		for (iy = 0; iy < BOARD_HEIGHT; ++iy)
		{
			copy_board_data[iy] = game_board[TILE_TO_BOARD_INDEX(ix, iy + BOARD_OOB_END + 1)];
			// Update_Tile fails if I try to just use a temp variable to store board data, or send
			// game_board directly. Using the copy array seems to work fine.
			UPDATE_TILE_BY_VALUE(
				(BOARD_START_X_PX >> 3) + (ix), 
				iy, //(BOARD_START_Y_PX >> 3) + (BOARD_OOB_END + 1) + iy - 4,
				copy_board_data[iy],
				0x10);
		}

		// multi_vram_buffer_vert(
		// 	copy_board_data,
		// 	BOARD_HEIGHT,
		// 	get_ppu_addr(
		// 		cur_nt,
		// 		BOARD_START_X_PX + (ix << 3),
		// 		BOARD_START_Y_PX + ((BOARD_OOB_END + 1) << 3)));

		// RIGHT SIDE


		for (iy = 0; iy < BOARD_HEIGHT; ++iy)
		{
			copy_board_data[iy] = game_board[TILE_TO_BOARD_INDEX(BOARD_END_X_PX_BOARD - ix, iy + BOARD_OOB_END + 1)];
			UPDATE_TILE_BY_VALUE(
				(BOARD_START_X_PX >> 3) + (BOARD_END_X_PX_BOARD - ix), 
				iy, //(BOARD_START_Y_PX >> 3) + (BOARD_OOB_END + 1) + iy - 4,
				copy_board_data[iy],
				0x10);
		}

		// multi_vram_buffer_vert(
		// 	copy_board_data,
		// 	BOARD_HEIGHT,
		// 	get_ppu_addr(
		// 		cur_nt,
		// 		BOARD_START_X_PX + ((BOARD_END_X_PX_BOARD - ix) << 3),
		// 		BOARD_START_Y_PX + ((BOARD_OOB_END + 1) << 3)));

		// Flash the screen when leveling up.
		if (level_up_remaining > 0)
		{			
			if (ix % 2 == 1)
			{
				// CGB
				set_bkg_palette(1, 1, palette_flash);
				// DMG
				BGP_REG = PAL_DEF(1, 2, 3, 3);
			}
			else
			{
				set_bkg_palette(1, 1, palette_normal);
				BGP_REG = PAL_DEF(0, 1, 2, 3);
			}
		}

		// Reveal these 2 new columns, and then move to the next one.
		vbl_delay(5);
		clear_vram_buffer();
	}

	// Just in case...
	BGP_REG = PAL_DEF(0, 1, 2, 3);
	set_bkg_palette(1, 1, palette_normal);

	// Move on to the next phase...
	try_collapse_empty_row_data();
}

void try_collapse_empty_row_data(void)
{
	// cannot use global ix,ix for some reason. Causes Kraken to not
	// retreat when hit.
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
				wait_vbl_done();
				ClearOAMs();	
				draw_gameplay_sprites();
				SwapOAMs();	
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
	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.

	// This also gets called when going back to the main menu.
	if (state == STATE_GAME)
	{
		ClearOAMs();	
		draw_gameplay_sprites();
		SwapOAMs();	
	}
	//delay(1);
	//clear_vram_buffer();
	//return;

	for (local_ix = 0; local_ix <= BOARD_END_X_PX_BOARD; ++local_ix)
	{
		// copy a column into an array.
		for (local_iy = 0; local_iy < BOARD_HEIGHT; ++local_iy)
		{
			copy_board_data[local_iy] = game_board[TILE_TO_BOARD_INDEX(local_ix, local_iy + BOARD_OOB_END + 1)];

			// if (iy <= STRESS_MUSIC_LEVEL && copy_board_data[iy] != 0)
			// {
			// 	fast_music = 1;
			// }

			UPDATE_TILE_BY_VALUE(
				(BOARD_START_X_PX >> 3) + (local_ix), 
				(BOARD_START_Y_PX >> 3) + (BOARD_OOB_END + 1) + local_iy - 4,
				copy_board_data[local_iy],
				0x10);
		}

		// multi_vram_buffer_vert(
		// 	copy_board_data,
		// 	BOARD_HEIGHT,
		// 	get_ppu_addr(
		// 		cur_nt,
		// 		BOARD_START_X_PX + (local_ix << 3),
		// 		BOARD_START_Y_PX + ((BOARD_OOB_END + 1) << 3)));

		// delay often enough to avoid buffer overrun.
		if (local_ix % 3 == 0)
		{
			// calling this again here isn't needed, as time will not have advanced, so
			// drawing the sprites again will do nothing.
			//draw_gameplay_sprites();
//PROFILE_POKE(PROF_CLEAR);
			wait_vbl_done();
			clear_vram_buffer();
		}
//PROFILE_POKE(PROF_R);
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
					if (game_board[TILE_TO_BOARD_INDEX(ix, iy)] == EMPTY_TILE)
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

			// found the attack, exit the search loop.
			break;
		}
	}

	// TODO: Compare to level expectations.
	if (attacks == 0)
	{
		// where to start the attack!
		attack_row_status[(unsigned char)rand() % BOARD_WIDTH] = 1;
	}
	else
	{
		// TODO: Only if changed above.
		//copy_board_to_nt();

		// custom logic to update nt in 1 single row.

		// Clear out any existing vram commands to ensure we can safely do a bunch
		// of work in this function.

		// This also gets called when going back to the main menu.
		if (state == STATE_GAME)
		{
			ClearOAMs();	
			draw_gameplay_sprites();
			SwapOAMs();	
		}

		local_ix = ix;

		// copy a column into an array.
		for (local_iy = 0; local_iy < BOARD_HEIGHT; ++local_iy)
		{
			copy_board_data[local_iy] = game_board[TILE_TO_BOARD_INDEX(local_ix, local_iy + BOARD_OOB_END + 1)];

			UPDATE_TILE_BY_VALUE(
				(BOARD_START_X_PX >> 3) + (local_ix), 
				(BOARD_START_Y_PX >> 3) + (BOARD_OOB_END + 1) + local_iy - 4,
				copy_board_data[local_iy],
				0x10);
		}
	}
}

void add_row_at_bottom()
{
	memfill(&game_board[TILE_TO_BOARD_INDEX(0, BOARD_END_Y_PX_BOARD - kill_row_cur)], KILL_SCREEN_TILE, BOARD_WIDTH);
	++kill_row_cur;
	copy_board_to_nt();
}

void add_garbage_row_at_bottom(UINT8 num_rows)
{
	static UINT8 dest_row;

	SFX_PLAY_WRAPPER(SOUND_BLOCKED);

	for(dest_row = 0; dest_row <= (BOARD_END_Y_PX_BOARD - num_rows); ++dest_row)
	{
		memcpy(&game_board[TILE_TO_BOARD_INDEX(0, dest_row)], &game_board[TILE_TO_BOARD_INDEX(0, dest_row + num_rows)], BOARD_WIDTH);
	}

	for (dest_row = (BOARD_END_Y_PX_BOARD - num_rows + 1); dest_row <= BOARD_END_Y_PX_BOARD ; ++dest_row)
	{
		memcpy(&game_board[TILE_TO_BOARD_INDEX(0, dest_row)], garbage_row + garbage_offset, BOARD_WIDTH);

		++garbage_row_count;

		if (garbage_row_count >= 9)
		{
			garbage_offset = (unsigned char)rand() % 10u;
			garbage_row_count = 0;
		}
	}

	copy_board_to_nt();
}


void reset_gameplay_area()
{
	memfill(game_board, EMPTY_TILE, BOARD_SIZE);
	memfill(attack_row_status, 0, BOARD_WIDTH);
	// where to start the attack!
	attack_row_status[(unsigned char)rand() % BOARD_WIDTH] = 1;

	// Reset stats.
	lines_cleared_one = lines_cleared_ten = lines_cleared_hundred = cur_score = 0;
	cur_level = savegame.saved_starting_level;
	fall_rate = fall_rates_per_level[MIN(cur_level, sizeof(fall_rates_per_level))];
	row_to_clear = -1;
	delay_lock_remaining = -1;
	kill_row_cur = 0;
	start_delay_remaining = START_DELAY;
	attack_queued = 0;
	delay_spawn_remaining = -1;
	level_up_remaining = 0;
	garbage_row_queue = 0;

	garbage_offset = (unsigned char)rand() % 10u;
	garbage_row_count = 0;

	// We don't want the previous round's blocks to impact the choice
	// the starting blocks next round, as that will cause a desync in 
	// sio matches.
	// TODO: Would -1 be better? I think the way it is, it would be
	// very rare for piece 0 to get picked.
	memfill(&next_cluster, 0, sizeof(next_cluster));
	memfill(&cur_cluster, 0, sizeof(cur_cluster));

	// load the palettes
	time_of_day = 0;
	pal_bg(palette_bg_list[time_of_day]);
	pal_spr(palette_sp);

	// Clear the skull if needed.
	one_vram_buffer(0x8, get_ppu_addr(2, 4<<3, 9<<3));

	display_lines_cleared();
	display_score();
	display_level();

	oam_clear();

	// clear the "next" block for cases of restarting.
	multi_vram_buffer_horz(empty_row, 4, get_ppu_addr(cur_nt, 120, 16));
	multi_vram_buffer_horz(empty_row, 4, get_ppu_addr(cur_nt, 120, 24));

	// Reset the ppu for gameover case.
	copy_board_to_nt();
}


#if !VS_SYS_ENABLED	
void display_song()
{
	static unsigned char num[3];
	UIntToString(test_song, num);
	PRINT(0,0,num);
}

void display_sound()
{	
	static unsigned char num[3];
	UIntToString(test_sound, num);
	PRINT(0,1,num);

}

#endif //#if !VS_SYS_ENABLED	

void display_options()
{
 	static unsigned char start_y = 16;
	static unsigned char i;

	// TODO: Could be smarter and only update the line that changed, and delay
	// 		 could probably be removed.
	// Avoid overrun when mashing mode change.
	wait_vbl_done();
	clear_vram_buffer();

	multi_vram_buffer_horz(&starting_levels[cur_level], 1, get_ppu_addr(0,17<<3,start_y<<3));
	PRINT(15,7, digits[cur_level]);
	multi_vram_buffer_horz(attack_style_strings[savegame.attack_style], ATTACK_STRING_LEN, get_ppu_addr(0,17<<3,(start_y+2)<<3));
	PRINT(5,3, attack_style_strings[savegame.attack_style]);
	PRINT(5,4, attack_style__desc_strings_01[savegame.attack_style]);
	PRINT(5,5, attack_style__desc_strings_02[savegame.attack_style]);
	multi_vram_buffer_horz(off_on_string[savegame.music_on], OFF_ON_STRING_LEN, get_ppu_addr(0,17<<3,(start_y+4)<<3));
	PRINT(13,8, off_on_string[savegame.music_on]);
	multi_vram_buffer_horz(off_on_string[savegame.sfx_on], OFF_ON_STRING_LEN, get_ppu_addr(0,17<<3,(start_y+6)<<3));
	PRINT(13,9, off_on_string[savegame.sfx_on]);
	multi_vram_buffer_horz(hard_drop_types[savegame.hard_drops_on], HARD_DROP_STRING_LEN, get_ppu_addr(0,17<<3,(start_y+8)<<3));
	PRINT(12,10, hard_drop_types[savegame.hard_drops_on]);

	// // NOTE: One redundant call.
	// multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 7<<3, (start_y)<<3));
	// multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 7<<3, (start_y+2)<<3));
	// multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 7<<3, (start_y+4)<<3));
	// multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 7<<3, (start_y+6)<<3));
	// multi_vram_buffer_horz(option_empty, 2, get_ppu_addr(0, 7<<3, (start_y+8)<<3));

	// for (i = 0 ; i < 5; ++i)
	// {
	// 	UPDATE_TILE_BY_VALUE(1, 6 + (i*2), 0, 0x10);
	// 	UPDATE_TILE_BY_VALUE(2, 6 + (i*2), 0, 0x10);
	// }

	// multi_vram_buffer_horz(option_icon, 2, get_ppu_addr(0, 7<<3, (start_y + (cur_option<<1)<<3)));
	// UPDATE_TILE_BY_VALUE(1, 6 + (cur_option * 2), 1, 0x10);
	// UPDATE_TILE_BY_VALUE(2, 6 + (cur_option * 2), 2, 0x10);

	// Avoid overrun when mashing mode change.
	//wait_vbl_done();
	//clear_vram_buffer();
}

void fade_to_black()
{
	// pal_bright(3);
	// delay(2);
	// pal_bright(2);
	// delay(2);
	// pal_bright(1);
	// delay(2);
	// pal_bright(0);
	// //delay(2);

	FadeIn();
	DISPLAY_OFF;
}

void fade_from_black()
{
	// pal_bright(1);
	// delay(2);
	// pal_bright(2);
	// delay(2);
	// pal_bright(3);
	// delay(2);
	// pal_bright(4);
	// //delay(2);

	// Fade function on DMG reads the colors directly from registers before fading.
	// As a result, the FadeOut fails by default because it trys to fade to a solid
	// color.
	// To fix this, we set the real colors right before FadeOut to set the proper
	// destination colors.
	// if (state == STATE_OPTIONS && sgb_check() != 0 )
	// {
	// 	BGP_REG = PAL_DEF(3, 2, 1, 0);
	// }
	// else
	{
		BGP_REG = PAL_DEF(0, 1, 2, 3);
	}
	// NOTE: Shifting colors right to account for transparent Col0.
	OBP0_REG = PAL_DEF(0, 0, 1, 2);
	// Blinking requires darkest shade. Although, if this becomes a problem
	// that portion of the sprite make be able to be transparent and just
	// show the black background tile for the eyebrow. Not sure.
	OBP1_REG = PAL_DEF(0, 0, 1, 3);
	// Now in fadein code to avoid flicker when vblank waiting.
//	DISPLAY_ON;
	FadeOut();
}

#if PLAT_NES
void difficulty_to_leaderboard_pos(unsigned char dif)
{
	switch (dif)
	{
	case 0:
		in_x = 4;
		in_y = 12;
		break;
	case 1:
		in_x = 18;
		in_y = 12;
		break;
	case 2:
		in_x = 4;
		in_y = 22;
		break;
	case 3:
		in_x = 18;
		in_y = 22;
		break;
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

	// Clear out any existing vram commands to ensure we can safely do a bunch
	// of work in this function.

	wait_vbl_done();
	clear_vram_buffer();

	for (local_ix = 0; local_ix <= BOARD_END_X_PX_BOARD; ++local_ix)
	{
		// copy a column into an array.
		for (local_iy = 0; local_iy < BOARD_HEIGHT; ++local_iy)
		{
			copy_board_data[local_iy] = '0' + game_board[TILE_TO_BOARD_INDEX(local_ix, local_iy + BOARD_OOB_END + 1)];
		}

		multi_vram_buffer_vert(
			copy_board_data,
			BOARD_HEIGHT,
			get_ppu_addr(
				cur_nt,
				BOARD_START_X_PX + (local_ix << 3),
				BOARD_START_Y_PX + ((BOARD_OOB_END + 1) << 3)));

		// delay often enough to avoid buffer overrun.
		if (local_ix % 4 == 0)
		{
			wait_vbl_done();
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
	wait_vbl_done();
	clear_vram_buffer();
}
#endif //DEBUG_ENABLED
#endif // PLAT_NES

void send_queued_packet()
{
	if (is_sio_game && queued_packet != 0)
	{
		++packet_count_out;
		// PRINT_POS(0,0);
		// Printf("O:%d %d ", packet_count_out, queued_packet);
		_io_out = queued_packet;

		do {
			send_byte();
			/* Wait for IO completion... */
			while((_io_status == IO_SENDING));
		 // If this packet is required, keep trying until it sends! This will deadlock
		 // if host and client have required packets at the same time, or the cable gets
		 // disconnected which a packet is queued up.
		 // TODO: Detect timeout and disconnect, but make sure its not enough for all the
		 // 	  delay and fade calls.
		} while(queued_packet_required && (_io_status != IO_IDLE));

		if (_io_status == IO_ERROR)
		{
			--packet_count_out;
		}

		// Probably not needed since queued packet is cleared at the start of
		// the frame.
		queued_packet = queued_packet_required = 0;

		// Start listening again...
		receive_byte();
	}
}