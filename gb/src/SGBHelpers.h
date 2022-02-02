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

void sgb_init_menu()
{
    UINT16 col;

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_FREEZE); 
    
    memset(map_buf, 0, sizeof(map_buf));

    map_buf[0] = (SGB_PAL_01 << 3) | 1;
    // Background - light blue
    col = RGB2(23, 28, 31);
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
    col = RGB2(23, 28, 31);
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
    col = RGB2(23, 28, 31);
    map_buf[1] = COL_LOW(col);
    map_buf[2] = COL_HIGH(col);
    // pal 0 - col 1 light blue dupe
    col = RGB2(10, 10, 10);
    map_buf[3] = COL_LOW(col);
    map_buf[4] = COL_HIGH(col);
    // pal 0 - col 2 - dark grey
    col = RGB2(15, 15, 15);
    map_buf[5] = COL_LOW(col);
    map_buf[6] = COL_HIGH(col);
    // pal 0 - col 3 - black
    col = RGB2(0, 0, 0);
    map_buf[7] = COL_LOW(col);
    map_buf[8] = COL_HIGH(col);

    ///

    // pal 1 - col 1 - light blue dupe
    col = RGB2(23, 28, 31);
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

    // 14 cloud
    // 15 top of statues

    memset(map_buf, 0, sizeof(map_buf));

    // map_buf[0] = (SGB_ATTR_LIN << 3) | 3;
    // map_buf[1] = 4;
    // // 5 bits - line number
    // // 2 bits - pal
    // // 1 bit - 1 horz
    // map_buf[2] = (12) | (1 << 5) | (1 << 7);
    // map_buf[3] = (13) | (1 << 5) | (1 << 7);
    // map_buf[4] = (14) | (1 << 5) | (1 << 7);
    // map_buf[5] = (15) | (1 << 5) | (1 << 7);

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
    map_buf[2] = (17) | (2 << 5) | (1 << 7);

    sgb_transfer(map_buf);				

    // // pal 0 - col 1 (red)
    // map_buf[3] = 0x1f; // 0x1f | (0x1f << 5) | (0x1f << 5)
    // map_buf[4] = 0x0;
    // // pal 0 - col 2 (green)
    // map_buf[5] = 0xe0;
    // map_buf[6] = 0x03;
    // // pal 0 - col 3 (blue)
    // map_buf[7] = 0x0;
    // map_buf[8] = 0x7c;

    SGB_TRANSFER((SGB_MASK_EN << 3) | 1, SGB_SCR_UNFREEZE); 
}

#endif /* D161F945_3DE5_43C6_BBE0_9786D5BA6B6C */
