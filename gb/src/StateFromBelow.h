/** (C) Matt Hughson 2020 */

// VS_SYS_ENABLED: When true, the game is compiled targeting the Nintendo Vs Arcade System.
//                 It will still build a .nes file, and Vs ROM files will need to be generated
//                 manually outside of this build system.
//                 This flag is set from compile_master.bat.

// VS_SRAM_ENABLED: When true, the Vs System version of the game will attempt to use Shared
//                  Save RAM available on the Vs System. It will use that RAM for highscores.
//                  NOTE: This feature is INCOMPLETE and DOES NOT WORK.
//                  This flag is set from compile_master.bat.

// When set to 1, certain debug features will be enabled.
// Should be set to 0 before shipping.
#define DEBUG_ENABLED 0

#if DEBUG_ENABLED
// Used to profile sections of code using emphasis bit.
#define PROFILE_POKE(val) POKE((0x2001),(val));
#else
#define PROFILE_POKE(val)
#endif

// Different colors for used with PROFILE_POKE.
#define PROF_CLEAR 0x1e // none
#define PROF_R 0x3f // red
#define PROF_G 0x5f // green
#define PROF_B 0x9f // blue
#define PROF_W 0x1f // white

// Nametable A: 	2400-2000 = 400
// Attributes: 		2400-23c0 = 0x40
// Patterns: 		0x400-0x40 = 0x3c0
#define NAMETABLE_SIZE 0x400
#define NAMETABLE_PATTERN_SIZE 0x3c0

// The position, in pixels, where the play area starts (top left).
// Note: There are 4 tiles of space above the visible play area which are still 
//       used. Players can put blocks in that area above the visible board and 
//       not lose. The game isn't over unless a NEW block is blocked from spawning.
#define BOARD_START_X_PX 96
#define BOARD_START_Y_PX 0 //16
// The position, in pixels, where the play area ends (bottom right).
// NOTE: This is the position of the bottom right tile INCLUDED in the player area.
//       So the actual end position in pixels would be +8 in both directions.
#define BOARD_END_X_PX 168
#define BOARD_END_Y_PX (184) //(184 + 16) // not sure why +16. Likely just making an adjustment at some point and forgot to remove.

// The last tile in the Out of Bounds area at the top of the board.
// Again, this is the last tile in that area, so the actual number of rows in that
// area is 4 not 3.
#define BOARD_OOB_END 3
#define BOARD_END_X_PX_BOARD 9 // left edge of last block (width = 10)
#define BOARD_END_Y_PX_BOARD 23 // top edge of last block (height = 24)

// The number of tiles in the board (10x20 + 4 rows in the OOB area)
#define BOARD_SIZE 240
// The height of the main board area (not including OOB)
#define BOARD_HEIGHT (BOARD_END_Y_PX_BOARD - BOARD_OOB_END)
// +1 because that enum is the position of the last tile, not the width of the board.
#define BOARD_WIDTH (BOARD_END_X_PX_BOARD + 1)

// Convert a tile position in x,y to an index into the linear board array.
#define TILE_TO_BOARD_INDEX(x,y) ((board_lookup_y[(y)]) + (x))

#define BLINK_LEN (60 * 5)

#define SFX_PLAY_WRAPPER(id) if (sfx_on) { sfx_play((id), 0); }
// play a sound effect that is treated like music to the user (jingles, etc).
#define SFX_MUSIC_PLAY_WRAPPER(id) if (music_on) { sfx_play((id), 0); }
#define MUSIC_PLAY_WRAPPER(id) if (music_on) { music_play((id)); }
#if VS_SYS_ENABLED
#define MUSIC_PLAY_ATTRACT_WRAPPER(id) if (music_on && (DIP8 == 0 || credits_remaining >= game_cost)) { music_play((id)); }
#else
// No attract mode on the NES.
#define MUSIC_PLAY_ATTRACT_WRAPPER(id) MUSIC_PLAY_WRAPPER(id)
#endif // VS_SYS_ENABLED
#define SKULL_SPRITE 0x3b

// The time before another code will be accepted.
#define CREDIT_DELAY 70

#if VS_SYS_ENABLED
// Note: 1 based to match documentation and user face numbering.
#define DIP1 (PEEK(0x4016) & 1<<3) // FREE PLAY
#define DIP2 (PEEK(0x4016) & 1<<4) // EXTRA COIN COST

#define DIP3 (PEEK(0x4017) & 1<<2) // PPU.0
#define DIP4 (PEEK(0x4017) & 1<<3) // PPU.1
#define DIP5 (PEEK(0x4017) & 1<<4) // PPU.2

#define DIP6 (PEEK(0x4017) & 1<<5) // MUSIC OFF
#define DIP7 (PEEK(0x4017) & 1<<6) // SFX OFF
#define DIP8 (PEEK(0x4017) & 1<<7) // MUTE MAIN MENU
#endif // VS_SYS_ENABLED

// Delay in the settings screens before the player options are auto-chosen for them.
#define AUTO_FORWARD_DELAY (60*30)
#define HIGH_SCORE_ENTRY 4
#define NO_SCORE (0xffffffff)

#if VS_SRAM_ENABLED
#define IS_PRIMARY_CPU ((PEEK(0x4016) & (1 << 7)) == 0)
#else
#define IS_PRIMARY_CPU (1)
#endif // #if VS_SRAM_ENABLED

// Essentially the different game modes, desribed by the behaviour of the Kraken.
enum 
{ 
    // The Kraken advances every time a block lands.
    ATTACK_ON_LAND, 
    
    // The Kraken advances after a certain amount of time passes.
    // See: attack_delay
    ATTACK_ON_TIME, 
    
    // Kraken disabled aka Classic mode.
    ATTACK_NEVER, 
    
    // The number of game modes.
    ATTACK_NUM 
};

#if VS_SRAM_ENABLED
#pragma bss-name(push, "XRAM")
unsigned char xram_test[8];
#if VS_SYS_ENABLED
unsigned char high_scores_vs_initials[ATTACK_NUM][4][3][3];
unsigned long high_scores_vs_value[ATTACK_NUM][4][3];
#endif //#if VS_SYS_ENABLED
#pragma bss-name(pop)
#endif // #if VS_SRAM_ENABLED


#if PLAT_NES
#pragma bss-name(push, "ZEROPAGE")
#endif // PLAT_NES

// GLOBAL VARIABLES

// Really just a location.
struct block
{
    unsigned char x;
    unsigned char y;
};

// A cluster is a collection of 8x8 blocks making up a tetromino.
struct cluster
{
    // The current layout. 1 entry from "def".
    unsigned char layout[4];
    // The 4 rotations defining this block.
    // Each entry in the array is an index into index_to_x/y_lookup.
    // That look up contains x/y offsets from the position of this block.
    const unsigned char def[4][4];
    // The sprite to use when drawing this cluster.
    unsigned char sprite;
    // An index for this particular cluster type, used to index into different
    // arrays for type specific information.
    unsigned char id;
};

// small number tracking how many frames have passed. Good for things
// that just need to change over time, but are ok looping every 256 frames.
unsigned char tick_count;
// Larger number tracking how many frames have passed. More expensive to use
// but doesn't repeat often.
unsigned int tick_count_large;
// How many frames has the game been in the current state.
unsigned int ticks_in_state_large;
// When the Kraken is hit by a line clear, it plays a reaction. This tracks how
// much time is LEFT in the reaction. When it hits zero, reaction over.
unsigned char hit_reaction_remaining;
// In the ATTACK_ON_TIME game mode, the Kraken attacks after a certain amount of
// time. This is how much time until the next attack (in frames).
unsigned int attack_queue_ticks_remaining;
// The ammount of time (in frames) between each attack of the Kraken when playing
// the ATTACK_ON_TIME game mode. 10 seconds.
const unsigned int attack_delay = 600;
// Buttons that were pressed or held this frame (both player 1 and 2).
unsigned char pad_all;
// Buttons that were pressed this frame (but not last frame) (both player 1 and 2).
unsigned char pad_all_new;
// Same as above but for player 1 and 2 specifically.
unsigned char pad1;
unsigned char pad1_new;
unsigned char pad2;
unsigned char pad2_new;
// The current amount of vertical scrolling.
unsigned int scroll_y_game;

unsigned int scroll_x_camera;
unsigned int scroll_y_camera;

// The number of options on the settings screen.
#define NUM_OPTIONS 5
// The currently selected option on the settings screen.
unsigned char cur_option;

// The current game mode. See ATTACK_ON_TIME and other at the top of this file.
unsigned char attack_style;

// Length of the string used to describe the game type ("Classic", etc) on the settings
// screen.
#define ATTACK_STRING_LEN 7

#if VS_SYS_ENABLED
// unsigned char high_scores_vs_initials[ATTACK_NUM][4][3][3] = 
// { 
//     // ATTACK_ON_LAND/FIXED
//     {
//         { "1FE", "2FE", "3FE" }, // EASY
//         { "1FM", "2FM", "3FM" }, // MED
//         { "1FI", "2FI", "3FI" }, // INSANE
//         { "1FD", "2FD", "3FD" }, // DEATH
//     },
//     // ATTACK_ON_TIME/TIMED
//     {
//         { "1TE", "2TE", "3TE" }, // EASY
//         { "1TM", "2TM", "3TM" }, // MED
//         { "1TI", "2TI", "3TI" }, // INSANE
//         { "1TD", "2TD", "3TD" }, // DEATH
//     },
//     // ATTACK_NEVER/CLASSIC
//     {
//         { "1CE", "2CE", "3CE" }, // EASY
//         { "1CM", "2CM", "3CM" }, // MED
//         { "1CI", "2CI", "3CI" }, // INSANE
//         { "1CD", "2CD", "3CD" }, // DEATH
//     },
// };

// Temporary table used for manipulating high score table.
unsigned char *temp_table;
unsigned char last_initials[3] = "AAA";

#if !VS_SRAM_ENABLED
unsigned char high_scores_vs_initials[ATTACK_NUM][4][3][3] = 
{ 
    // ATTACK_ON_LAND/FIXED
    {
        { "PMY", "---", "---" }, // EASY
        { "PEL", "---", "---" }, // MED
        { "NAH", "---", "---" }, // INSANE
        { "PEL", "---", "---" }, // DEATH
    },
    // ATTACK_ON_TIME/TIMED
    {
        { "NAH", "---", "---" }, // EASY
        { "PEL", "---", "---" }, // MED
        { "KRB", "---", "---" }, // INSANE
        { "PMY", "---", "---" }, // DEATH
    },
    // ATTACK_NEVER/CLASSIC
    {
        { "PEL", "---", "---" }, // EASY
        { "PEL", "---", "---" }, // MED
        { "CHZ", "---", "---" }, // INSANE
        { "PMY", "---", "---" }, // DEATH
    },
};

// unsigned long high_scores_vs_value[ATTACK_NUM][4][3] = 
// { 
//     // ATTACK_ON_LAND/FIXED
//     {
//         { 1234567, 12345, 12 }, // Easy
//         { 1234567, 12345, 12 }, // Med
//         { 1234567, 12345, 12 }, // Insane
//         { 1234567, 12345, 12 }, // Death
//     },
//     {        
//         { 1234567, 12345, 12 }, // Easy
//         { 1234567, 12345, 12 }, // Med
//         { 1234567, 12345, 12 }, // Insane
//         { 1234567, 12345, 12 }, // Death
//     },
//     {        
//         { 1234567, 12345, 12 }, // Easy
//         { 1234567, 12345, 12 }, // Med
//         { 1234567, 12345, 12 }, // Insane
//         { 1234567, 12345, 12 }, // Death
//     },
// };

unsigned long high_scores_vs_value[ATTACK_NUM][4][3] = 
{ 
    // ATTACK_ON_LAND/FIXED
    {
        { 310000, NO_SCORE, NO_SCORE }, // Easy
        { 596000, NO_SCORE, NO_SCORE }, // Med
        { 431000, NO_SCORE, NO_SCORE }, // Insane
        { 259000, NO_SCORE, NO_SCORE }, // Death
    },
    // ATTACK_ON_TIME/TIMED
    {
        { 806000, NO_SCORE, NO_SCORE }, // Easy
        { 891000, NO_SCORE, NO_SCORE }, // Med
        { 1529000, NO_SCORE, NO_SCORE }, // Insane
        { 1024000, NO_SCORE, NO_SCORE }, // Death
    },
    // ATTACK_NEVER/CLASSIC
    {
        { 1057000, NO_SCORE, NO_SCORE }, // Easy
        { 1195000, NO_SCORE, NO_SCORE }, // Med
        { 1510000, NO_SCORE, NO_SCORE }, // Insane
        { 1460000, NO_SCORE, NO_SCORE }, // Death
    },
};
#endif // #if !VS_SRAM_ENABLED
unsigned char cur_initial_index;
#endif

// High scores for the 3 game modes. 32 bit numbers to allow for scores
// in the millions.
unsigned long high_scores[ATTACK_NUM] = { 0, 0, 0}; // NOTE: long!

// Is the music and sound currently enabled?
unsigned char music_on;
unsigned char sfx_on;
// Length of the string used to display "on" and "off" in the settings screen.
#define OFF_ON_STRING_LEN 4

// Originally the game was going to feature different clusters, other than 
// tetrominos, but that ended up getting cut. This is legacy from that and could
// be stripped out.
enum {BLOCK_STYLE_MODERN, BLOCK_STYLE_CLASSIC};
unsigned char block_style;
#define BLOCK_STYLE_STRING_LEN 7

// Display for the starting levels on the settings screen.
const unsigned char starting_levels[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
// After the starting level is chosen, we save that (in RAM) so that if 
// the player restarts, they restart at the same level.
// NOTE: The game does not feature battery save.
unsigned char saved_starting_level;

// This stores an enum for the 3 types of hard drop (off, hold, and tap).
unsigned char hard_drops_on;
// In the case of "hold" hard drops, this tracks how much time remains of
// the player holding UP before the block hard drops.
unsigned char hard_drop_hold_remaining;
// The amount of time the player must hold UP before a harddrop is executed.
// In "Tap" mode, this is 0 frames (instant).
unsigned char hard_drop_tap_required;

// The different states the game can be in.
// These get set via go_to_state.
enum { STATE_BOOT, STATE_TY, STATE_MENU, STATE_OPTIONS, STATE_GAME, STATE_PAUSE, STATE_OVER, STATE_SOUND_TEST, STATE_HIGH_SCORE_TABLE };
// The state the game is in right now.
unsigned char state = STATE_BOOT;

// Konami code is used on the title screen to enter Sound Test.
// It works by storing a current index in to the code. If the player
// presses the button at that index in the code array, the index advnaces.
// If they press anything else, the index goes back to 0 (resets).
#define KONAMI_CODE_LEN 11
const unsigned char konami_code[KONAMI_CODE_LEN] = { PAD_UP, PAD_UP, PAD_DOWN, PAD_DOWN, PAD_LEFT, PAD_RIGHT, PAD_LEFT, PAD_RIGHT, PAD_B, PAD_A, PAD_START };
unsigned char cur_konami_index;


// The block operates in "logical space" from 0 -> w/h. The logical
// space is converted to screen space at time of render (or ppu get).
struct block cur_block = { 0, 0 };


// How many frames need to pass before it falls 8 pixels.
// This value will change as the level advances.
unsigned char fall_rate = 48;
// The current level the player is on. Goes up by 1 every 10 lines cleared.
unsigned char cur_level = 0;
// When changing starting level, we actually change this linear number which then
// gets mapped to starting numbers.
#if VS_SYS_ENABLED
unsigned char cur_level_vs_setting = 0;
unsigned char high_score_entry_placement;
#define VS_CODE_LEN 7
const unsigned char vs_code[VS_CODE_LEN] = { PAD_RIGHT, PAD_RIGHT, PAD_RIGHT, PAD_DOWN, PAD_DOWN, PAD_DOWN, PAD_RIGHT };
unsigned char vs_code_index;
#endif // #if VS_SYS_ENABLED


// CLASSIC RIGHT-HANDED (NES)

// TODO: offset in pixels as well?
const unsigned char index_to_x_lookup[16] = { 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3 };
const unsigned char index_to_y_lookup[16] = { 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3 };

// The definition of a tetromino and it's 4 rotations (which can often repeat).
// Each rotation is as an array of 4 indexes into a 4x4 (16) grid of blocks.
// 
//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
// 12 13 14 15
//
// So, and definition of { 8, 9, 10, 11 } is a line straight across the 3rd row.
const unsigned char def_line[4][4] = 
{
    { 8, 9, 10, 11 },
    { 2, 6, 10, 14 },
    { 8, 9, 10, 11 },
    { 2, 6, 10, 14 }
};

const unsigned char def_square[4][4] =
{
    { 5, 6, 9, 10 },
    { 5, 6, 9, 10 },
    { 5, 6, 9, 10 },
    { 5, 6, 9, 10 }
};

const unsigned char def_L_rev[4][4] =
{
    { 4, 5, 6, 10 },
    { 1, 5, 8, 9 },
    { 0, 4, 5, 6 },
    { 1, 2, 5, 9 },
};

const unsigned char def_L[4][4] =
{
    { 4, 5, 6, 8 },
    { 0, 1, 5, 9 },
    { 2, 4, 5, 6 },
    { 1, 5, 9, 10 },
};

const unsigned char def_S[4][4] =
{
    { 5, 6, 8, 9 },
    { 1, 5, 6, 10 },
    { 5, 6, 8, 9 },
    { 1, 5, 6, 10 },
};

const unsigned char def_Z[4][4] =
{
    { 4, 5, 9, 10 },
    { 2, 5, 6, 9 },
    { 4, 5, 9, 10 },
    { 2, 5, 6, 9 },
};

const unsigned char def_T[4][4] =
{
    { 4, 5, 6, 9 },
    { 1, 4, 5, 9 },
    { 1, 4, 5, 6 },
    { 1, 5, 6, 9 },
};



// The number of types of tetrominos.
#define NUM_CLUSTERS (unsigned char)7

// Lookup array for each of the types of tetromino. When we used to have a modern
// set as well, this look up table made more sense, because I could just point to
// a different lookup table when the setting changed. Now it always points here.
const unsigned char** cluster_defs_classic [NUM_CLUSTERS] =
{
    def_Z,
    def_S,
    def_line, // if move update places that check for line via id.
    def_square,
    def_T,
    def_L,
    def_L_rev,
};

// The current rotation of the active cluster. 0-3.
unsigned char cur_rot;

// The currently active tetromino be placed by the player.
struct cluster cur_cluster;
// The tetromino queued to be spawned next.
struct cluster next_cluster;

// How many blocks the Kraken tentacle climbs before entering the board.
#define ATTACK_QUEUE_SIZE 3
// The max length of the tentacle before it automatically retreats.
#define ATTACK_MAX 10

// Tracks the height of the tentacle at each column of the board. However,
// the tentacle can only ever be at one place at at time, so this is kind of 
// a weird way to store this information. I think at one point I had multiple
// tentacle, but I'm not sure.
// This Row and Height is used to detecting when the player clears a line and
// hits the tentacle. It is also used to animate the tentacle backwards when 
// it retreats (see: row_to_clear).
unsigned char attack_row_status[BOARD_WIDTH];

// Each cluster uses a different sprite to help better differentiate itself
// from the other clusters. This array is the sprite each cluster type uses,
// in order.
const unsigned char cluster_sprites[NUM_CLUSTERS] =
{
    //0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 0xe, 
};

// The starting y offet (in tiles) each cluster type uses when spawning into
// gameplay.
const unsigned char cluster_offsets[NUM_CLUSTERS] = 
{
    3, 3, 2, 3, 3, 3, 3,
};

// DAS. The amount of delay before a held horizontal direction is
// accepted. Note the amount is different for the first repeat and 
// the repeats after that. (it gets doubled)
unsigned char horz_button_delay;
// The amount of delay before a held horizontal button press is accepted
// again (DAS). This gets double for the intitial delay.
const unsigned char button_delay = 5;
// When a new block is spawn (or the gameplay starts for the first time), we 
// require that the player release and press the down button again before we
// start soft dropping. This is to avoid needing to quickly release down before
// the next block spawns.
unsigned char require_new_down_button;
// Tracks how many frames a left to pass before the cluster will move to the 
// next space down.
unsigned char fall_frame_counter;
// The number of lines cleared is tracked via each digit induvidually.
// This is to make the display logic faster.
unsigned char lines_cleared_one;
unsigned char lines_cleared_ten;
unsigned char lines_cleared_hundred;
// The current score. Stored as 32bit number to allow for millions.
unsigned long cur_score; // NOTE: long!
// Originally the game used both nametables for clearing lines with no
// visible glitches (it would change in a single frame). Eventually I
// decided not to do that, but these variables are legacy from that and
// for the most part are unused.
unsigned char cur_nt;
unsigned char off_nt;

// movement()
char hit;
unsigned char temp_fall_frame_counter;
unsigned char old_x;
// spawn_new_cluster()
unsigned char id;
// put_cur_cluster()
unsigned char min_y;
unsigned char max_y;
// set_block()
unsigned char in_x; 
unsigned char in_y; 
unsigned char in_id;
// draw_gameplay_sprites()
unsigned char local_start_x;
unsigned char local_start_y;
unsigned char local_ix;
unsigned char local_iy;
unsigned int local_t;
unsigned char local_bit;
unsigned char local_row_status;
const unsigned char OOB_TOP = (BOARD_START_Y_PX + (BOARD_OOB_END << 3));

// Used by the sound test screen.
unsigned char test_song;
unsigned char test_song_active;
unsigned char test_sound;

// Index for each music track. This must start at 0 and line up with famitracker file.
enum { MUSIC_TITLE, MUSIC_GAMEPLAY, MUSIC_STRESS, MUSIC_PAUSE };

// Index for each sound effect, as it appears in the famitracker source file.
enum 
{ 
    SOUND_ROTATE, SOUND_LAND, SOUND_ROW, SOUND_MULTIROW, SOUND_GAMEOVER, 
    SOUND_START, SOUND_BLOCKED, SOUND_LEVELUP, SOUND_LEVELUP_MULTI, 
    SOUND_PAUSE, SOUND_MENU_HIGH, SOUND_MENU_LOW, SOUND_GAMEOVER_SONG
};

// During gameplay if the blocks reach high enough (See: STRESS_MUSIC_LEVEL) a more
// stressful version of the music will play. This tracks which music is playing.
unsigned char cur_gameplay_music;
// How high a block must be placed for the stressful music to start.
#define STRESS_MUSIC_LEVEL 7 // 5 blocks down from the out of bounds area
// Has a Kraken attack been queued up. This can happen in a number of ways
// and is intentionally delayed a frame to avoid too much happening on the frame
// that a cluster lands.
unsigned char attack_queued;

// Used for animating the wiggle of the tentacle (and reused for a couple other similar
// animations).
const char tenatcle_offsets[4] = { -1, 0, 1, 0 };

// When the Kraken tentacle retreats the blocks it leaves in its place are special
// partially destroyed-looking sprites. These are this sprites.
#define NUM_GARBAGE_TYPES 3
const unsigned char garbage_types[NUM_GARBAGE_TYPES] = { 0x60, 0x70, 0x2f };
// Rather than randomly selecting a garbage type, this counter just picks the 
// next one and loops back when it hits the max.
unsigned char cur_garbage_type;

// When a block lands at higher difficulty levels there is an extra delay before the
// block locks into place, allowing the player the ability to slide the piece around
// for a short time.
// However there is already a delay before a block attempts to lock into place, just 
// based on the fall speeed.
// So this DELAY_LOCK_LEN is actually a minimum TOTAL time since the last movedown.
// At slow speeds, where there is delay longer than 15 frames before the block moves
// no additional lock delay is needed. But at high levels, where say the blocks drop
// every frame, this lock delay will add an extra 14 frames before it locks.
#define DELAY_LOCK_LEN 15
signed char delay_lock_remaining;
// If the player is taps down when lock delay is active it will skip the rest of the
// delay. It will also skip if a hard drop is used.
unsigned char delay_lock_skip;

// The amount of time before another block is spawned after the previous one lands.
#define DELAY_SPAWN_LEN 5
signed char delay_spawn_remaining;

// How many frames the player must hold UP to trigger a Hard Drop when using the
// HOLD Hard Drop setting.
#define HARD_DROP_HOLD_TIME 2

// Poorly named, this is actually the COLUMN of Tentacle that is currently retreating.
// -1 if not active.
signed char row_to_clear;

// When the player reaches level 30, a row of unbreakable blocks is added to the bottom
// of the screen, called a "kill row". Every level (10 lines) anoter kill row is added
// until level 40 where it stop (10 levels total).
unsigned char kill_row_cur;
unsigned char kill_row_queued;

// The first block of the game has an extra delay where the player can move
// the block left and right to give them a little breathing room before the
// match begins.
// NOTE: Pressing down skips the delay, while hard drops do not.
#define START_DELAY 120
unsigned char start_delay_remaining;

// Quick look up for y position (in tiles) to index into the board data. Basically just n*10.
const unsigned char board_lookup_y[24] = 
{
    0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230
};

// Temp variable used for storing nametable addresses.
int address;
// Current palette being used (See: palette_bg_list).
unsigned char time_of_day;
// While the background palette is overwritten completely when advancing the time of day,
// the sprites only use a few of the colors. This temp palette copies the parts
// of the background it needs, and levels the rest.
unsigned char temp_pal[16];
// Keep track of rows that we cleared so that they can be quickly
// collapsed later.
// NOTE: That the most rows that can be cleared in a single move is 4.
char lines_cleared_y[4];
// Temp variable used for calculating the score given for line(s) cleared.
unsigned int line_score_mod;

#if PLAT_NES
#pragma bss-name(push, "BSS")
#endif // PLAT_NES

// Defined in crt0.s
// Used by the Vs System.
extern unsigned char CREDITS_QUEUED;
extern unsigned char PPU_VERSION;

// This is the current state of the game board, in a linear array.
// 0 means empty, an otherwise it is the sprite index of the block
// that is being stored there.
unsigned char game_board[BOARD_SIZE];
// We need to make a copy of game_board, because memcpy can not copy over itself.
// memmove would be function to use, but it does not exist in this library.
unsigned char game_board_temp[BOARD_SIZE];
// An empty row of the board is just 10 0's. Used for quick memcpy.
const char empty_row[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
// When selecting an option in the settings screen, and little cursor moves around.
// This is done via the nametable, and these are the sprites used to clear and set
// that cursor.
const unsigned char option_empty[] = {0x0, 0x0};
const unsigned char option_icon[] = {0x25, 0x26};

// copy_board_to_nt()
char copy_board_data[BOARD_HEIGHT];

// 2C03
// 333,014,006,326,403,    503,510,420,320,120,    031,040,022,000,000,    000
// 555,036,027,407,507,    704,700,630,430,140,    040,053,044,000,000,    000
// 777,357,447,637,707,    737,740,750,660,360,    070,276,077,000,000,    000
// 777,567,657,757,747,    755,764,772,773,572,    473,276,467,000,000,    000

// 755,637,700,447,044,    120,222,704,777,333,    750,503,403,660,320,     777
// 357,653,310,360,467,    657,764,027,760,276,    000,200,666,444,707,     014
// 003,567,757,070,077,    022,053,507,000,420,    747,510,407,006,740,     000
// 000,140,555,031,572,    326,770,630,020,036,    040,111,773,737,430,     473

// first "16" entry is remapping of 772 to 764.
// unsigned char ppu_RP2C04_0001_mapping[] = 
// {
// $09,$1f,$2d,$35,$0c, $0b,$2b,$29,$0e,$05, $33,$3a,$25,$30,$30, $30,
// $32,$39,$17,$2c,$27, $07,$02,$37,$3e,$31, $3a,$26,$04,$30,$30, $30,
// $0f,$10,$03,$01,$0e, $3d,$2e,$0a,$0d,$13, $23,$19,$24,$30,$30, $30,
// $0f,$21,$15,$22,$2a, $00,$36,$16,$3c,$34, $3f,$19,$14,$30,$30, $30
// };

// // 0001
// unsigned char ppu_RP2C04_0001[] = 
// {
// 0x35,0x23,0x16,0x22,0x1c,0x09,0x2d,0x15,0x30,0x00,0x27,0x05,0x04,0x28,0x08,0x30,
// 0x21,0x18,0x06,0x29,0x3c,0x32,0x36,0x12,0x18,0x3b,0x0d,0x06,0x10,0x00,0x24,0x01,
// 0x01,0x31,0x33,0x2a,0x2c,0x0c,0x1b,0x14,0x0d,0x07,0x34,0x06,0x13,0x02,0x26,0x0d,
// 0x0d,0x19,0x10,0x0a,0x39,0x03,0x28,0x17,0x09,0x11,0x0b,0x10,0x38,0x25,0x18,0x3a,
// };


// const unsigned char palette_bg_[16] = 
// { 
//     0x30,   //     0x0f,
//     0x03,   //     0x22,
//     0x21,   //     0x31,
//     0x08,   //     0x30,

//     0x30,   //     0x0f,
//     0x09,   //     0x00,
//     0x37,   //     0x17,
//     0x0d,   //     0x28,

//     0x30,   //     0x0f,
//     0x23,   //     0x2a,
//     0x02,   //     0x16,
//     0x3c,   //     0x37, 

//     0x30,   //     0x0f,
//     0x03,   //     0x22,
//     0x2e,   //     0x26, // 0x00 - Should be 2e, but appears to be blue, so picked something close.
//     0x3c    //     0x37 
// };

// Default palettes for the title screen and settings.
const unsigned char palette_bg[16]={ 0x0f,0x22,0x31,0x30,0x0f,0x00,0x17,0x28,0x0f,0x2a,0x16,0x36,0x0f,0x22,0x26,0x36 };
const unsigned char palette_sp[16]={ 0x0f,0x22,0x31,0x30,0x0f,0x0f,0x26,0x36,0x0f,0x16,0x31,0x36,0x0f,0x0f,0x26,0x36 };
const unsigned char palette_bg_options[16]={ 0x0f,0x22,0x31,0x30,0x0f,0x30,0x0f,0x26,0x0f,0x22,0x0f,0x26,0x0f,0x22,0x26,0x36 };

#if VS_SYS_ENABLED
//const unsigned char palette_vs_options[16]={ 0x0f,0x00,0x10,0x30,0x0f,0x00,0x10,0x30,0x0f,0x00,0x10,0x20,0x0f,0x22,0x32,0x30 };
const unsigned char palette_vs_options[16]={ 0x0f,0x00,0x10,0x30,0x0f,0x00,0x10,0x30,0x0f,0x00,0x10,0x20,0x0f,0x22,0x31,0x30 };
const unsigned char palette_vs_highscore_table[16]={ 0x0f,0x22,0x31,0x30,0x0f,0x30,0x0f,0x26,0x0f,0x22,0x0f,0x26,0x0f,0x38,0x0f,0x26 };

const unsigned char palette_vs_options_inactive[] = { 0x0f,0x00,0x10,0x30 };
const unsigned char palette_vs_options_active[] = { 0x0f,0x22,0x26,0x36 };
const unsigned char palette_vs_options_skulls[16] = { 0x0f,0x06,0x16,0x36,0x0f,0x06,0x16,0x36,0x0f,0x06,0x16,0x36,0x0f,0x06,0x16,0x36, };
#endif 


// const unsigned char pal_changes[20] = 
// {
//     0x01, 0x21, // blues
//     0x13, 0x23, // purples
//     0x1c, 0x26, // dark blue + oj
//     0x0b, 0x1b, // dark green, soft green
//     0x06, 0x15, // reds
//     0x2c, 0x39, // limey blue
//     0x03, 0x35, // purples
//     0x16, 0x26, // oranges
//     0x11, 0x2b, // light green, blue
//     0x0f, 0x15, // bright reds
// };

// Every 10 levels the color palette changes. Thematically, this is supposed to be the
// time of day changing from Day, Evening, Night, Morning, and back to Day.
#define NUM_TIMES_OF_DAY 8
const unsigned char palette_bg_list[NUM_TIMES_OF_DAY][16]=
{
    { 0x0f,0x22,0x31,0x30,0x0f,0x00,0x17,0x28,0x0f,0x2a,0x16,0x36,0x0f,0x22,0x26,0x36 },
    { 0x0f,0x22,0x36,0x30,0x0f,0x00,0x17,0x27,0x0f,0x2a,0x16,0x38,0x0f,0x22,0x26,0x36 },
    { 0x0f,0x1c,0x36,0x30,0x0f,0x00,0x17,0x27,0x0f,0x1a,0x16,0x38,0x0f,0x1c,0x26,0x36 },
    { 0x0f,0x0c,0x23,0x34,0x0f,0x00,0x07,0x10,0x0f,0x1a,0x17,0x27,0x0f,0x0c,0x16,0x27 },
    { 0x0f,0x0c,0x1c,0x10,0x0f,0x00,0x07,0x22,0x0f,0x1b,0x07,0x17,0x0f,0x0c,0x11,0x22 },
    { 0x0f,0x0c,0x22,0x27,0x0f,0x00,0x18,0x27,0x0f,0x1a,0x17,0x27,0x0f,0x0c,0x22,0x36 },
    { 0x0f,0x1c,0x36,0x30,0x0f,0x00,0x18,0x26,0x0f,0x2a,0x16,0x38,0x0f,0x1c,0x26,0x36 },
    { 0x0f,0x23,0x32,0x30,0x0f,0x00,0x17,0x28,0x0f,0x2a,0x16,0x36,0x0f,0x23,0x26,0x36 },
};

/*
Level	Frames per Gridcell
00	        48
01	        43
02	        38
03	        33
04	        28
05	        23
06	        18
07	        13
08	        8
09	        6
10–12	    5
13–15	    4
16–18	    3
19–28	    2
29+	        1
*/

// How many frames pass before a block moves down a row (per level).
// https://tetris.fandom.com/wiki/Tetris_(NES,_Nintendo)
const unsigned char fall_rates_per_level[] =
{
    48,
    43,
    38,
    33,
    28,
    23,
    18,
    13,
    8,
    6,
    5, // 10-12
    5, // 10-12
    5, // 10-12
    4, // 13-15
    4, // 13-15
    4, // 13-15
    3, // 16–18
    3, // 16–18
    3, // 16–18
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    2, // 19–28
    1, // 29+
};

// Different game modes for the settings screen.
const unsigned char attack_style_strings[3][ATTACK_STRING_LEN] = 
{
    "FIXED",
    "TIMED",
    "CLASSIC"
};
const unsigned char off_on_string[2][OFF_ON_STRING_LEN] = 
{
    "OFF",
    "ON"
};
// Originally the game was going to feature different clusters, other than 
// tetrominos, but that ended up getting cut. This is legacy from that and could
// be stripped out.
const unsigned char block_style_strings[2][BLOCK_STYLE_STRING_LEN] =
{
    "MODERN",
    "CLASSIC"
};

// How many options for hard drop are there.
#define NUM_HARD_DROP_SETTINGS 3
// How long is the string used to display the harddrop setting name.
#define HARD_DROP_STRING_LEN 4
// The different types of hard drop.
const unsigned char hard_drop_types[NUM_HARD_DROP_SETTINGS][HARD_DROP_STRING_LEN] = { "OFF", "TAP", "HOLD" };

#if VS_SYS_ENABLED
unsigned char auto_forward_leaderboards;
unsigned char free_play_enabled;
unsigned char game_cost;
unsigned char option_state;
// Store the value at $4016 from previous frame, so that
// it can be compared for release events.
unsigned char prev_4016;
// Counter used to turn on the hardware coin counter for 3 frames
// then off for at least 3 frames.
unsigned char maintenance_counter;
#endif //VS_SYS_ENABLED

// To force the screen to shake, set this to a value > 0. That is how many
// frames the screen will skake for.
unsigned char screen_shake_remaining;

#if VS_SYS_ENABLED
const unsigned char text_insert_1_coin[] = { "\xDB  INSERT COIN  \xDC" }; // { "PUSH START" };
const unsigned char text_insert_2_coin[] = { "\xDB INSERT  COINS \xDC" }; // { "PUSH START" };
const unsigned char text_free_play[] = {     "\xdb  FREE   PLAY  \xDC" }; // { "PUSH START" };
const unsigned char text_push_start[] = {    "\xDBPUSH ANY BUTTON\xDC" }; // { "PUSH START" };
const unsigned char clear_push_start[] = {   "\xDB               \xDC" }; //{ "          " };
#else
// Text that appears on the title screen, and the empty string used to clear it 
// so that it appears to flash.
const unsigned char text_push_start[] =  { "PUSH START" };
const unsigned char clear_push_start[] = { "          " };
#endif //VS_SYS_ENABLED

#if VS_SYS_ENABLED
unsigned char credits_remaining;

const unsigned char metasprite_vs_logo[]={
	  0,  0,0x0a,3,
	  8,  0,0x0b,3,
	 16,  0,0x0c,3,
	 24,  0,0x0d,3,
	  0,  8,0x1a,3,
	  8,  8,0x1b,3,
	 16,  8,0x1c,3,
	 24,  8,0x1d,3,
	128
};

// const unsigned char metasprite_button2[]={
// 	  0,  0,0xab,3,
// 	  8,  0,0xac,3,
// 	  0,  8,0xbb,3,
// 	  8,  8,0xbc,3,
// 	128
// };

unsigned char attract_gameplay_enabled = 0;

#endif // VS_SYS_ENABLED

// On the title screen a sprite is used to draw the tentacle around the statue
// to avoid attribute clash. This is that sprite.
const unsigned char metasprite_tentacle_title[]={
	  0,  0,0x60,1,
	  8,  0,0x61,1,
	  0,  8,0x70,1,
	  8,  8,0x71,1,
	  0, 16,0x80,1,
	  8, 16,0x81,1,
	  0, 24,0x90,1,
	  8, 24,0x91,1,
	128
};

// PROTOTYPES

void draw_menu_sprites(void) { } // PLAT_GB
void draw_gameplay_sprites(void);
void movement(void);

// Set a block in x, y (board space)
void set_block(/*unsigned char x, unsigned char y, unsigned char id*/);
void set_block_nt(unsigned char x, unsigned char y, unsigned char id, unsigned char nt);
// x, y in board space.
void clear_block(unsigned char x, unsigned char y);

// Drops the current cluster at its current location.
void put_cur_cluster();

// x, y in map space.
unsigned char get_block(unsigned char x, unsigned char y);

// x, y in map space.
unsigned char is_block_free(unsigned char x, unsigned char y);

// Checks if the entire cluster is currently hitting any other blocks.
unsigned char is_cluster_colliding();

// creates a new cluster at the top of the play area.
void spawn_new_cluster();

// Rotate the current cluster by 90degs.
void rotate_cur_cluster(char dir);

// Transition to a new state.
void go_to_state(unsigned char new_state);

void inc_lines_cleared();
void display_lines_cleared();
void display_score();
void display_highscore() { } // PLAT_GB
void display_level() { } // PLAT_GB

// CLEAR PHASES

// First clear all the rows in CPU memory.
void clear_rows_in_data(unsigned char start_y);

// Then show the empty rows, 2 columns at a time...
void reveal_empty_rows_to_nt();

// Then collapse the empty rows in CPU memory...
void try_collapse_empty_row_data(void);

// Finally reveal to resolved board with all blocks in final position.
void copy_board_to_nt();

void add_block_at_bottom() { } // PLAT_GB
void add_row_at_bottom() { } // PLAT_GB

void reset_gameplay_area() { } // PLAT_GB

void display_song() { } // PLAT_GB
void display_sound() { } // PLAT_GB
void display_options() { } // PLAT_GB

void fade_to_black();
void fade_from_black();

void difficulty_to_leaderboard_pos(unsigned char dif);

// DEBUG
#if DEBUG_ENABLED
void debug_fill_nametables(void);
void debug_draw_board_area(void);
void debug_copy_board_data_to_nt(void);
void debug_display_number(unsigned char num, unsigned char index);
#endif