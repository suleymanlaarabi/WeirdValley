#include <flecs.h>
#include <stdio.h>
#include "utils.h"
#include "sokol_animation.h"
#include "sokol_flecs.h"

ECS_COMPONENT_DECLARE(SpriteSheetAtlas);
ECS_COMPONENT_DECLARE(PlayingAnimation);
ECS_COMPONENT_DECLARE(SpriteSheetAnimationTimer);

void UpdateAnimationTimerSystem(ecs_iter_t *it) {
    SpriteSheetAnimationTimer *times = ecs_field(it, SpriteSheetAnimationTimer, 0);
    const PlayingAnimation *animations = ecs_field(it, PlayingAnimation, 1);
    SpriteSheetAtlas *atlases = ecs_field(it, SpriteSheetAtlas, 2);
    ImageOffset *positions = ecs_field(it, ImageOffset, 3);
    const ImageRect *rects = ecs_field(it, ImageRect, 4);

    for (int i = 0; i < it->count; i++) {
        SpriteSheetAnimationTimer *time = &times[i];
        SpriteSheetAtlas *atlase = &atlases[i];
        const PlayingAnimation *animation = &animations[i];

        time->current_time += it->delta_time;

        bool delay_passed = time->current_time > time->delay;

        if (delay_passed) {
            time->current_time = 0;
            atlase->index += 1;
        }

        if (unlikely(atlase->index > animation->end)) {
            atlase->index = animation->start;
        } else if (atlase->index < animation->start) {
            atlase->index = animation->start;
        }

        if (delay_passed) {
            int col = atlase->index % atlase->cols;
            int row = atlase->index / atlase->cols;

            positions[i].x = rects[i].width * col;
            positions[i].y = rects[i].height * row;
        }
    }
}

void SokolAnimationImport(ecs_world_t *world) {
    ECS_MODULE(world, SokolAnimation);
    ECS_IMPORT(world, Sokol);
    ECS_COMPONENT_DEFINE(world, SpriteSheetAtlas);
    ECS_COMPONENT_DEFINE(world, PlayingAnimation);
    ECS_COMPONENT_DEFINE(world, SpriteSheetAnimationTimer);

    ecs_add_id(world, ecs_id(PlayingAnimation), EcsExclusive);
    ecs_struct(world, {
        .entity = ecs_id(SpriteSheetAtlas),
        .members = {
            { .name = "index", .type = ecs_id(ecs_i32_t) },
            { .name = "cols", .type = ecs_id(ecs_i32_t) },
            { .name = "rows", .type = ecs_id(ecs_i32_t) },
        }
    });
    ecs_struct(world, {
        .entity = ecs_id(PlayingAnimation),
        .members = {
            { .name = "start", .type = ecs_id(ecs_i32_t) },
            { .name = "end", .type = ecs_id(ecs_i32_t) },
        }
    });
    ecs_struct(world, {
        .entity = ecs_id(SpriteSheetAnimationTimer),
        .members = {
            { .name = "current_time", .type = ecs_id(ecs_f32_t) },
            { .name = "delay", .type = ecs_id(ecs_f32_t) },
        }
    });
    ECS_SYSTEM(world, UpdateAnimationTimerSystem, EcsOnUpdate,
        [inout] SpriteSheetAnimationTimer,
        [in] (PlayingAnimation, *),
        [inout] SpriteSheetAtlas,
        [inout] ImageOffset,
        [in] ImageRect
    );
}
