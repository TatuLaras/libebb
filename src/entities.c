#include "entities.h"

#include <assert.h>

VEC_IMPLEMENT(Entity, EntityVector, entityvec)
VEC_IMPLEMENT(EntityHandle, EntityHandleVector, entityhandlevec)

static EntityVector entities = {0};

void entities_init(void) {
    entities = entityvec_init();
}

EntityHandle entities_new(void) {
    assert(entities.data);
    return entityvec_append(&entities, (Entity){0});
}

EntityHandleVector entities_query(ComponentMask mask) {
    EntityHandleVector handles = entityhandlevec_init();

    for (size_t i = 0; i < entities.data_used; i++) {
        if ((entities.data[i].component_mask & mask) == mask)
            entityhandlevec_append(&handles, i);
    }

    return handles;
}

int entities_query_one(ComponentMask mask, EntityHandle *out_handle) {
    for (size_t i = 0; i < entities.data_used; i++) {
        if ((entities.data[i].component_mask & mask) == mask) {
            *out_handle = i;
            return 0;
        }
    }

    return 1;
}

void entities_register_component(EntityHandle entity, ComponentID component) {
    entities.data[entity].component_mask |= component;
}

void entities_free(void) {
    entityvec_free(&entities);
}
