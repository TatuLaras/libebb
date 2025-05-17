#ifndef _MATH
#define _MATH

#include <raylib.h>
#include <raymath.h>
#include <stddef.h>

typedef struct {
    RayCollision result;
    size_t entity_id;
} ObjectRaycastResult;

// Intersection of `ray` with the ground plane.
Vector3 raycast_ground_intersection(Ray ray, float ground_height);

// Casts a `ray` into the scene and returns information about which object was
// hit and the hit point.
ObjectRaycastResult raycast_scene_objects(Ray ray);

#endif
