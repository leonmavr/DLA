#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  float x, y, z;
} vec3f_t;
typedef struct { int x, y; } vec2i_t;
typedef struct { uint8_t x, y, z; } vec3u_t;

/** Pinhole camera */
typedef struct {
  float cx, cy;           // camera's center of perspective
  float f;                // focal length
  struct {
    float x0, y0, x1, y1;
    int width, height;    // for easier referencing
  } boundary;             // projection plane boundaries
  vec2i_t (*project)(float x, float y, float z, bool *is_visible);
} camera_t;

typedef struct {
  camera_t camera;
  float **dbuffer;        // depth buffer - depth of each point
  uint32_t **pbuffer;     // color buffer - color of each point
  uint8_t bg_color[4];    // background color  
  void (*init)(float cx, float cy, float f, float fovx_deg, float fovy_deg);
} scene_t;

typedef struct {
  vec3f_t origin;
  float rad;
  vec3u_t color;
} sphere_t;

extern scene_t scene;

sphere_t sphere_make(float x0, float y0, float z0, float rad, uint8_t r, uint8_t g, uint8_t b);

void sphere_write(sphere_t* sphere);
void pbuffer_save(const char* filename);
void dbuffer_write(int x, int y, float dist, uint32_t color);

void scene_init(float cx, float cy, float f, float fovx_deg, float fovy_deg);
void scene_background(uint8_t r, uint8_t g, uint8_t b);
void buffer_free();
void render_to_sdl(SDL_Renderer *renderer);

#endif // CAMERA_H
