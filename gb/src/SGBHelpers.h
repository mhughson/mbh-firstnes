#ifndef D161F945_3DE5_43C6_BBE0_9786D5BA6B6C
#define D161F945_3DE5_43C6_BBE0_9786D5BA6B6C

#include <gb/sgb.h>

#define SGB_TRANSFER(A,B) map_buf[0]=(A),map_buf[1]=(B),sgb_transfer(map_buf) 
#define SGB_CHR_BLOCK0 0
#define SGB_CHR_BLOCK1 1

#define SGB_SCR_FREEZE 1
#define SGB_SCR_UNFREEZE 0

#define COL_LOW(col) ((UINT8)col)
#define COL_HIGH(col) ((UINT8)(col >> 8))

UINT8 map_buf[16];

UINT8 sound_a_pitch_table[] = {2,3,3,3,3,3,3,3,2,2,2,2,1,1,1,3,3,0,0,3,3,3,2,3,3,0,0,2,1,2,1,1,2,1,1,1,1,1,3,0,0,1,0,3,0,3,0,0,2};
UINT8 sound_b_pitch_table[] = {0,2,2,2,1,1,1,2,0,0,0,0,3,2,3,3,1,0,1,2,0,3,2,3,0,0};

#include "sgb_menu_pal_attr.h"
#include "sgb_settings_pal_attr.h"

void sgb_init_pals();

void sgb_play_sounda(UINT8 index)
{
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_SOUND << 3) | 1; // play a sound effect
    map_buf[1] = index; // sound effect A-1
//    map_buf[2] = 0x0C; // sound effect B-3
    map_buf[3] = sound_a_pitch_table[index]; // pitch 3 for A sound, pitch 2 for B sound. Both at max volume.

    sgb_transfer(map_buf);  
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE); 
}

void sgb_play_soundb(UINT8 index)
{
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_SOUND << 3) | 1; // play a sound effect
//    map_buf[1] = index; // sound effect A-1
    map_buf[2] = index; // sound effect B-3
    map_buf[3] = sound_b_pitch_table[index] << 4; // pitch 3 for A sound, pitch 2 for B sound. Both at max volume.

    sgb_transfer(map_buf);  
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE); 
}

void sgb_play_testsound()
{
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_SOUND << 3) | 1; // play a sound effect
//    map_buf[1] = 1; // sound effect A-1
    map_buf[2] = 0x0C; // sound effect B-3
    map_buf[3] = 3 | (3 << 4); // pitch 3 for A sound, pitch 2 for B sound. Both at max volume.

    sgb_transfer(map_buf);  
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE); 
}

void sgb_stop_testsound()
{
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_SOUND << 3) | 1; // play a sound effect
    map_buf[1] = 0x80; // sound effect A-1
    map_buf[2] = 0x80; // sound effect B-3
    //map_buf[3] = 3 | (3 << 2) | (2 << 4); // a pitch | a vol |b pitch | b vol
    //map_buf[4] = 2;

    sgb_transfer(map_buf);  
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE); 
}

void sgb_init_gameplay()
{
    sgb_init_pals();

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    // memset(map_buf, 0, sizeof(map_buf));

    // map_buf[0] = (SGB_ATTR_DIV << 3) | 3;
    // // 2 bits - pal below
    // // 2 bits - pal above
    // // 2 bits - pal on line
    // // 1 bit - 1 split up/down
    // map_buf[1] = 0	| (1 << 2) | (1 << 4) | (1 << 6);
    // map_buf[2] = 8;	

    // sgb_transfer(map_buf);

    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_ATTR_BLK << 3) | 1;
    map_buf[1] = 1;
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[2] = 1; // inside
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[3] = 1; // inside color
    // x1 left
    map_buf[4] = 0;
    // y1 top
    map_buf[5] = 0;
    // x2 right
    map_buf[6] = 20;
    // y2 bottom
    map_buf[7] = 18;  
    sgb_transfer(map_buf);  

    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_ATTR_LIN << 3) | 3;
    map_buf[1] = 3;
    // 5 bits - line number
    // 2 bits - pal
    // 1 bit - 1 horz
    map_buf[2] = (7) | (2 << 5) | (0 << 7);
    map_buf[3] = (8) | (2 << 5) | (0 << 7);
    map_buf[4] = (0x13) | (2 << 5) | (0 << 7);
    sgb_transfer(map_buf);


    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_ATTR_BLK << 3) | 1;
    map_buf[1] = 2;
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[2] = 1; // 1
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[3] = 3; // 3
    // x1 left
    map_buf[4] = 0;
    // y1 top
    map_buf[5] = 0x0c;
    // x2 right
    map_buf[6] = 2;
    // y2 bottom
    map_buf[7] = 0x0e;
    
    // ////
    
    // left lower tower
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[8] = 2; // border only
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[9] = 8; // border pal 2
    // x1 left
    map_buf[10] = 3;
    // y1 top
    map_buf[11] = 0x0a;
    // x2 right
    map_buf[12] = 4;
    // y2 bottom
    map_buf[13] = 0x0c;
    sgb_transfer(map_buf);    


    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_ATTR_BLK << 3) | 1;
    map_buf[1] = 2;

    // Kraken
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[2] = 1; // inside
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[3] = 0; // inside color
    // x1 left
    map_buf[4] = 1;
    // y1 top
    map_buf[5] = 0x11 - 2;
    // x2 right
    map_buf[6] = 5;
    // y2 bottom
    map_buf[7] = 0x13 - 2;

    // gameplay well
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[8] = 1; // inside
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[9] = 0; // inside color
    // x1 left
    map_buf[10] = 9;
    // y1 top
    map_buf[11] = 0;
    // x2 right
    map_buf[12] = 0x12;
    // y2 bottom
    map_buf[13] = 17;
    sgb_transfer(map_buf);



    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_ATTR_BLK << 3) | 1;
    map_buf[1] = 2;

    // left tower lower
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[2] = 1; // inside
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[3] = 2; // inside color
    // x1 left
    map_buf[4] = 3;
    // y1 top
    map_buf[5] = 0x0b;
    // x2 right
    map_buf[6] = 6;
    // y2 bottom
    map_buf[7] = 0x0e;

    // bottom left statue
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[8] = 2; // border only
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[9] = 8; // border pal 2
    // x1 left
    map_buf[10] = 6;
    // y1 top
    map_buf[11] = 0x0e;
    // x2 right
    map_buf[12] = 7;
    // y2 bottom
    map_buf[13] = 0x11;
    sgb_transfer(map_buf);    


    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_ATTR_BLK << 3) | 1;
    map_buf[1] = 2;

    // water below left statue
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[2] = 2; // border only
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[3] = 0; // border pal 0
    // x1 left
    map_buf[4] = 6;
    // y1 top
    map_buf[5] = 17;
    // x2 right
    map_buf[6] = 8;
    // y2 bottom
    map_buf[7] = 19;

    // next piece
    // control code
    // 0 - color inside
    // 1 - color border
    // 2 - color outside
    map_buf[8] = 1; // inside
    // colors
    // 0-1 - inside color palette
    // 2-3 - border color
    // 4-5 - outside color
    map_buf[9] = 0; // inside pal 0
    // x1 left
    map_buf[10] = 1;
    // y1 top
    map_buf[11] = 0;
    // x2 right
    map_buf[12] = 5;
    // y2 bottom
    map_buf[13] = 4;
    sgb_transfer(map_buf);       

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE); 
}

void sgb_init_menu()
{
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE); 


/// PALETTES

    UINT16 col;

    #define SHARED_MENU_BG_COL RGB2(31, 31, 31)
    #define SHARED_MENU_SKY_BG_COL RGB2(23, 28, 31)
    #define SHARED_MENU_WATER_BG_COL RGB2(16, 18, 31)
    #define SHARED_MENU_STONE_WALL_BG_COL RGB2(12, 12, 12)
    #define SHARED_MENU_STONE_HIGHLIGHT_BG_COL RGB2(20, 20, 23)
    #define SHARED_MENU_SHADOW_BG_COL RGB2(3, 4, 5)
    #define SHARED_MENU_KRAKEN_LIGHT_BG_COL RGB2(31, 16, 11)
    #define SHARED_MENU_KRAKEN_DARK_BG_COL RGB2(17, 9, 0)

    #define SHARED_MENU_UNUSED RGB2(31, 0, 31)

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    memset(map_buf, 0, sizeof(map_buf));

    map_buf[0] = (SGB_PAL_01 << 3) | 1;
 
    // Sky and Water
    //

    col = SHARED_MENU_BG_COL;
    map_buf[1] = COL_LOW(col);
    map_buf[2] = COL_HIGH(col);

    // pal 0 - col 1 - Sky / Light Water
    col = SHARED_MENU_SKY_BG_COL
    map_buf[3] = COL_LOW(col);
    map_buf[4] = COL_HIGH(col);

    // pal 0 - col 2 - Dark Water
    col = SHARED_MENU_WATER_BG_COL
    map_buf[5] = COL_LOW(col);
    map_buf[6] = COL_HIGH(col);

    // pal 0 - col 3 - Shadow
    col = SHARED_MENU_SHADOW_BG_COL
    map_buf[7] = COL_LOW(col);
    map_buf[8] = COL_HIGH(col);

    // Stone
    //

    // pal 1 - col 1
    col = SHARED_MENU_STONE_HIGHLIGHT_BG_COL;
    map_buf[9] = COL_LOW(col);
    map_buf[10] = COL_HIGH(col);
    // pal 1 - col 2
    col = SHARED_MENU_STONE_WALL_BG_COL;
    map_buf[11] = COL_LOW(col);
    map_buf[12] = COL_HIGH(col);
    // pal 1 - col 3 (blue) - black
    col = SHARED_MENU_SHADOW_BG_COL;
    map_buf[13] = COL_LOW(col);
    map_buf[14] = COL_HIGH(col);	

    sgb_transfer(map_buf);	


    memset(map_buf, 0, sizeof(map_buf));

    // Kraken
    //

    map_buf[0] = (SGB_PAL_23 << 3) | 1;
    
    // Background
    col = SHARED_MENU_BG_COL;
    map_buf[1] = COL_LOW(col);
    map_buf[2] = COL_HIGH(col);

    // pal 2 - col 1 
    col = SHARED_MENU_KRAKEN_LIGHT_BG_COL
    map_buf[3] = COL_LOW(col);
    map_buf[4] = COL_HIGH(col);
    // pal 2 - col 2 
    col = SHARED_MENU_KRAKEN_DARK_BG_COL
    map_buf[5] = COL_LOW(col);
    map_buf[6] = COL_HIGH(col);
    // pal 2 - col 3 
    col = SHARED_MENU_SHADOW_BG_COL
    map_buf[7] = COL_LOW(col);
    map_buf[8] = COL_HIGH(col);

    // ///

    // // pal 3 - col 1 - light green
    // col = RGB2(7, 23, 8);
    // map_buf[9] = COL_LOW(col);
    // map_buf[10] = COL_HIGH(col);
    // // pal 3 - col 2 - dark green
    // col = RGB2(4, 14, 6);
    // map_buf[11] = COL_LOW(col);
    // map_buf[12] = COL_HIGH(col);
    // // pal 3 - col 3 - black
    // col = RGB2(0, 0, 0);
    // map_buf[13] = COL_LOW(col);
    // map_buf[14] = COL_HIGH(col);	

    sgb_transfer(map_buf);

/// END PALETTES

//      Byte  Content
//  0     Command*8+Length (length=1..6)
//  1     Beginning X-Coordinate
//  2     Beginning Y-Coordinate
//  3-4   Number of Data Sets (1-360)
//  5     Writing Style   (0=Left to Right, 1=Top to Bottom)
//  6     Data Sets 1-4   (Set 1 in MSBs, Set 4 in LSBs)
//  7     Data Sets 5-8   (if any)
//  8     Data Sets 9-12  (if any)
//  etc.

    sgb_transfer(menu_pal_attr_0);
    sgb_transfer(menu_pal_attr_1);
    sgb_transfer(menu_pal_attr_2);

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE); 
}

void sgb_init_settings()
{
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE); 


/// PALETTES

    UINT16 col;

    //#define SHARED_SETTINGS_BG_COL RGB2(0, 0, 0)
    #define SHARED_SETTINGS_BG_COL RGB2(31, 27, 20)
    #define SHARED_SETTINGS_DARK_BLUE RGB2(3, 4, 6)
    #define SHARED_SETTINGS_LIGHT_GREY RGB2(23, 23, 23)
    #define SHARED_SETTINGS_DARK_GREY RGB2(12, 12, 12)
    #define SHARED_SETTINGS_BLACK RGB2(0,0,0)

    #define SHARED_SETTINGS_UNUSED RGB2(31, 0, 31)

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    memset(map_buf, 0, sizeof(map_buf));

    map_buf[0] = (SGB_PAL_01 << 3) | 1;
 
    // Sky and Water
    //

    col = SHARED_SETTINGS_BG_COL;
    map_buf[1] = COL_LOW(col);
    map_buf[2] = COL_HIGH(col);

    // pal 0 - col 1 - Sky / Light Water
    col = RGB2(6, 13, 10); //SHARED_SETTINGS_DARK_BLUE
    map_buf[3] = COL_LOW(col);
    map_buf[4] = COL_HIGH(col);

    // pal 0 - col 2 - Dark Green Plant
    col = SHARED_SETTINGS_DARK_BLUE// RGB2(6, 13, 10);
    map_buf[5] = COL_LOW(col);
    map_buf[6] = COL_HIGH(col);

    // pal 0 - col 3 - Light Green Plant
    col = SHARED_SETTINGS_BLACK //RGB2(16, 24, 13);
    map_buf[7] = COL_LOW(col);
    map_buf[8] = COL_HIGH(col);

    // Stone
    //

    // pal 1 - col 1
    col = SHARED_SETTINGS_DARK_GREY //SHARED_SETTINGS_DARK_BLUE;
    map_buf[9] = COL_LOW(col);
    map_buf[10] = COL_HIGH(col);
    // pal 1 - col 2 - Stone Grey
    col = SHARED_SETTINGS_DARK_BLUE //SHARED_SETTINGS_DARK_GREY
    map_buf[11] = COL_LOW(col);
    map_buf[12] = COL_HIGH(col);
    // pal 1 - col 3 - sand
    col = SHARED_SETTINGS_BLACK //RGB2(31, 27, 20);
    map_buf[13] = COL_LOW(col);
    map_buf[14] = COL_HIGH(col);	

    sgb_transfer(map_buf);	


    memset(map_buf, 0, sizeof(map_buf));

    // Kraken
    //

    map_buf[0] = (SGB_PAL_23 << 3) | 1;
    
    // Background
    col = SHARED_SETTINGS_BG_COL;
    map_buf[1] = COL_LOW(col);
    map_buf[2] = COL_HIGH(col);

    // pal 2 - col 1 
    col = SHARED_SETTINGS_DARK_GREY //SHARED_SETTINGS_DARK_BLUE
    map_buf[3] = COL_LOW(col);
    map_buf[4] = COL_HIGH(col);
    // pal 2 - col 2 
    col = SHARED_SETTINGS_DARK_BLUE //SHARED_SETTINGS_DARK_GREY
    map_buf[5] = COL_LOW(col);
    map_buf[6] = COL_HIGH(col);
    // pal 2 - col 3 
    col = SHARED_SETTINGS_BLACK //RGB2(23, 23, 23);
    map_buf[7] = COL_LOW(col);
    map_buf[8] = COL_HIGH(col);

    // ///

    // pal 3 - col 1 - menu highlight blue
    col = SHARED_SETTINGS_UNUSED //RGB2(16, 18, 31);
    map_buf[9] = COL_LOW(col);
    map_buf[10] = COL_HIGH(col);
    // pal 3 - col 2 - dark green
    col = RGB2(16, 18, 31); //SHARED_SETTINGS_UNUSED
    map_buf[11] = COL_LOW(col);
    map_buf[12] = COL_HIGH(col);
    // pal 3 - col 3 - white
    col = SHARED_SETTINGS_BLACK //RGB2(31, 31, 31);
    map_buf[13] = COL_LOW(col);
    map_buf[14] = COL_HIGH(col);	

    sgb_transfer(map_buf);

/// END PALETTES

//      Byte  Content
//  0     Command*8+Length (length=1..6)
//  1     Beginning X-Coordinate
//  2     Beginning Y-Coordinate
//  3-4   Number of Data Sets (1-360)
//  5     Writing Style   (0=Left to Right, 1=Top to Bottom)
//  6     Data Sets 1-4   (Set 1 in MSBs, Set 4 in LSBs)
//  7     Data Sets 5-8   (if any)
//  8     Data Sets 9-12  (if any)
//  etc.

    sgb_transfer(settings_pal_attr_0);
    sgb_transfer(settings_pal_attr_1);
    sgb_transfer(settings_pal_attr_2);

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE); 
}

void sgb_init_pals()
{
    UINT16 col;

    #define SHARED_BG_COL RGB2(23, 28, 31)
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE);

    memset(map_buf, 0, sizeof(map_buf));

    map_buf[0] = (SGB_PAL_01 << 3) | 1;
    // Background - light blue
    col = SHARED_BG_COL;
    map_buf[1] = COL_LOW(col);
    map_buf[2] = COL_HIGH(col);
    // pal 0 - col 1 (red) - orange
    col = RGB2(31, 14, 13);
    map_buf[3] = COL_LOW(col);
    map_buf[4] = COL_HIGH(col);
    // pal 0 - col 2 (green) - dark blue
    col = RGB2(14, 14, 25);
    map_buf[5] = COL_LOW(col);
    map_buf[6] = COL_HIGH(col);
    // pal 0 - col 3 (blue) - black
    col = RGB2(0, 0, 0);
    map_buf[7] = COL_LOW(col);
    map_buf[8] = COL_HIGH(col);

    ///

    // pal 1 - col 1 - light blue dupe
    col = SHARED_BG_COL;
    map_buf[9] = COL_LOW(col);
    map_buf[10] = COL_HIGH(col);
    // pal 1 - col 2 (green) - dark blue
    col = RGB2(14, 14, 25);
    map_buf[11] = COL_LOW(col);
    map_buf[12] = COL_HIGH(col);
    // pal 1 - col 3 (blue) - black
    col = RGB2(0, 0, 0);
    map_buf[13] = COL_LOW(col);
    map_buf[14] = COL_HIGH(col);	

    sgb_transfer(map_buf);	


    memset(map_buf, 0, sizeof(map_buf));

    map_buf[0] = (SGB_PAL_23 << 3) | 1;
    // Background - light blue
    col = SHARED_BG_COL;
    map_buf[1] = COL_LOW(col);
    map_buf[2] = COL_HIGH(col);
    // pal 2 - col 1 light grey
    col = RGB2(10, 10, 10);
    map_buf[3] = COL_LOW(col);
    map_buf[4] = COL_HIGH(col);
    // pal 2 - col 2 - dark grey
    col = RGB2(15, 15, 15);
    map_buf[5] = COL_LOW(col);
    map_buf[6] = COL_HIGH(col);
    // pal 2 - col 3 - black
    col = RGB2(0, 0, 0);
    map_buf[7] = COL_LOW(col);
    map_buf[8] = COL_HIGH(col);

    ///

    // pal 3 - col 1 - light green
    col = RGB2(7, 23, 8);
    map_buf[9] = COL_LOW(col);
    map_buf[10] = COL_HIGH(col);
    // pal 3 - col 2 - dark green
    col = RGB2(4, 14, 6);
    map_buf[11] = COL_LOW(col);
    map_buf[12] = COL_HIGH(col);
    // pal 3 - col 3 - black
    col = RGB2(0, 0, 0);
    map_buf[13] = COL_LOW(col);
    map_buf[14] = COL_HIGH(col);	

    sgb_transfer(map_buf);	
        
    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE);
}

// Sound test for SGB sounds.
void sgb_sound_test()
{
#if 0
    const char* sound_a_names[] = {
    "DUMMY FLAG, RE-TRIGGER",
    "NINTENDO",
    "GAME OVER",
    "DROP",
    "OK ... A",
    "OK ... B",
    "SELECT...A",
    "SELECT...B",
    "SELECT...C",
    "MISTAKE...BUZZER",
    "CATCH ITEM",
    "GATE SQUEAKS 1 TIME",
    "EXPLOSION...SMALL",
    "EXPLOSION...MEDIUM",
    "EXPLOSION...LARGE",
    "ATTACKED...A",
    "ATTACKED...B",
    "HIT (PUNCH)...A",
    "HIT (PUNCH)...B",
    "BREATH IN AIR",
    "ROCKET PROJECTILE...A",
    "ROCKET PROJECTILE...B",
    "ESCAPING BUBBLE",
    "JUMP",
    "FAST JUMP",
    "JET (ROCKET) TAKEOFF",
    "JET (ROCKET) LANDING",
    "CUP BREAKING",
    "GLASS BREAKING",
    "LEVEL UP",
    "INSERT AIR",
    "SWORD SWING",
    "WATER FALLING",
    "FIRE",
    "WALL COLLAPSING",
    "CANCEL",
    "WALKING",
    "BLOCKING STRIKE",
    "PICTURE FLOATS ON & OFF",
    "FADE IN",
    "FADE OUT",
    "WINDOW BEING OPENED",
    "WINDOW BEING CLOSED",
    "BIG LASER",
    "STONE GATE CLOSES/OPENS",
    "TELEPORTATION",
    "LIGHTNING",
    "EARTHQUAKE",
    "SMALL LASER",
    "EFFECT A, STOP/SILENT",
    };
    const char* sound_b_names[] = {
        "DUMMY FLAG, RE-TRIGGER",
        "APPLAUSE...SMALL GROUPS",
        "APPLAUSE...MEDIUM GROUPS",
        "APPLAUSE...LARGE GROUPS",
        "WINDS",
        "RAINS",
        "STORMS",
        "STORM WITH WIND/THUNDERS",
        "LIGHTNINGS",
        "EARTHQUAKES",
        "AVALANCHES",
        "WAVES",
        "RIVERS",
        "WATERFALLS",
        "SMALL CHARACTER RUNNINGS",
        "HORSE RUNNINGS",
        "WARNING SOUNDS",
        "APPROACHING CARS",
        "JET FLYINGS",
        "UFO FLYINGS",
        "ELECTROMAGNETIC WAVESS",
        "SCORE UPS",
        "FIRES",
        "CAMERA SHUTTER, FORMANTOS",
        "WRITE, FORMANTOS",
        "SHOW UP TITLE, FORMANTOS",
    };

    if (pad_all_new & PAD_LEFT)
    {
        if (scroll_y_camera > 0)
        {
            --scroll_y_camera;
        }
    }
    else if (pad_all_new & PAD_RIGHT)
    {
        if (scroll_y_camera < (scroll_x_camera == 0 ? 0x30 : 0x19))
        {
            ++scroll_y_camera;
        }
    }

    if (pad_all_new & PAD_UP)
    {
        if (scroll_x_camera > 0)
        {
            --scroll_x_camera;
        }
    }
    else if (pad_all_new & PAD_DOWN)
    {
        if (scroll_x_camera < 1)
        {
            ++scroll_x_camera;
            scroll_y_camera = MIN(scroll_y_camera, 0x19);
        }
    }

    PRINT_POS(0,1);
    Printf("TABLE %s ID: %d", scroll_x_camera == 0 ? "A" : "B", scroll_y_camera);

    PRINT(0,3,"                    ");
    PRINT(0,3, scroll_x_camera == 0 ? sound_a_names[scroll_y_camera] : sound_b_names[scroll_y_camera]);

    if (pad_all_new & PAD_A)
    {
        if (scroll_x_camera == 0)
        {
            sgb_play_sounda(scroll_y_camera);
        }
        else
        {
            sgb_play_soundb(scroll_y_camera);
        }
    }
    else if (pad_all_new & PAD_B)
    {
        sgb_stop_testsound();
    }
#endif // #if 0
}

#endif /* D161F945_3DE5_43C6_BBE0_9786D5BA6B6C */
