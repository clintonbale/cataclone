#ifndef _CATACOMB_TILES_H_
#define _CATACOMB_TILES_H_

//menu stuff
#define TILE_MENU_POT       22
#define TILE_MENU_SCROLL    23
#define TILE_MENU_KEY       24

#define TILE_MENU_WHITE     25
#define TILE_MENU_BLACK     0

#define TILE_MENU_PROGRESSL 194
#define TILE_MENU_PROGRESS  195
#define TILE_MENU_PROGRESSR 196

#define TILE_MENU_TOPL      14
#define TILE_MENU_TOP       15
#define TILE_MENU_TOPR      16

#define TILE_MENU_LEFT      17
#define TILE_MENU_RIGHT     18

#define TILE_MENU_BOTTOML   19
#define TILE_MENU_BOTTOM    20
#define TILE_MENU_BOTTOMR   21
//end menu stuff

//items
#define TILE_ITEM_POTION    162
#define TILE_ITEM_SCROLL    163
#define TILE_ITEM_KEY       164
#define TILE_ITEM_TREASURE  167
//end items

#define TILE_TYPE_A         230
#define TILE_TYPE_SPAWN     230

#define TILE_TYPE_B         231
#define TILE_TYPE_TELE      231

#define TILE_TYPE_C         232
#define TILE_TYPE_REDIMP    232

#define TILE_TYPE_D         233
#define TILE_TYPE_WHITEIMP  233

#define TILE_TYPE_E         234
#define TILE_TYPE_BIGREDIMP 234

#define TILE_TYPE_F          235
#define TILE_TYPE_BIGPURPIMP 235

#define TILE_TYPE_G         236
#define TILE_TYPE_LASTBOSS  236

#define TILE_TYPE_H     237
#define TILE_TYPE_I     238
#define TILE_TYPE_J     239
#define TILE_TYPE_K     240

#define TILE_TYPE_FLOOR 128

#define TILE_TYPE_V_DOOR  165
#define TILE_TYPE_H_DOOR  166
#define ISDOOR(x) ((x)==TILE_TYPE_V_DOOR||(x)==TILE_TYPE_H_DOOR)


#define TILE_HIDDEN_POTION 143
#define TILE_HIDDEN_BOLT   144
#define TILE_HIDDEN_KEY    145

#define TILE_FIRST_HIDDEN  136
#define TILE_LAST_HIDDEN   146
#define IS_TILE_HIDDEN(x) (((x)>=TILE_FIRST_HIDDEN)&&((x)<=TILE_LAST_HIDDEN))

#define TELE_MAX_ANIMATIONS 4
#define TELE_ANIMATION_SIZE 32

#define TILE_PINK_BACKGROUND 129

#endif // _CATACOMB_TILES_H_
