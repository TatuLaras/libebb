#include "components.h"
#include "entities.h"
#include "handles.h"
#include "unity.h"

static EntityHandle has_transform;
static EntityHandle has_transform_mesh;
static EntityHandle has_transform_camera;
static TransformComponent transform1 = {.m0 = 123, .m10 = 4, .m15 = 2};
static TransformComponent transform2 = {.m0 = 124, .m10 = 5, .m15 = 3};
static TransformComponent transform3 = {.m0 = 125, .m10 = 6, .m15 = 4};
static Mesh mesh = {.vaoId = 56};
static Camera camera = {.position = (Vector3){1, 2, 3}};

void setUp(void) {
    entities_init();
    has_transform = entities_new();
    components_add_TransformComponent(has_transform, transform1);

    has_transform_mesh = entities_new();
    components_add_TransformComponent(has_transform_mesh, transform2);
    components_add_Mesh(has_transform_mesh, mesh);

    has_transform_camera = entities_new();
    components_add_TransformComponent(has_transform_camera, transform3);
    components_add_Camera(has_transform_camera, camera);
}

void tearDown(void) {
    entities_free();
    components_free();
}

void test_component_data_is_correct(void) {
    TransformComponent *actual_transform =
        components_get_TransformComponent(has_transform);
    TEST_ASSERT_EQUAL(transform1.m0, actual_transform->m0);
    TEST_ASSERT_EQUAL(transform1.m10, actual_transform->m10);
    TEST_ASSERT_EQUAL(transform1.m15, actual_transform->m15);

    actual_transform = components_get_TransformComponent(has_transform_mesh);
    Mesh *actual_mesh = components_get_Mesh(has_transform_mesh);
    TEST_ASSERT_EQUAL(transform2.m0, actual_transform->m0);
    TEST_ASSERT_EQUAL(transform2.m10, actual_transform->m10);
    TEST_ASSERT_EQUAL(transform2.m15, actual_transform->m15);
    TEST_ASSERT_EQUAL(mesh.vaoId, actual_mesh->vaoId);

    actual_transform = components_get_TransformComponent(has_transform_camera);
    Camera *actual_camera = components_get_Camera(has_transform_camera);
    TEST_ASSERT_EQUAL(transform3.m0, actual_transform->m0);
    TEST_ASSERT_EQUAL(transform3.m10, actual_transform->m10);
    TEST_ASSERT_EQUAL(transform3.m15, actual_transform->m15);
    TEST_ASSERT_EQUAL(camera.position.x, actual_camera->position.x);
    TEST_ASSERT_EQUAL(camera.position.y, actual_camera->position.y);
    TEST_ASSERT_EQUAL(camera.position.z, actual_camera->position.z);
}

void test_null_pointer_returned_if_no_such_component(void) {
    TEST_ASSERT_FALSE(components_get_Camera(has_transform_mesh));
    TEST_ASSERT_FALSE(components_get_Mesh(has_transform));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_component_data_is_correct);
    RUN_TEST(test_null_pointer_returned_if_no_such_component);

    return UNITY_END();
}
