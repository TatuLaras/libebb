#include "orbital_controls.h"
#include <raymath.h>

#define ROTATE_SENSITIVITY_X 0.004
#define ROTATE_SENSITIVITY_Y 0.006
#define SHIFT_SENSITIVITY_X 0.0013
#define SHIFT_SENSITIVITY_Y 0.0013

#define BASE_ZOOM_SENSITIVITY 0.08
#define DRAG_ZOOM_SENSITIVITY_MULTIPLIER 0.02

void orbital_adjust_camera_zoom(Camera *camera, float amount) {
    if (amount == 0)
        return;

    Vector3 target_to_pos = Vector3Subtract(camera->position, camera->target);
    Vector3 local_z_axis = Vector3Normalize(Vector3Scale(target_to_pos, -1));

    amount = (amount * Vector3Length(target_to_pos)) * BASE_ZOOM_SENSITIVITY;
    camera->position =
        Vector3Add(camera->position, Vector3Scale(local_z_axis, amount));
}

void orbital_camera_update(Camera *camera, int zoom_with_mouse_horizontal) {
    Vector2 mouse_delta = GetMouseDelta();
    Vector3 target_to_pos = Vector3Subtract(camera->position, camera->target);
    Vector3 local_z_axis = Vector3Normalize(Vector3Scale(target_to_pos, -1));
    Vector3 local_x_axis = Vector3CrossProduct(local_z_axis, camera->up);

    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        Vector3 local_y_axis = Vector3CrossProduct(local_z_axis, local_x_axis);
        float shift_x =
            -mouse_delta.x * SHIFT_SENSITIVITY_X * Vector3Length(target_to_pos);
        float shift_y =
            -mouse_delta.y * SHIFT_SENSITIVITY_Y * Vector3Length(target_to_pos);
        camera->target =
            Vector3Add(camera->target, Vector3Scale(local_x_axis, shift_x));
        camera->target =
            Vector3Add(camera->target, Vector3Scale(local_y_axis, shift_y));
        camera->position =
            Vector3Add(camera->position, Vector3Scale(local_x_axis, shift_x));
        camera->position =
            Vector3Add(camera->position, Vector3Scale(local_y_axis, shift_y));
        return;
    }

    if (IsKeyDown(KEY_LEFT_CONTROL)) {
        float delta = mouse_delta.x;
        if (zoom_with_mouse_horizontal)
            delta = mouse_delta.y;

        orbital_adjust_camera_zoom(camera,
                                   delta * DRAG_ZOOM_SENSITIVITY_MULTIPLIER);
        return;
    }

    float yaw = -mouse_delta.x * ROTATE_SENSITIVITY_X;
    float pitch = -mouse_delta.y * ROTATE_SENSITIVITY_Y;

    Quaternion yaw_q = QuaternionFromAxisAngle((Vector3){0, 1.0, 0}, yaw);
    Quaternion pitch_q = QuaternionFromAxisAngle(local_x_axis, pitch);
    Quaternion total_q = QuaternionMultiply(pitch_q, yaw_q);

    target_to_pos = Vector3RotateByQuaternion(target_to_pos, total_q);
    camera->position = Vector3Add(camera->target, target_to_pos);
}
