#include <SDL/SDL.h>
#include <stdio.h>
#include "def.h"

int main(int ac, char **av)
{
    context_t context;
    int r;
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Failed to init SDL : '%s'\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    atexit(SDL_Quit);
    SDL_WM_SetCaption("Minesweeper", NULL);
    SDL_WM_SetIcon(SDL_LoadBMP("ico.bmp"), NULL);
    InitContext(&context);
    do
    {
        switch(r = Menu(&context))
        {
        case PLAY:
            Play(&context);
            break;
        case SETTINGS:
            Settings(&context);
            break;
        default:
            break;
        }
    }
    while(r != EXIT);
    FreeContext(&context);
    return 0;
    (void)ac;
    (void)av;
}
