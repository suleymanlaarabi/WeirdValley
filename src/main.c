#define SOKOL_IMPL
#define SOKOL_GLUE_IMPL
#define SOKOL_GP_IMPL
#define SOKOL_METAL
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define SGP_BATCH_OPTIMIZER_DEPTH 12

#include "sokol_flecs.h"
#include "flecs.h"
#include "transform.h"
#include "controller.h"
#include "sokol_animation.h"
#include "player.h"
#include "tiled_flecs.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

ecs_world_t *world;

InputState input_state;

static void frame(void) {
    ecs_progress(world, 0);
}

static void init(void) {
    world = ecs_init();
    ECS_IMPORT(world, FlecsRest);
    ECS_IMPORT(world, FlecsStats);
    ECS_IMPORT(world, Transform);
    ECS_IMPORT(world, Sokol);
    ECS_IMPORT(world, SokolAnimation);
    ECS_IMPORT(world, Controller);
    ECS_IMPORT(world, Player);
    ECS_IMPORT(world, Tiled);
    ecs_shrink(world);
}

static void cleanup(void) {
    sgp_shutdown();
    sg_shutdown();
}

void input_event(const sapp_event* e) {
    if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
        input_state.keys[e->key_code] = true;
    } else if (e->type == SAPP_EVENTTYPE_KEY_UP) {
        input_state.keys[e->key_code] = false;
    }
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .window_title = "Rectangle (Sokol GP)",
        .logger.func = slog_func,
        .event_cb = input_event,
    };
}
