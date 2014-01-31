#ifndef DEF_H
#define DEF_H

#include <SDL/SDL.h>

#define TILESET_PATH "Tileset.bmp"
#define NUMBER_TILES 14
#define HEIGHT_CELL 45
#define WIDTH_CELL HEIGHT_CELL
/*
    Settings Rows : (7; 4)
    Settings Columns : (8; 7)
    Settings Bombs : (7; 11)
*/

enum {EMPTY = 0, ONE = 1, TWO = 2, THREE = 3, FOUR = 4, FIVE = 5, SIX = 6, SEVEN = 7, EIGHT = 8, NINE = 9, ZERO = 10,
        FLAG = 11, BOMB = 12, UNEXPLORED = 13};

typedef struct
{
    SDL_Surface *image;
    SDL_Rect pos[NUMBER_TILES];
}
tileset_t;

typedef struct
{
    #define MINE 12
    unsigned short value;
    SDL_bool visited,
             flagged;
}
cell_t;

typedef struct
{
    unsigned short rows, columns,
                   nbombs;
    cell_t *board;
    SDL_Surface *screen;
    tileset_t tileset;
}
context_t;

void InitContext(context_t *ctxt);
void FreeContext(context_t *ctxt);

void UpdateValue(context_t *ctxt, int x, int y);
void SetMine(context_t *ctxt, int x, int y);
void GenBoard(context_t *ctxt);

int CountFlagsAround(context_t *ctxt, int x, int y);

SDL_bool isover(context_t *ctxt);
void Play(context_t *ctxt);
#define PLAY 1
#define SETTINGS 2
#define EXIT 3
int Menu(context_t *ctxt);

void BlitNumber(context_t *ctxt, unsigned short number, int x, int y);
void Settings(context_t *ctxt);

void BlitAll(context_t *ctxt, SDL_bool showbombs);

#endif
