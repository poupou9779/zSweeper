#include "def.h"
#include <time.h>

const int   moveX[8] = {-1, +0, +1, -1, +1, -1, +0, +1},
            moveY[8] = {-1, -1, -1, +0, +0, +1, +1, +1};

void InitContext(context_t *ctxt)
{
    int i, j;
    srand((unsigned int)time(NULL));
    ctxt->rows = 14;
    ctxt->columns = 21;
    ctxt->nbombs = 35;
    ctxt->board = NULL;
    ctxt->screen = SDL_SetVideoMode(WIDTH_CELL * ctxt->columns, HEIGHT_CELL * ctxt->rows, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    if(ctxt->screen == NULL)
    {
        fprintf(stderr, "Unable to load screen in InitContext : '%s'\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    ctxt->tileset.image = SDL_LoadBMP(TILESET_PATH);
    if(ctxt->tileset.image == NULL)
    {
        fprintf(stderr, "Unable to load tileset in InitContext : '%s'\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    for(i = 0; i < NUMBER_TILES; ++i)
    {
        ctxt->tileset.pos[i].h = HEIGHT_CELL;
        ctxt->tileset.pos[i].w = WIDTH_CELL;
        ctxt->tileset.pos[i].x = i*WIDTH_CELL;
        ctxt->tileset.pos[i].y = 0;
    }
}

void FreeContext(context_t *ctxt)
{
    free(ctxt->board);
    ctxt->board = NULL;
    SDL_FreeSurface(ctxt->tileset.image);
    ctxt->tileset.image = NULL;
}

void UpdateValue(context_t *ctxt, int x, int y)
{
    if(x >= 0 && x < ctxt->columns && y >= 0 && y < ctxt->rows && ctxt->board[y*ctxt->columns + x].value != MINE)
        ++ctxt->board[y*ctxt->columns + x].value;
}

void SetMine(context_t *ctxt, int x, int y)
{
    ctxt->board[y*ctxt->columns + x].value = MINE;
    int i;
    for(i = 0; i < 8; ++i)
        UpdateValue(ctxt, x+moveX[i], y+moveY[i]);
}

void GenBoard(context_t *ctxt)
{
    int i = 0,
        x, y;
    free(ctxt->board);
    ctxt->board = malloc(ctxt->rows * ctxt->columns * sizeof(*(ctxt->board)));
    if(ctxt->board == NULL)
    {
        fprintf(stderr, "Unable to malloc board in InitContext !\n");
        exit(EXIT_FAILURE);
    }
    for(x = 0; x < ctxt->rows; ++x)
    {
        for(y = 0; y < ctxt->columns; ++y)
        {
            ctxt->board[x*ctxt->columns + y].visited = SDL_FALSE;
            ctxt->board[x*ctxt->columns + y].value = 0;
            ctxt->board[x*ctxt->columns + y].flagged = SDL_FALSE;
        }
    }
    do
    {
        x = rand() % ctxt->columns;
        y = rand() % ctxt->rows;
        if(ctxt->board[y*ctxt->columns + x].value != MINE)
        {
            SetMine(ctxt, x, y);
            ++i;
        }
    }
    while(i != ctxt->nbombs);
}

void ExploreCell(context_t *ctxt, int x, int y)
{
    int i;
    if(x >= 0 && x < ctxt->columns && y >= 0 && y < ctxt->rows && !ctxt->board[y*ctxt->columns + x].visited && ctxt->board[y*ctxt->columns + x].value != MINE)
    {
        ctxt->board[y*ctxt->columns + x].visited = SDL_TRUE;
        if(ctxt->board[y*ctxt->columns + x].value == 0)
            for(i = 0; i < 8; ++i)
				ExploreCell(ctxt, x+moveX[i], y+moveY[i]);
    }
}

int CountFlagsAround(context_t *ctxt, int x, int y)
{
    int i, count = 0;
    for(i = 0; i < 8; ++i)
        if((y+moveY[i]) >= 0 && (y+moveY[i]) < ctxt->rows && (x+moveX[i]) >= 0 && x+moveX[i] < ctxt->columns)
            count += ctxt->board[(y+moveY[i])*ctxt->columns + x+moveX[i]].flagged;
    return count;
}

SDL_bool isover(context_t *ctxt)
{
    int i, j;
    for(i = 0; i < ctxt->rows; ++i)
        for(j = 0; j < ctxt->columns; ++j)
            if(ctxt->board[i*ctxt->columns + j].value != MINE && !ctxt->board[i*ctxt->columns + j].visited)
                return SDL_FALSE;
    return SDL_TRUE;
}

void Play(context_t *ctxt)
{
    SDL_bool won = SDL_TRUE;
    SDL_Event e;
    int i;
    GenBoard(ctxt);
    SDL_FreeSurface(ctxt->screen);
    ctxt->screen = SDL_SetVideoMode(ctxt->columns*WIDTH_CELL, ctxt->rows*HEIGHT_CELL, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    BlitAll(ctxt, SDL_TRUE);
    SDL_Flip(ctxt->screen);
    while(won && !isover(ctxt))
    {
        SDL_WaitEvent(&e);
        if(e.type == SDL_MOUSEBUTTONDOWN)
        {
            if(e.button.button == SDL_BUTTON_LEFT && !ctxt->board[(e.button.y/HEIGHT_CELL)*ctxt->columns + e.button.x/WIDTH_CELL].flagged)
            {
                if(ctxt->board[(e.button.y/HEIGHT_CELL)*ctxt->columns + e.button.x/WIDTH_CELL].value == MINE)
                    won = SDL_FALSE;
                else
                    ExploreCell(ctxt, e.button.x/WIDTH_CELL, e.button.y/HEIGHT_CELL);
            }
            else if(e.button.button == SDL_BUTTON_RIGHT)
            {
                if(!ctxt->board[(e.button.y/HEIGHT_CELL)*ctxt->columns + e.button.x/WIDTH_CELL].visited)
                    ctxt->board[(e.button.y/HEIGHT_CELL)*ctxt->columns + e.button.x/WIDTH_CELL].flagged =
                                    1 - ctxt->board[(e.button.y/HEIGHT_CELL)*ctxt->columns + e.button.x/WIDTH_CELL].flagged;;
            }
            else if(e.button.button == SDL_BUTTON_MIDDLE)
            {
                if(ctxt->board[(e.button.y/HEIGHT_CELL)*ctxt->columns + e.button.x/WIDTH_CELL].visited)
                    if(CountFlagsAround(ctxt, e.button.x/WIDTH_CELL, e.button.y/HEIGHT_CELL) ==
                       ctxt->board[(e.button.y/HEIGHT_CELL)*ctxt->columns + e.button.x/WIDTH_CELL].value)
                        for(i = 0; i < 8; ++i)
                            ExploreCell(ctxt, e.button.x/WIDTH_CELL + moveX[i], e.button.y/HEIGHT_CELL + moveY[i]);
            }
            BlitAll(ctxt, SDL_FALSE);
            SDL_Flip(ctxt->screen);
        }
    }
    if(won)
    {
        SDL_Delay(2000);
    }
    else
    {
        BlitAll(ctxt, SDL_TRUE);
        SDL_Flip(ctxt->screen);
        SDL_Delay(1000);
    }
}

void BlitNumber(context_t *ctxt, unsigned short number, int x, int y)
{
    int i;
    SDL_Rect dst;
    dst.w = WIDTH_CELL;
    dst.h = HEIGHT_CELL;
    dst.y = y;
    for(i = 0; i < 3; ++i)
    {
        dst.x = x + (2-i)*WIDTH_CELL;
        SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[number % 10 != 0 ? number % 10 : ZERO], ctxt->screen, &dst);
        number /= 10;
    }
}

void Settings(context_t *ctxt)
{
    SDL_Event e;
    SDL_Surface *img = SDL_LoadBMP("Settings.bmp");
    SDL_FreeSurface(ctxt->screen);
    ctxt->screen = SDL_SetVideoMode(720, 720, 32, SDL_HWSURFACE);
    do
    {
        SDL_BlitSurface(img, NULL, ctxt->screen, NULL);
        BlitNumber(ctxt, ctxt->rows, 7*WIDTH_CELL, 4*HEIGHT_CELL);
        BlitNumber(ctxt, ctxt->columns, 8*WIDTH_CELL, 7*HEIGHT_CELL);
        BlitNumber(ctxt, ctxt->nbombs, 7*WIDTH_CELL, 11*HEIGHT_CELL);
        SDL_Flip(ctxt->screen);
        do
            SDL_WaitEvent(&e);
        while(e.type != SDL_KEYDOWN);
        if(e.key.keysym.sym == SDLK_RIGHT)
            ++ctxt->columns;
        else if(e.key.keysym.sym == SDLK_LEFT)
            --ctxt->columns;
        else if(e.key.keysym.sym == SDLK_DOWN)
            --ctxt->rows;
        else if(e.key.keysym.sym == SDLK_UP)
            ++ctxt->rows;
        else if(e.key.keysym.sym == SDLK_KP_PLUS)
            ++ctxt->nbombs;
        else if(e.key.keysym.sym == SDLK_KP_MINUS)
            --ctxt->nbombs;
    }
    while(e.key.keysym.sym != SDLK_RETURN);
}

int Menu(context_t *ctxt)
{
    SDL_Event e;
    SDL_Surface *img = SDL_LoadBMP("Menu.bmp");
    SDL_FreeSurface(ctxt->screen);
    ctxt->screen = SDL_SetVideoMode(720, 720, 32, SDL_HWSURFACE);
    SDL_BlitSurface(img, NULL, ctxt->screen, NULL);
    SDL_Flip(ctxt->screen);
    do
        SDL_WaitEvent(&e);
    while(e.type != SDL_KEYDOWN || e.key.keysym.sym < SDLK_KP1 || e.key.keysym.sym > SDLK_KP3);
    return e.key.keysym.sym - SDLK_KP0;
}

void BlitAll(context_t *ctxt, SDL_bool showbombs)
{
    int i, j;
    SDL_Rect dst;
    dst.w = WIDTH_CELL;
    dst.h = HEIGHT_CELL;
    for(i = 0; i < ctxt->rows; ++i)
    {
        dst.y = i*dst.h;
        for(j = 0; j < ctxt->columns; ++j)
        {
            dst.x = j*dst.w;
            if(showbombs && ctxt->board[i*ctxt->columns + j].value == MINE)
                SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[BOMB], ctxt->screen, &dst);
            else if(ctxt->board[i*ctxt->columns + j].flagged)
                SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[FLAG], ctxt->screen, &dst);
            else if(!ctxt->board[i*ctxt->columns + j].visited)
                SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[UNEXPLORED], ctxt->screen, &dst);
            else
                SDL_BlitSurface(ctxt->tileset.image, &ctxt->tileset.pos[ctxt->board[i*ctxt->columns + j].value], ctxt->screen, &dst);
        }
    }
}

