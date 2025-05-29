#ifndef _COMPONENTS
#define _COMPONENTS

#include "entities.h"
#include "handles.h"
#include "vec.h"
#include <raylib.h>

typedef struct {
    EntityHandle entity;
    size_t component;
} EntityComponentRelation;

//  NOTE: Not sure if this macro is a good idea. I definitely don't want to keep
//  writing these same functions over and over again for each new component but
//  this seems like an undebuggable / unmaintainable nightmare.
//   TODO: Metaprogramming solution for this using something other than the C
//   preprocessor.

// The macro below will declare two functions to add a component of type
// `component_type` to an entity.
//
// void components_add_{component_type}(EntityHandle entity, {component_type}
// component);
//
// This will add a component to an entity, such that it will be
// queryable with entities_query and the data will be retrievable by the next
// function.
//
// {component_type} *components_get_{component_type}(EntityHandle entity);
//
//  This will return a pointer to an entity's components data.

#define COMPONENT_DECLARE(component_type)                                      \
    VEC_DECLARE(component_type, component_type##Vector,                        \
                component_type##_comp_vec)                                     \
    VEC_DECLARE(EntityComponentRelation, component_type##RelationVector,       \
                component_type##_relation_vec)                                 \
    void components_add_##component_type(EntityHandle entity,                  \
                                         component_type component);            \
    component_type *components_get_##component_type(EntityHandle entity);

#define COMPONENT_IMPLEMENT(component_type, component_id)                      \
    VEC_IMPLEMENT(component_type, component_type##Vector,                      \
                  component_type##_comp_vec)                                   \
    VEC_IMPLEMENT(EntityComponentRelation, component_type##RelationVector,     \
                  component_type##_relation_vec)                               \
    static component_type##Vector component_type##_comp_vec = {0};             \
    static component_type##RelationVector component_type##_relation_vec = {0}; \
                                                                               \
    void components_add_##component_type(EntityHandle entity,                  \
                                         component_type component) {           \
        if (!component_type##_comp_vec.data)                                   \
            component_type##_comp_vec = component_type##_comp_vec_init();      \
        if (!component_type##_relation_vec.data)                               \
            component_type##_relation_vec =                                    \
                component_type##_relation_vec_init();                          \
                                                                               \
        size_t component_handle = component_type##_comp_vec_append(            \
            &component_type##_comp_vec, component);                            \
        component_type##_relation_vec_append(                                  \
            &component_type##_relation_vec,                                    \
            (EntityComponentRelation){.entity = entity,                        \
                                      .component = component_handle});         \
        entities_register_component(entity, component_id);                     \
    }                                                                          \
                                                                               \
    component_type *components_get_##component_type(EntityHandle entity) {     \
        for (size_t i = 0; i < component_type##_relation_vec.data_used; i++) { \
            if (component_type##_relation_vec.data[i].entity == entity) {      \
                return component_type##_comp_vec_get(                          \
                    &component_type##_comp_vec,                                \
                    component_type##_relation_vec.data[i].component);          \
            }                                                                  \
        }                                                                      \
        return 0;                                                              \
    }

// Must be used after COMPONENT_IMPLEMENT.
#define COMPONENT_FREE(component_type)                                         \
    component_type##_comp_vec_free(&component_type##_comp_vec);                \
    component_type##_relation_vec_free(&component_type##_relation_vec);

// ----- Components -----

typedef Matrix TransformComponent;

COMPONENT_DECLARE(TransformComponent)
COMPONENT_DECLARE(Mesh)
COMPONENT_DECLARE(Camera)

void components_free(void);

#endif
