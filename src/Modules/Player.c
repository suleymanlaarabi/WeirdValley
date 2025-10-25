#include <math.h>
#include <flecs.h>
#include <machine/limits.h>
#include <math.h>
#include <player.h>
#include <sokol_flecs.h>
#include <sokol_animation.h>
#include <stdio.h>
#include <stdlib.h>
#include <transform.h>
#include <controller.h>

ECS_TAG_DECLARE(Player);
ECS_TAG_DECLARE(MainPlayer);
ECS_TAG_DECLARE(Walking);
ECS_TAG_DECLARE(Idle);

#define ANIMATION(a, b) { .start = a, .end = b }
#define WALKING_ANIMATION ANIMATION(9, 13)
#define IDLE_ANIMATION ANIMATION(0, 3)

void PlayerSystem(ecs_iter_t *it) {
    const Velocity *velocities = ecs_field(it, Velocity, 0);
    ImageSize *rects = ecs_field(it, ImageSize, 1);
    Position *positions = ecs_field(it, Position, 2);
    const ecs_id_t PlayingAnimationWalking = ecs_pair(ecs_id(PlayingAnimation), Walking);
    const ecs_id_t PlayingAnimationIdle = ecs_pair(ecs_id(PlayingAnimation), Idle);


    for (int i = 0; i < it->count; i++) {
        if (velocities[i].x == 0) {
            if (ecs_has_id(it->world, it->entities[i], PlayingAnimationWalking)) {
                ecs_set_pair(it->world, it->entities[i], PlayingAnimation, Idle, IDLE_ANIMATION);
            }
        } else {
            if (ecs_has_id(it->world, it->entities[i], PlayingAnimationIdle)) {
                ecs_set_pair(it->world, it->entities[i], PlayingAnimation, Walking, WALKING_ANIMATION);
            }
        }

        if ((velocities[i].x < 0 && rects[i].width > 0) || (velocities[i].x > 0 && rects[i].width < 0)) {
            rects[i].width = -rects[i].width;
            positions[i].x += -rects[i].width;
        }
    }
}

void MainPlayerSyncCamera(ecs_iter_t *it) {
    Position *position = ecs_field(it, Position, 0);
    SokolCamera *camera = ecs_singleton_get_mut(it->world, SokolCamera);

    camera->position = *position;
}

void PlayerImport(ecs_world_t *world) {
    ECS_MODULE(world, Player);
    ECS_TAG_DEFINE(world, Player);
    ECS_TAG_DEFINE(world, MainPlayer);
    ECS_TAG_DEFINE(world, Walking);
    ECS_TAG_DEFINE(world, Idle);

    ecs_image(world, AdventurerImage, "./assets/adventurer.png");

    ecs_entity_t entity = ecs_entity(world, {
        .name = "Adventurer",
        .add = ecs_ids(Player, MainPlayer),
        .set = ecs_values(
            ecs_value(ImageOffset, {0, 0}),
            ecs_value(ImageRect, { 50, 37 }),
            ecs_value(ImageSize, { 50 * 3, 37 * 3 }),
            ecs_value(Position, { 100, 100 }),
            ecs_value(Velocity, {0, 0}),
            ecs_value(CharacterController, {
                .key_up = SAPP_KEYCODE_W,
                .key_down = SAPP_KEYCODE_S,
                .key_right = SAPP_KEYCODE_D,
                .key_left = SAPP_KEYCODE_A,
                .speed = 400
            }),
            ecs_value(SpriteSheetAnimationTimer, {
                .current_time = 0,
                .delay = 0.15
            }),
            ecs_value(SpriteSheetAtlas, {
                .rows = 11,
                .cols = 7,
                .index = 0
            }),
            ecs_value_pair(PlayingAnimation, Idle, IDLE_ANIMATION)
        )
    });
    ecs_set_image(world, AdventurerImage, entity);

    ECS_SYSTEM(world, MainPlayerSyncCamera, EcsOnUpdate,
        [inout] transform.Position,
        [inout] sokol.SokolCamera(sokol.SokolCamera),
        [in] MainPlayer
    );
    ECS_SYSTEM(world, PlayerSystem, EcsOnUpdate,
        [in] transform.Velocity,
        [inout] sokol.ImageSize,
        [inout] transform.Position,
        [none] Player
    );
}
