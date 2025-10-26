#ifndef TRAY_TRANSFORM_H
    #define TRAY_TRANSFORM_H
    #include "flecs.h"

    #define REGISTER_VECTOR2_COMPONENT(world, component) ecs_struct(world, { \
            .entity = ecs_id(component), \
            .members = { \
                { .name = "x", .type = ecs_id(ecs_f32_t) }, \
                { .name = "y", .type = ecs_id(ecs_f32_t) } \
            } \
        });
typedef struct { float x, y; } Position, Velocity, Scale;
typedef struct { float width, height; } RectSize;
typedef float Rotation;
extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(Velocity);
extern ECS_COMPONENT_DECLARE(RectSize);
extern ECS_COMPONENT_DECLARE(Scale);
extern ECS_COMPONENT_DECLARE(Rotation);

void TransformImport(ecs_world_t *world);

#endif
