#include "raycast.h"

#include "scene.h"

Vector3 raycast_ground_intersection(Ray ray, float ground_height) {
    float t = (ground_height - ray.position.y) / ray.direction.y;
    return Vector3Add(ray.position, Vector3Scale(ray.direction, t));
}

ObjectRaycastResult raycast_scene_objects(Ray ray) {
    ObjectRaycastResult object_result = {0};
    RayCollision collision = {0};
    Entity *entity = {0};
    size_t i = 0;
    while ((entity = scene_get_entity(i++))) {
        if (entity->is_destroyed)
            continue;

        Model *model = scene_entity_get_model(entity);

        collision =
            GetRayCollisionMesh(ray, model->meshes[0], entity->transform);

        if (collision.hit && !entity->ignore_raycast) {
            int is_closer_than_previous =
                !object_result.result.hit ||
                (Vector3DistanceSqr(ray.position, collision.point) <
                 Vector3DistanceSqr(ray.position, object_result.result.point));

            if (is_closer_than_previous)
                object_result = (ObjectRaycastResult){
                    .result = collision,
                    .entity_id = i - 1,
                };
        }
    }

    return object_result;
}
