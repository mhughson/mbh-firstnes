/** (C) Matt Hughson 2020 */

#define DEBUG_ENABLED 1

// Nametable A: 	2400-2000 = 400
// Attributes: 		2400-23c0 = 0x40
// Patterns: 		0x400-0x40 = 0x3c0
#define NAMETABLE_SIZE 0x400
#define NAMETABLE_PATTERN_SIZE 0x3c0

#define BOARD_START_X_PX 96
#define BOARD_START_Y_PX 0
#define BOARD_END_X_PX 168
#define BOARD_END_Y_PX 184

#define BOARD_OOB_END 3
#define BOARD_END_X_PX_BOARD 9 // left edge of last block (width = 10)
#define BOARD_END_Y_PX_BOARD 23 // top edge of last block (height = 24)

#define BOARD_SIZE 240 // 
#define BOARD_HEIGHT (BOARD_END_Y_PX_BOARD - BOARD_OOB_END)

// TODO: Rename. This is board x,y to board index.
#define TILE_TO_BOARD_INDEX(x,y) (((y) * 10) + (x))

#pragma bss-name(push, "ZEROPAGE")

// GLOBAL VARIABLES

struct block
{
    unsigned char x;
    unsigned char y;
};

struct cluster
{
    /*
        1 1 0 0 ─┬─ [ byte 0 ]
        0 1 1 0 ─┘
        0 0 0 0
        0 0 0 0
    */
    unsigned short layout;
    const unsigned short* def;
    unsigned char sprite;
    unsigned char id;
};

unsigned char tick_count;
unsigned char pad1;
unsigned char pad1_new;
const unsigned char text[] = "- PRESS START -";

enum { STATE_MENU, STATE_GAME, STATE_OVER, STATE_SOUND_TEST };
unsigned char state = STATE_MENU;

// The block operates in "logical space" from 0 -> w/h. The logical
// space is converted to screen space at time of render (or ppu get).
struct block cur_block = { 0, 0 };


// How many frames need to pass before it falls 8 pixels.
unsigned char fall_rate = 48;
unsigned char cur_level = 0;

// Each entry in the array is a rotation.
// Stored as 4x4 16 bit matrix to support line (otherwise 3x3 would do it).
// TODO: Perhaps a special character could be user to terminate the array
//       prior to the end.
/*
    1 1 0 0
    0 1 1 0
    0 0 0 0
    0 0 0 0

    0 0 1 0
    0 1 1 0
    0 1 0 0
    0 0 0 0
*/
const unsigned short def_z_clust[4] = 
{ 
    0xc60,
    0x264,
    0xc60, // dupe.
    0x264, // dupe.
};


const unsigned short def_z_rev_clust[4] = 
{ 
    0x6C0,
    0x8C40,
    0x6C0, // dupe.
    0x8C40, // dupe.
};

const unsigned short def_line_clust[4] =
{
    0xf0,
    0x4444,
    0xf0, // dupe.
    0x4444 // dupe.
};

const unsigned short def_box_clust[4] =
{
    0x660,    
    0x660,    
    0x660,
    0x660,
};

const unsigned short def_tee_clust[4] =
{
    0x4e00,    
    0x4640,    
    0xe40,
    0x4c40,
};

const unsigned short def_L_clust[4] =
{
    0xe80,    
    0xc440,    
    0x2e00,
    0x4460,
};

const unsigned short def_L_rev_clust[4] =
{
    0xe20,    
    0x44c0,    
    0x8e00,
    0x6440,
};

#define NUM_CLUSTERS 7
const unsigned short* cluster_defs [NUM_CLUSTERS] =
{
    def_z_clust,
    def_z_rev_clust,
    def_line_clust,
    def_box_clust,
    def_tee_clust,
    def_L_clust,
    def_L_rev_clust,
};

unsigned char cur_rot;

struct cluster cur_cluster;// = { def_z_clust }; // 165 1010 0101
struct cluster next_cluster;

unsigned char cluster_sprites[NUM_CLUSTERS] =
{
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16
};

unsigned char cluster_offsets[NUM_CLUSTERS] = 
{
    3, 3, 2, 3, 4, 3, 3,
    //10,10,10,10,10,10,10
};

unsigned char horz_button_delay;
const unsigned char button_delay = 5;
unsigned char require_new_down_button;
unsigned char fall_frame_counter;
unsigned char lines_cleared_one;
unsigned char lines_cleared_ten;
unsigned char lines_cleared_hundred;
unsigned char cur_nt;
unsigned char off_nt;

// movement()
char hit;
unsigned char temp_fall_rate;
unsigned char old_x;
// spawn_new_cluster()
unsigned char id;
// put_cur_cluster()
unsigned char min_y;
unsigned char max_y;

unsigned char test_song;
unsigned char test_song_active;
unsigned char test_sound;

enum { MUSIC_TITLE, MUSIC_GAMEPLAY, MUSIC_PAUSE, MUSIC_GAMEOVER };
enum { SOUND_ROTATE, SOUND_LAND, SOUND_ROW, SOUND_MULTIROW, SOUND_GAMEOVER, SOUND_START, SOUND_BLOCKED, SOUND_LEVELUP };

#pragma bss-name(push, "BSS")

unsigned char game_board[BOARD_SIZE];
unsigned char game_board_temp[BOARD_SIZE];
char empty_row[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char full_row[10] =  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
unsigned char full_col[20] =  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

// copy_board_to_nt()
char copy_board_data[BOARD_HEIGHT];
char lines_cleared_y[4];

// const unsigned char palette_bg[]={
// //1x0c, 0x14, 0x23, 0x37,
// 0x0f, 0x00, 0x10, 0x30, // black, gray, lt gray, white
// 0x0f, 0x07, 0x17, 0x27, // oranges
// 0x0f, 0x02, 0x12, 0x22, // blues
// 0x0f, 0x09, 0x19, 0x29, // greens
// }; 

const unsigned char palette_bg[16]=
{ 
    0x3C,0x01,0x21,0x30, // bricks
    0x3C,0x22,0x01,0x30, // water
    0x3C,0x0f,0x1d,0x22, // rocks
    0x3C,0x0f,0x26,0x29  // platforms
};

const unsigned char pal_changes[20] = 
{
    0x01, 0x21, // blues
    0x13, 0x23, // purples
    0x1c, 0x26, // dark blue + oj
    0x0b, 0x1b, // dark green, soft green
    0x06, 0x15, // reds
    0x2c, 0x39, // limey blue
    0x03, 0x35, // purples
    0x16, 0x26, // oranges
    0x11, 0x2b, // light green, blue
    0x0f, 0x15, // bright reds
};

const unsigned char palette_sp[]={
0x0f, 0x00, 0x10, 0x30, // grey bricks
//0x3C,0x01,0x21,0x30, // bricks
0x0f, 0x09, 0x19, 0x29, // greens
0x0f, 0x07, 0x28, 0x38, // dk brown, yellow, white yellow
0,0,0,0
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

// https://tetris.fandom.com/wiki/Tetris_(NES,_Nintendo)
unsigned char fall_rates_per_level[] =
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

// PROTOTYPES
void draw_sprites(void);
void movement(void);

// Set a block in x, y (board space)
void set_block(unsigned char x, unsigned char y, unsigned char id);
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
void display_level();

// CLEAR PHASES

// First clear all the rows in CPU memory.
void clear_rows_in_data(unsigned char start_y);

// Then show the empty rows, 2 columns at a time...
void reveal_empty_rows_to_nt();

// Then collapse the empty rows in CPU memory...
void try_collapse_empty_row_data(void);

// Finally reveal to resolved board with all blocks in final position.
void copy_board_to_nt();

void display_song();
void display_sound();


// DEBUG
#if DEBUG_ENABLED
void debug_fill_nametables(void);
void debug_draw_board_area(void);
void debug_copy_board_data_to_nt(void);
void debug_display_number(unsigned char num, unsigned char index);
#endif