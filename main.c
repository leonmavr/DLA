#include "camera.h"
#include "sdl_wrapper.h"
#include "utils.h"
#include <stdio.h>

int main() {
  printf("start\n");

  // Initialize the scene
  scene_init(0, 0, 300, 40, 30);

  // Add spheres to the scene
  for (int i = 0; i < 300 + xrandom() % 200; ++i) {
    sphere_t sphere = sphere_make(
        -600 + xrandom() % 1200, -400 + xrandom() % 800, 800, 30,
        150 + xrandom() % 50, 150 + xrandom() % 50, 150 + xrandom() % 50);
    sphere_write(&sphere);
  }

  // SDL context setup
  int width = scene.camera.boundary.width;
  int height = scene.camera.boundary.height;

  sdl_context_t *context = sdl_context_create("Ray-Traced Scene", 180, 260);
  if (!context) {
    return 1;
  }

  // Render loop
  bool is_done = false;
  while (!is_done) {
    is_done =
        sdl_context_render(context, (uint32_t **)scene.pbuffer, width, height);
    SDL_Delay(16); // Cap frame rate to ~60 FPS
  }
  // Save buffer to PPM after rendering
  // pbuffer_save("output.ppm");

  // Cleanup
  buffer_free();
  sdl_context_release(context);

  return 0;
}
