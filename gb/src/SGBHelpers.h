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

unsigned char map_buf[20];

void sgb_int_gameplay()
{
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
    
    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_ATTR_DIV << 3) | 3;
    // 2 bits - pal below
    // 2 bits - pal above
    // 2 bits - pal on line
    // 1 bit - 1 split up/down
    map_buf[1] = 1	| (0 << 2) | (1 << 4) | (1 << 6);
    map_buf[2] = 12;	
    sgb_transfer(map_buf);

    memset(map_buf, 0, sizeof(map_buf));
    map_buf[0] = (SGB_ATTR_LIN << 3) | 3;
    map_buf[1] = 1;
    // 5 bits - line number
    // 2 bits - pal
    // 1 bit - 1 horz
    map_buf[2] = (17U) | (2U << 5) | (1U << 7);
    sgb_transfer(map_buf);	

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

#endif /* D161F945_3DE5_43C6_BBE0_9786D5BA6B6C */
