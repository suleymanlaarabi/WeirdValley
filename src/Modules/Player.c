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
ECS_TAG_DECLARE(WalkingUp);
ECS_TAG_DECLARE(WalkingDown);

#define ANIMATION(a, b) { .start = a, .end = b }
#define WALKING_HANIMATION ANIMATION(24, 28)
#define WALKING_VANIMATION ANIMATION(18, 23)
#define WALKING_UP_ANIMATION ANIMATION(18, 23)
#define WALKING_DOWN_ANIMATION ANIMATION(30, 35)
#define IDLE_ANIMATION ANIMATION(0, 5)

void PlayerSystem(ecs_iter_t *it) {
    const Velocity *velocities = ecs_field(it, Velocity, 0);
    ImageSize *rects = ecs_field(it, ImageSize, 1);
    Position *positions = ecs_field(it, Position, 2);
    const ecs_id_t PlayingAnimationWalking = ecs_pair(ecs_id(PlayingAnimation), Walking);
    const ecs_id_t PlayingAnimationIdle = ecs_pair(ecs_id(PlayingAnimation), Idle);
    const ecs_id_t PlayingAnimationWalkingUp = ecs_pair(ecs_id(PlayingAnimation), WalkingUp);
    const ecs_id_t PlayingAnimationWalkingDown = ecs_pair(ecs_id(PlayingAnimation), WalkingDown);

    ecs_world_t *world = it->world;
    const ecs_entity_t *entities = it->entities;
    for (int i = 0; i < it->count; i++) {
        const float vel_x = velocities[i].x;
        const float vel_y = velocities[i].y;
        const ecs_entity_t entity = entities[i];

        if (vel_x == 0 & vel_y == 0) {
            if (!ecs_has_id(world, entity, PlayingAnimationIdle)) {
                ecs_set_pair(world, entity, PlayingAnimation, Idle, IDLE_ANIMATION);
                animation_set(world, entity, Idle, IDLE_ANIMATION);
            }
        } else if (vel_x != 0) {
            if (!ecs_has_id(world, entity, PlayingAnimationWalking)) {
                animation_set(world, entity, Walking, WALKING_HANIMATION);
            }
        } else if (vel_y > 0) {
            if (!ecs_has_id(world, entity, PlayingAnimationWalkingUp)) {
                animation_set(world, entity, WalkingUp, WALKING_UP_ANIMATION);
            }
        } else {
            if (!ecs_has_id(world, entity, PlayingAnimationWalkingDown)) {
                animation_set(world, entity, WalkingDown, WALKING_DOWN_ANIMATION);
            }
        }

        float rect_width = rects[i].width;
        if ((vel_x < 0 && rect_width > 0) || (vel_x > 0 && rect_width < 0)) {
            rects[i].width = -rect_width;
            positions[i].x -= -rect_width;
        }
    }
}

void MainPlayerSyncCamera(ecs_iter_t *it) {
    const Position *position = ecs_field(it, Position, 0);
    const ImageSize *size = ecs_field(it, ImageSize, 1);

    SokolCamera *camera = ecs_singleton_get_mut(it->world, SokolCamera);

    int screen_width = sapp_width();
    int screen_height = sapp_height();

    float player_center_x = position->x + (size->width / 2.0f);
    float player_center_y = position->y + (size->height / 2.0f);

    camera->position.x = player_center_x - (screen_width * camera->zoom / 2.0f);
    camera->position.y = player_center_y - (screen_height * camera->zoom / 2.0f);
}

void PlayerImport(ecs_world_t *world) {
    ECS_MODULE(world, Player);
    ECS_TAG_DEFINE(world, Player);
    ECS_TAG_DEFINE(world, MainPlayer);
    ECS_TAG_DEFINE(world, Walking);
    ECS_TAG_DEFINE(world, Idle);
    ECS_TAG_DEFINE(world, WalkingUp);
    ECS_TAG_DEFINE(world, WalkingDown);

    ecs_image(world, AdventurerImage, "./assets/Player.png");

    ecs_entity_t entity = ecs_entity(world, {
        .name = "Adventurer",
        .add = ecs_ids(Player, MainPlayer),
        .set = ecs_values(
            ecs_value(ImageOffset, {0, 0}),
            ecs_value(ImageRect, { 32, 32 }),
            ecs_value(ImageSize, { 32 * 3, 32 * 3 }),
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
                .rows = 10,
                .cols = 6,
                .index = 0
            })
        )
    });
    ecs_set_image(world, AdventurerImage, entity);

    ECS_SYSTEM(world, PlayerSystem, EcsOnUpdate,
        [in] transform.Velocity,
        [inout] sokol.ImageSize,
        [inout] transform.Position,
        [none] Player
    );
    ECS_SYSTEM(world, MainPlayerSyncCamera, EcsOnUpdate,
        [in] transform.Position,
        [in] sokol.ImageSize,
        [inout] sokol.SokolCamera(sokol.SokolCamera),
        [in] MainPlayer
    );
}
