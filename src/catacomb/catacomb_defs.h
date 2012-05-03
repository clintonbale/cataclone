#ifndef _CATACOMB_TILES_H_
#define _CATACOMB_TILES_H_

//
// Menu tiles.
//
#define T_MNU_POTION        22
#define T_MNU_SCROLL        23
#define T_MNU_KEY           24
#define T_MNU_BLOB          10
#define T_MNU_WHITE         25
#define T_MNU_PROG_L        194
#define T_MNU_PROG_M        195
#define T_MNU_PROG_R        196
#define T_MNU_BORDER_TL     14
#define T_MNU_BORDER_T      15
#define T_MNU_BORDER_TR     16
#define T_MNU_BORDER_L      17
#define T_MNU_BORDER_R      18
#define T_MNU_BORDER_BL     19
#define T_MNU_BORDER_B      20
#define T_MNU_BORDER_BR     21

//
// Level tiles.
//
#define T_BLACK             0
#define T_FLOOR             128
#define T_PINK              129
#define T_POTION            162
#define T_SCROLL            163
#define T_KEY               164
#define T_TREASURE          167
#define T_DOOR_V            165
#define T_DOOR_H            166
#define T_HIDDEN_START      136
#define T_HIDDEN_END        146
#define T_HIDDEN_POTION     143
#define T_HIDDEN_BOLT       144
#define T_HIDDEN_KEY        145
#define T_A                 230
#define T_B                 231
#define T_C                 232
#define T_D                 233
#define T_E                 234
#define T_F                 235
#define T_G                 236
#define T_H                 237
#define T_I                 238
#define T_J                 239
#define T_K                 240
#define T_SPAWN             T_A
#define T_TELE              T_B
#define T_MON_REDIMP        T_C
#define T_MON_WHITEIMP      T_D
#define T_MON_BIGREDIMP     T_E
#define T_MON_BIGPURPIMP    T_F
#define T_MON_LASTBOSS      T_G

//
// Level tile macros.
//
#define T_ISDOOR(x)     ((x)==T_DOOR_V||(x)==T_DOOR_H)
#define T_ISWALL(x)     ((x)!=T_FLOOR&&(x)<153)
#define T_ISHIDDEN(x)   (((x)>=T_HIDDEN_START)&&((x)<=T_HIDDEN_END))

//
// Animations
//
#define TELE_ANIM_NUM   4
#define TELE_ANIM_SIZE  (TILE_WIDTH*TELE_ANIM_NUM)

//
// Texture lumps
// Offsets to textures in the data.
//
#define MAINTILES_LUMP_START    0
#define MAINTILES_LUMP_END      256

#define PLAYER_LUMP_START       256
#define PLAYER_LUMP_END         320

#define REDIMP_LUMP_START       320
#define REDIMP_LUMP_END         384

#define WHITEIMP_LUMP_START     384
#define WHITEIMP_LUMP_END       448

#define BOLT_LUMP_START         448
#define BOLT_LUMP_END           480

#define MONSTERDEAD_LUMP_START  480
#define MONSTERDEAD_LUMP_END    492

#define TELE_LUMP_START         492
#define TELE_LUMP_END           512

#define BIGREDIMP_LUMP_START    512
#define BIGREDIMP_LUMP_END      656

#define BIGREDIMPATK_LUMP_START 656
#define BIGREDIMPATK_LUMP_END   684

#define BIGPURPIMP_LUMP_START   684
#define BIGPURPIMP_LUMP_END     939

#define BIGPURPIMPATK_LUMP_START 939
#define BIGPURPIMPATK_LUMP_END   987

#define LASTBOSS_LUMP_START     987
#define LASTBOSS_LUMP_END       1387

#define LASTBOSSATK_LUMP_START  1387
#define LASTBOSSATK_LUMP_END    1462

#endif // _CATACOMB_TILES_H_
