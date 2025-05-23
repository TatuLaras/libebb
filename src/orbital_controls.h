#ifndef _ORBITAL_CONTROLS
#define _ORBITAL_CONTROLS

#include <raylib.h>

// Updates `camera` positions according to user input in order to enable
// Blender-style orbital controls.
//
// Hold middle mouse down to orbit
//
// Additionally:
//
// Shift to shift camera position
//
// Control to move camera closer or further
void orbital_camera_update(Camera *camera, int zoom_with_mouse_horizontal);
// Moves camera closer to its target by `amount`.
void orbital_adjust_camera_zoom(Camera *camera, float amount);

#endif
