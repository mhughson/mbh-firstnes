#include "Banks/SetAutoBank.h"

#include "ZGBMain.h"
#include "Scroll.h"
#include "SpriteManager.h"
#include "Print.h"
#include "Sprite.h"
#include "SGB.h"

//IMPORT_MAP(map);
IMPORT_TILES(font);

UINT16 frames_passed;

Sprite* BlockSprites_[8];

IMPORT_MAP(gb_border);
IMPORT_MAP(boot_screen);

const unsigned char flag_anim_[] = {4, 0, 1, 2, 3 };

void START() {
	UINT8 i;

	LOAD_SGB_BORDER(gb_border);

	// scroll_target = SpriteManagerAdd(SpritePlayer, 50, 50);
	// InitScroll(BANK(map), &map, 0, 0);
	frames_passed=0;
	INIT_FONT(font, PRINT_BKG);

	for (i=0; i < 8; ++i)
	{
		BlockSprites_[i] = SpriteManagerAdd(SpriteBlock, 0xFFF, 0xFFFF);
		BlockSprites_[i]->lim_x = 0xfff; // don't despawn
		BlockSprites_[i]->lim_y = 0xfff; // don't despawn
		SetSpriteAnim(BlockSprites_[i], flag_anim_, 15);
	}

	InitScroll(BANK(boot_screen), &boot_screen, 0, 0);
}

void UPDATE() {

	static char num[16];
	UIntToString(frames_passed, num);
	PRINT(0,0,num);
	++frames_passed;
}
