#include <flecs.h>
#include <sys/cdefs.h>
#include "transform.h"

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(Rotation);
ECS_COMPONENT_DECLARE(Scale);
ECS_COMPONENT_DECLARE(RectSize);

void ApplyVelocitySystem(ecs_iter_t *it) {
    Position *positions = ecs_field(it, Position, 0);
    Velocity *velocities = ecs_field(it, Velocity, 1);

    for (int i = 0; i < it->count; i++) {
        positions[i].x += velocities[i].x * it->delta_time;
        positions[i].y += velocities[i].y * it->delta_time;
    }
}

void TransformImport(ecs_world_t *world) {
    ECS_MODULE(world, Transform);

    ECS_COMPONENT_DEFINE(world, Rotation);
    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_COMPONENT_DEFINE(world, Scale);
    ECS_COMPONENT_DEFINE(world, RectSize);

    REGISTER_VECTOR2_COMPONENT(world, Position);
    REGISTER_VECTOR2_COMPONENT(world, Scale);
    REGISTER_VECTOR2_COMPONENT(world, Velocity);
    REGISTER_VECTOR2_COMPONENT(world, RectSize);

    ecs_primitive(world, {
        .entity = ecs_id(Rotation),
        .kind = ecs_id(ecs_f32_t)
    });

    ECS_SYSTEM(world, ApplyVelocitySystem, EcsOnUpdate, [inout] Position, [in] Velocity);
}
