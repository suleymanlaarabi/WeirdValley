#include <flecs.h>
#include "controller.h"
#include "sokol_flecs.h"
#include "transform.h"

ECS_COMPONENT_DECLARE(CharacterController);

void MoveCharacterSystem(ecs_iter_t *it) {
    CharacterController *controllers = ecs_field(it, CharacterController, 0);
    Velocity *velocities = ecs_field(it, Velocity, 1);

    for (int i = 0; i < it->count; i++) {
        if (input_state.keys[controllers[i].key_up]) {
            velocities[i].y = -controllers[i].speed;
        } else if (input_state.keys[controllers[i].key_down]) {
            velocities[i].y = controllers[i].speed;
        } else {
            velocities[i].y = 0;
        }
        if (input_state.keys[controllers[i].key_right]) {
            velocities[i].x = controllers[i].speed;
        } else if (input_state.keys[controllers[i].key_left]) {
            velocities[i].x = -controllers[i].speed;
        } else {
            velocities[i].x = 0;
        }
    }
}

void ControllerImport(ecs_world_t *world) {
    ECS_MODULE(world, Controller);
    ECS_COMPONENT_DEFINE(world, CharacterController);

    ecs_struct(world, {
        .entity = ecs_id(CharacterController),
        .members = {
            { .name = "key_up", .type = ecs_id(ecs_i32_t) },
            { .name = "key_down", .type = ecs_id(ecs_i32_t) },
            { .name = "key_right", .type = ecs_id(ecs_i32_t) },
            { .name = "key_left", .type = ecs_id(ecs_i32_t) },
            { .name = "speed", .type = ecs_id(ecs_f32_t) },
        }
    });

    ECS_SYSTEM(world, MoveCharacterSystem, EcsPreUpdate, [in] CharacterController, [inout] transform.Velocity);
}
