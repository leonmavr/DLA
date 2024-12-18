#include "camera.h"
#include "utils.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define UT_ABS(a) ((a) > 0 ? (a) : -(a))
#define UT_PI 3.141592653589
#define UT_TWO_PI 2 * UT_PI
#define UT_MIN(a, b) ((a) < (b) ? (a) : (b))
#define UT_MAX(a, b) ((a) > (b) ? (a) : (b))
#define UT_DEG2RAD(deg) ((deg) * UT_PI / 180.0)

scene_t scene;

static vec2i_t cam_project(float x, float y, float z, bool *is_visible) {
    const float cx = scene.camera.cx, cy = scene.camera.cy, f = scene.camera.f;
    // negate x and y to avoid inverted projections
    vec2i_t projected = (vec2i_t) {f*x/z - cx, f*y/z - cy};
    *is_visible =
        (projected.x > scene.camera.boundary.x0 && projected.x < scene.camera.boundary.x1) &&
        (projected.y > scene.camera.boundary.y0 && projected.y < scene.camera.boundary.y1);
    return projected;
}

void scene_init(float cx, float cy, float f, float fovx_deg, float fovy_deg) {
  //scene.init = scene_init;
  scene.camera.project = cam_project;
  scene.camera.cx = cx;
  scene.camera.cy = cy;
  scene.camera.f = f;
  scene.camera.boundary.x0 = UT_MIN(f * tan(UT_DEG2RAD(fovx_deg / 2)) + cx,
                                    f * tan(UT_DEG2RAD(-fovx_deg / 2)) + cx);
  scene.camera.boundary.x1 = UT_MAX(f * tan(UT_DEG2RAD(fovx_deg / 2)) + cx,
                                    f * tan(UT_DEG2RAD(-fovx_deg / 2)) + cx);
  scene.camera.boundary.y0 = UT_MIN(f * tan(UT_DEG2RAD(fovy_deg / 2)) + cy,
                                    f * tan(UT_DEG2RAD(-fovy_deg / 2)) + cy);
  scene.camera.boundary.y1 = UT_MAX(f * tan(UT_DEG2RAD(fovy_deg / 2)) + cy,
                                    f * tan(UT_DEG2RAD(-fovy_deg / 2)) + cy);
  scene.camera.boundary.width =
      scene.camera.boundary.x1 - scene.camera.boundary.x0;
  scene.camera.boundary.height =
      scene.camera.boundary.y1 - scene.camera.boundary.y0;
  // initialize the two buffers
  scene.dbuffer = malloc(scene.camera.boundary.height * sizeof(float *));
  scene.pbuffer = malloc(scene.camera.boundary.height * sizeof(uint32_t*));
  // TODO: check if alloc failed - pbuffer and pbuffer[0]
  for (int i = 0; i < scene.camera.boundary.height; i++) {
    scene.dbuffer[i] =
        malloc(scene.camera.boundary.width * sizeof(scene.dbuffer[0]));
    scene.pbuffer[i] =
        malloc(scene.camera.boundary.width * sizeof(scene.pbuffer[0]));
  }
  for (int row = 0; row < scene.camera.boundary.height; ++row) {
    for (int col = 0; col < scene.camera.boundary.width; ++col) {
      scene.dbuffer[row][col] = FLT_MAX;
      scene.pbuffer[row][col] = 100;
    }
  }
}



void pbuffer_write(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    // as 0x00RRGGBB
    const uint32_t color = (r << 16) | (g << 8) | b;
    // map from camera plane to 2D array indexes
    int x_idx = lmap_float(x, scene.camera.boundary.x0, scene.camera.boundary.x1,
                              0, scene.camera.boundary.width - 1);
    int y_idx = lmap_float(y, scene.camera.boundary.y0, scene.camera.boundary.y1,
                              0, scene.camera.boundary.height - 1);
    scene.pbuffer[y_idx][x_idx] = color;
}

bool dbuffer_write(int x, int y, float dist) {
    // as 0x00RRGGBB
    // map from camera plane to 2D array indexes
    int x_idx = lmap_float(x, scene.camera.boundary.x0, scene.camera.boundary.x1,
                              0, scene.camera.boundary.width - 1);
    int y_idx = lmap_float(y, scene.camera.boundary.y0, scene.camera.boundary.y1,
                              0, scene.camera.boundary.height - 1);
    bool is_closer = false;
    if (scene.dbuffer[y_idx][x_idx] > dist) {
        scene.dbuffer[y_idx][x_idx] = dist;
        is_closer = true; 
    }
    return is_closer;
}

void sphere_write(sphere_t* sphere) {
    // Extract sphere properties
    float rad = sphere->rad;
    vec3f_t origin = sphere->origin;
    vec3u_t color = sphere->color;

    // Camera origin
    float cam_x = scene.camera.cx;
    float cam_y = scene.camera.cy;
    float cam_z = -scene.camera.f; // The camera looks along the -Z direction

    // Iterate over the bounding box of the sphere in the projected plane
    for (int x = floor(origin.x - rad); x <= ceil(origin.x + rad); ++x) {
        for (int y = floor(origin.y - rad); y <= ceil(origin.y + rad); ++y) {
            float dx = x - origin.x;
            float dy = y - origin.y;
            if (dx * dx + dy * dy <= rad * rad) {
                float dz = sqrtf(rad * rad - dx * dx - dy * dy);
                float sphere_z = origin.z - dz;
                bool is_visible = false;
                vec2i_t projected = scene.camera.project(x, y, sphere_z, &is_visible);
                if (!is_visible) continue;

                // camera coordinates to buffer coordinates
                int px = projected.x;
                int py = projected.y;
                float depth = sqrtf((origin.x - cam_x) * (origin.x - cam_x) +
                                    (origin.y - cam_y) * (origin.y - cam_y) +
                                    (sphere_z - cam_z) * (sphere_z - cam_z));
                // Depth test: write only if the point is closer than the current depth
                bool is_closer = dbuffer_write(px, py, depth);
                if (is_closer) pbuffer_write(px, py, color.x, color.y, color.z);
            }
        }
    }
}

sphere_t sphere_make(float x0, float y0, float z0, float rad, uint8_t r, uint8_t g, uint8_t b) {
    return (sphere_t){.origin=(vec3f_t){x0, y0,z0}, .rad=rad, .color=(vec3u_t){r, g, b}};
}

void pbuffer_save(const char* filename) {
    // open the file for writing
    FILE* ppm_file = fopen(filename, "wb");
    if (ppm_file == NULL) {
        perror("Error opening output file.\n");
        return;
    }
    fprintf(ppm_file, "P3\n%d %d\n255\n", scene.camera.boundary.width, scene.camera.boundary.height);
    // write the pixel buffer into the file
    for (int r = 0; r < scene.camera.boundary.height; ++r) {
        for (int c = 0; c < scene.camera.boundary.width; ++c) {
            uint32_t color = scene.pbuffer[r][c];
            uint8_t b = color & 0xff, g = (color >> 8) & 0xff, r = (color >> 16) & 0xff;
            fprintf(ppm_file, "%u %u %u ", r, g, b);
        }
        fprintf(ppm_file, "\n");
    } 
    printf("Saved ray tracing output as %s.\n", filename);
    fclose(ppm_file);
}

void buffer_free() {
    for (int i = 0; i < scene.camera.boundary.height; ++i) {
        free(scene.pbuffer[i]);
        free(scene.dbuffer[i]);
    }
    free(scene.pbuffer);   
    free(scene.dbuffer);   
}
