#ifndef ZGBMAIN_H
#define ZGBMAIN_H

#define STATES \
_STATE(StateGame)\
_STATE(StateFromBelow)\
STATE_DEF_END

#define SPRITES \
_SPRITE_DMG(SpritePlayer, player)\
_SPRITE_DMG(SpriteFlag, flag) \
_SPRITE_DMG(SpriteBlock, block) \
SPRITE_DEF_END

#include "ZGBMain_Init.h"

#endif