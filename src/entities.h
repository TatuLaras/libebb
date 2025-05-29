#ifndef _ENTITIES
#define _ENTITIES

#include "handles.h"
#include "vec.h"

// A mask with one bit for each component that the entity has. Which bits these
// are exactly is outlined in enum ComponentID below.
typedef uint64_t ComponentMask;

typedef enum {
    COMPONENT_ID_TRANSFORM = 1 << 0,
    COMPONENT_ID_MESH = 1 << 1,
    COMPONENT_ID_CAMERA = 1 << 2,
    COMPONENT_ID_RENDERABLE = 1 << 3,
} ComponentID;

typedef struct {
    ComponentMask component_mask;
} Entity;

VEC_DECLARE(Entity, EntityVector, entityvec)
VEC_DECLARE(EntityHandle, EntityHandleVector, entityhandlevec)

// Initializes memory for entities, call this before any other function in this
// module.
void entities_init(void);
void entities_free(void);
// Creates a new entity with no components, returns the handle of that entity.
EntityHandle entities_new(void);
// Returns an EntityHandleVector of all entity handles that have all components
// required by `mask`. Important: Ownership of the returned vector belongs to
// the caller.
EntityHandleVector entities_query(ComponentMask mask);
// Queries for the first entity handle that has all components required by
// `mask` and writes it to `out_handle`. Return value will be 0 in case of
// successful query, 1 if no such entity was found.
int entities_query_one(ComponentMask mask, EntityHandle *out_handle);

void entities_register_component(EntityHandle entity, ComponentID component);

#endif
