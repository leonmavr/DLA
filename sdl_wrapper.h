#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
} sdl_context_t;


sdl_context_t *sdl_context_create(const char *title, int width, int height);
void sdl_context_release(sdl_context_t *context);
bool sdl_context_render(sdl_context_t *context, uint32_t **img_raw, int width,
                        int height);

#endif // SDL_WRAPPER_H
