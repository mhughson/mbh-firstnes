//savegame.h
#ifndef SAVEGAME_H
#define SAVEGAME_H

#include <gb/gb.h>
#include "SRAM.h"

typedef struct {
	SAVEGAME_HEADER;

  //Whatever content you want to store in external ram
  unsigned char version;

  // High scores for the 3 game modes. 32 bit numbers to allow for scores
  // in the millions.
  unsigned long high_scores[3]; // NOTE: long!  

  // The current game mode. See ATTACK_ON_TIME and other at the top of StateFromBelow.h.
  unsigned char attack_style;

  // Is the music and sound currently enabled?
  unsigned char music_on;
  unsigned char sfx_on;
  
  // This stores an enum for the 3 types of hard drop (off, hold, and tap).
  unsigned char hard_drops_on;

} Savegame;

extern Savegame savegame;

#endif