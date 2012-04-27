#ifndef _CATACOMB_SCORES_H_
#define _CATACOMB_SCORES_H_

#define MAX_HIGHSCORES 5
#define HS_STRUCT_SIZE 9

typedef struct {
    int     score;
    short   level;
    char    name[3];
} highscore_t;

const highscore_t* catacomb_scores_get(void);

void catacomb_scores_load(void);
void catacomb_scores_save(void);
void catacomb_scores_add(int score, short level, char name[3]);

#endif //_CATACOMB_SCORES_H_
