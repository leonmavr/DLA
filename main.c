#include "camera.h"
#include "sdl_wrapper.h"
#include "utils.h"

int main() {
  scene_init(0, 0, 300, 80, 70);
  for (int i = 0; i < 300 + xrandom() % 200; ++i) {
    sphere_t sphere = sphere_make(
        -600 + xrandom() % 1200, -400 + xrandom() % 800, 800, 30,
        150 + xrandom() % 50, 150 + xrandom() % 50, 150 + xrandom() % 50);
    sphere_write(&sphere);
  }

  int width = scene.camera.boundary.width;
  int height = scene.camera.boundary.height;
  sdl_context_t *context =
      sdl_context_create("Multiple spheres", 720, (float)height / width * 720);
  if (!context) {
    return 1;
  }

  bool is_done = false;
  while (!is_done) {
    is_done =
        sdl_context_render(context, (uint32_t **)scene.pbuffer, width, height);
    const int delay_ms = 16;
    SDL_Delay(delay_ms);
  }
  // Uncomment to view the buffer as .ppm file
  // pbuffer_save("output.ppm");

  // Cleanup
  buffer_free();
  sdl_context_release(context);

  return 0;
}
