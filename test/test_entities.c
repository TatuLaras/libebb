#include "entities.h"
#include "handles.h"
#include "unity.h"

static EntityHandle has_mesh;
static EntityHandle has_transform;
static EntityHandle has_transform_camera;
static EntityHandle has_transform_mesh;
static EntityHandle has_transform_camera_renderable;
static EntityHandle has_camera;
static EntityHandle has_none;

void setUp(void) {
    entities_init();
    entities_new();
    entities_new();
    has_camera = entities_new();
    has_transform = entities_new();
    has_transform_mesh = entities_new();
    has_transform_camera = entities_new();
    has_transform_camera_renderable = entities_new();
    has_mesh = entities_new();
    has_none = entities_new();

    entities_register_component(has_camera, COMPONENT_ID_CAMERA);

    entities_register_component(has_transform, COMPONENT_ID_TRANSFORM);

    entities_register_component(has_transform_mesh, COMPONENT_ID_TRANSFORM);
    entities_register_component(has_transform_mesh, COMPONENT_ID_MESH);

    entities_register_component(has_transform_camera, COMPONENT_ID_TRANSFORM);
    entities_register_component(has_transform_camera, COMPONENT_ID_CAMERA);

    entities_register_component(has_transform_camera_renderable,
                                COMPONENT_ID_TRANSFORM);
    entities_register_component(has_transform_camera_renderable,
                                COMPONENT_ID_CAMERA);
    entities_register_component(has_transform_camera_renderable,
                                COMPONENT_ID_RENDERABLE);

    entities_register_component(has_mesh, COMPONENT_ID_MESH);
}

void tearDown(void) {
    entities_free();
}

void test_handles_work(void) {
    TEST_ASSERT_NOT_EQUAL(has_none, has_mesh);
    TEST_ASSERT_NOT_EQUAL(has_transform_mesh, has_camera);
}

void test_query_one_finds_correct_entity(void) {
    // One
    EntityHandle result = 0;
    TEST_ASSERT_FALSE(entities_query_one(COMPONENT_ID_CAMERA, &result));
    TEST_ASSERT_EQUAL(has_camera, result);

    result = 0;
    TEST_ASSERT_FALSE(entities_query_one(
        COMPONENT_ID_MESH | COMPONENT_ID_TRANSFORM, &result));
    TEST_ASSERT_EQUAL(has_transform_mesh, result);
}

void test_query_one_returns_1_when_not_found(void) {
    EntityHandle result = 0;
    TEST_ASSERT_TRUE(
        entities_query_one(COMPONENT_ID_MESH | COMPONENT_ID_CAMERA, &result));
    TEST_ASSERT_EQUAL(0, result);
}

void test_query_finds_all_correct_entities(void) {
    EntityHandleVector result = entities_query(COMPONENT_ID_TRANSFORM);
    TEST_ASSERT_EQUAL(4, result.data_used);

    TEST_ASSERT_EQUAL(has_transform, result.data[0]);
    TEST_ASSERT_EQUAL(has_transform_mesh, result.data[1]);
    TEST_ASSERT_EQUAL(has_transform_camera, result.data[2]);
    TEST_ASSERT_EQUAL(has_transform_camera_renderable, result.data[3]);

    entityhandlevec_free(&result);

    result = entities_query(COMPONENT_ID_TRANSFORM | COMPONENT_ID_CAMERA);
    TEST_ASSERT_EQUAL(2, result.data_used);

    TEST_ASSERT_EQUAL(has_transform_camera, result.data[0]);
    TEST_ASSERT_EQUAL(has_transform_camera_renderable, result.data[1]);

    entityhandlevec_free(&result);
}

void test_query_returns_empty_vector_when_not_found(void) {
    EntityHandleVector result =
        entities_query(COMPONENT_ID_MESH | COMPONENT_ID_CAMERA);
    TEST_ASSERT_EQUAL(0, result.data_used);
    entityhandlevec_free(&result);
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_handles_work);
    RUN_TEST(test_query_one_finds_correct_entity);
    RUN_TEST(test_query_one_returns_1_when_not_found);
    RUN_TEST(test_query_finds_all_correct_entities);
    RUN_TEST(test_query_returns_empty_vector_when_not_found);

    return UNITY_END();
}
