/** (C) Matt Hughson 2020 */

#pragma bss-name(push, "ZEROPAGE")

// GLOBAL VARIABLES

unsigned char pad1;
unsigned char pad1_new;
const unsigned char text[] = "- PRESS START -";

#pragma bss-name(push, "BSS")

const unsigned char palette_bg[]={
0x0f, 0x00, 0x10, 0x30, // black, gray, lt gray, white
0x0f, 0x07, 0x17, 0x27, // oranges
0x0f, 0x02, 0x12, 0x22, // blues
0x0f, 0x09, 0x19, 0x29, // greens
}; 

const unsigned char palette_sp[]={
0x0f, 0x07, 0x28, 0x38, // dk brown, yellow, white yellow
0,0,0,0,
0,0,0,0,
0,0,0,0
}; 

// PROTOTYPES
void draw_sprites(void);
void movement(void);
