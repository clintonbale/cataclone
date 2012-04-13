#pragma once
#ifndef _CATACOMB_DATA_H_
#define _CATACOMB_DATA_H_

#include "../common.h"

#define EGA_DATA_LENGTH 46784
#define EGA_TILE_SIZE   32
#define NUM_EGA_TILES   (EGA_DATA_LENGTH/EGA_TILE_SIZE)

#define CGA_DATA_LENGTH 23392
#define CGA_TILE_SIZE   16
#define NUM_CGA_TILES   (CGA_DATA_LENGTH/CGA_TILE_SIZE)

static const byte END_PIC[16384] = {};
static const byte TITLE_PIC[16384] = {};

static const byte CGA_DATA[CGA_DATA_LENGTH] = {};
static const byte EGA_DATA[EGA_DATA_LENGTH] = {};

#endif // _CATACOMB_DATA_H_
