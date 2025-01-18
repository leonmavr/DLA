#include "oct.h"
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

node_t *node_new(cuboid_t *boundary);
static bool node_is_leaf(node_t *node);
static void node_insert(node_t *node, point_t point);
static void node_nearest_neighbor(node_t *node, point_t query, point_t *nearest,
                                  double *best_dist_squared);
static bool point_in_cuboid(point_t point, cuboid_t boundary);
static void cuboid_divide(cuboid_t *src, cuboid_t *dest);
static int point_get_octant(cuboid_t cuboid, point_t point);
static double distance_sq(point_t p1, point_t p2);

static double distance_sq(point_t p1, point_t p2) {
  return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y) +
         (p1.z - p2.z) * (p1.z - p2.z);
}

static bool point_in_cuboid(point_t point, cuboid_t boundary) {
  return point.x >= boundary.x0 && point.x <= boundary.x1 &&
         point.y >= boundary.y0 && point.y <= boundary.y1 &&
         point.z >= boundary.z0 && point.z <= boundary.z1;
}

static void cuboid_divide(cuboid_t *src, cuboid_t *dest) {
  int mid_x = (src->x0 + src->x1) / 2;
  int mid_y = (src->y0 + src->y1) / 2;
  int mid_z = (src->z0 + src->z1) / 2;

  dest[0] = (cuboid_t){src->x0, src->y0, src->z0, mid_x, mid_y, mid_z};
  dest[1] = (cuboid_t){mid_x + 1, src->y0, src->z0, src->x1, mid_y, mid_z};
  dest[2] = (cuboid_t){src->x0, mid_y + 1, src->z0, mid_x, src->y1, mid_z};
  dest[3] = (cuboid_t){mid_x + 1, mid_y + 1, src->z0, src->x1, src->y1, mid_z};
  dest[4] = (cuboid_t){src->x0, src->y0, mid_z + 1, mid_x, mid_y, src->z1};
  dest[5] = (cuboid_t){mid_x + 1, src->y0, mid_z + 1, src->x1, mid_y, src->z1};
  dest[6] = (cuboid_t){src->x0, mid_y + 1, mid_z + 1, mid_x, src->y1, src->z1};
  dest[7] =
      (cuboid_t){mid_x + 1, mid_y + 1, mid_z + 1, src->x1, src->y1, src->z1};
}

static int point_get_octant(cuboid_t cuboid, point_t point) {
  int mid_x = (cuboid.x0 + cuboid.x1) / 2;
  int mid_y = (cuboid.y0 + cuboid.y1) / 2;
  int mid_z = (cuboid.z0 + cuboid.z1) / 2;

  int octant = 0;
  if (point.x > mid_x)
    octant |= 1;
  if (point.y > mid_y)
    octant |= 2;
  if (point.z > mid_z)
    octant |= 4;
  return octant;
}

node_t *node_new(cuboid_t *boundary) {
  node_t *node = malloc(sizeof(node_t));
  node->boundary = *boundary;
  node->count = 0;
  node->points = malloc(MAX_CHILDREN * sizeof(point_t));
  for (int i = 0; i < 8; ++i)
    node->children[i] = NULL;
  return node;
}

// Check if a node is a leaf
static bool node_is_leaf(node_t *node) {
  for (int i = 0; i < 8; ++i) {
    if (node->children[i])
      return false;
  }
  return true;
}

static void node_insert(node_t *node, point_t point) {
  if (!point_in_cuboid(point, node->boundary))
    return;
  if (node->count < 4 && node_is_leaf(node)) {
    node->points[node->count++] = point;
    return;
  }

  if (node_is_leaf(node)) {
    cuboid_t subcuboids[8];
    cuboid_divide(&node->boundary, subcuboids);
    for (int i = 0; i < 8; ++i) {
      node->children[i] = node_new(&subcuboids[i]);
    }
    for (int i = 0; i < node->count; ++i) {
      int octant = point_get_octant(node->boundary, node->points[i]);
      node_insert(node->children[octant], node->points[i]);
    }
    node->count = 0;
  }
  int octant = point_get_octant(node->boundary, point);
  node_insert(node->children[octant], point);
}

static void node_nearest_neighbor(node_t *node, point_t query, point_t *nearest,
                                  double *best_dist_squared) {
  if (!node)
    return;
  for (size_t i = 0; i < node->count; ++i) {
    double dist_sq = distance_sq(node->points[i], query);
    if (dist_sq < *best_dist_squared) {
      *best_dist_squared = dist_sq;
      *nearest = node->points[i];
    }
  }

  int octant = point_get_octant(node->boundary, query);
  if (node->children[octant]) {
    node_nearest_neighbor(node->children[octant], query, nearest,
                          best_dist_squared);
  }
  for (int i = 0; i < 8; ++i) {
    if (i != octant && node->children[i]) {
      node_nearest_neighbor(node->children[i], query, nearest,
                            best_dist_squared);
    }
  }
}

// wrapper functions
void octree_insert(octree_t *octree, point_t point) {
  node_insert(octree->root, point);
}

point_t octree_nearest_neighbor(octree_t *octree, point_t query) {
  point_t nearest = {0};
  double best_dist_squared = DBL_MAX;
  node_nearest_neighbor(octree->root, query, &nearest, &best_dist_squared);
  return nearest;
}

static void node_free(node_t *node) {
  if (!node)
    return;
  free(node->points);
  for (int i = 0; i < 8; ++i) {
    if (node->children[i]) {
      node_free(node->children[i]);
      node->children[i] = NULL;
    }
  }
  free(node);
}

void octree_free(octree_t *octree) {
  if (!octree || !octree->root)
    return;
  node_free(octree->root);
  octree->root = NULL; // Avoid dangling pointer
}
