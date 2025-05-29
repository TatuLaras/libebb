#include "components.h"

COMPONENT_IMPLEMENT(TransformComponent, COMPONENT_ID_TRANSFORM)
COMPONENT_IMPLEMENT(Mesh, COMPONENT_ID_MESH)
COMPONENT_IMPLEMENT(Camera, COMPONENT_ID_CAMERA)

void components_free(void) {
    COMPONENT_FREE(TransformComponent)
    COMPONENT_FREE(Mesh)
    COMPONENT_FREE(Camera)
}
