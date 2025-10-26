#include "tiled.h"
#include "sokol_flecs.h"
#include "tiled_flecs.h"
#include "transform.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>

ECS_COMPONENT_DECLARE(EcsTiledMap);
ECS_COMPONENT_DECLARE(EcsTiledLayer);
ECS_COMPONENT_DECLARE(EcsTiledTileset);
ECS_TAG_DECLARE(TiledChunkVisible);
ECS_TAG_DECLARE(EcsTiledChunk);
ECS_TAG_DECLARE(EcsTile);

void TilesRenderSystem(ecs_iter_t *it) {
    const SokolCamera *camera = ecs_singleton_get(it->world, SokolCamera);
    const Position *positions = ecs_field(it, Position, 0);
    const Image *images = ecs_field(it, Image, 1);
    const ImageRect *rects = ecs_field(it, ImageRect, 2);
    const ImageOffset *offsets = ecs_field(it, ImageOffset, 3);
    const ImageSize *sizes = ecs_field(it, ImageSize, 4);

    sgp_set_image(0, images[0]);
    sgp_draw_textured_rects_sep(0, -camera->position.x, -camera->position.y, (sgp_vec2 *) positions, (sgp_vec2 *) sizes, offsets, (sgp_vec2 *) rects, it->count);
}

void MarkTiledChunkVisibleSystem(ecs_iter_t *it) {
    Position *positions = ecs_field(it, Position, 0);
    RectSize *rects = ecs_field(it, RectSize, 1);
    const SokolCamera *camera = ecs_singleton_get(it->world, SokolCamera);

    const float cam_start_x = camera->position.x;
    const float cam_end_x = camera->position.x + sapp_widthf();
    const float cam_start_y = camera->position.y;
    const float cam_end_y = camera->position.y + sapp_heightf();

    for (int i = 0; i < it->count; i++) {
        const float start_x = positions[i].x;
        const float end_x = positions[i].x + rects[i].width;
        const float start_y = positions[i].y;
        const float end_y = positions[i].y + rects[i].height;

        if (end_x >= cam_start_x && start_x <= cam_end_x &&
            end_y >= cam_start_y && start_y <= cam_end_y) {
            ecs_add(it->world, it->entities[i], TiledChunkVisible);
        }
    }
}

void MarkTiledChunkNotVisibleSystem(ecs_iter_t *it) {
    Position *positions = ecs_field(it, Position, 0);
    RectSize *rects = ecs_field(it, RectSize, 1);
    const SokolCamera *camera = ecs_singleton_get(it->world, SokolCamera);

    const float cam_start_x = camera->position.x;
    const float cam_end_x = camera->position.x + sapp_widthf();
    const float cam_start_y = camera->position.y;
    const float cam_end_y = camera->position.y + sapp_heightf();

    for (int i = 0; i < it->count; i++) {
        const float start_x = positions[i].x;
        const float end_x = positions[i].x + rects[i].width;
        const float start_y = positions[i].y;
        const float end_y = positions[i].y + rects[i].height;

        if (end_x < cam_start_x || start_x > cam_end_x ||
            end_y < cam_start_y || start_y > cam_end_y) {
            ecs_remove(it->world, it->entities[i], TiledChunkVisible);
        }
    }
}


void TiledImport(ecs_world_t *world) {
    ECS_MODULE(world, Tiled);
    ECS_COMPONENT_DEFINE(world, EcsTiledMap);
    ECS_COMPONENT_DEFINE(world, EcsTiledLayer);
    ECS_COMPONENT_DEFINE(world, EcsTiledTileset);
    ECS_TAG_DEFINE(world, EcsTile);
    ECS_TAG_DEFINE(world, TiledChunkVisible);
    ECS_TAG_DEFINE(world, EcsTiledChunk);

    ecs_struct(world, {
        .entity = ecs_id(EcsTiledMap),
        .members = {
            { .name = "x", .type = ecs_id(ecs_i32_t) },
            { .name = "y", .type = ecs_id(ecs_i32_t) }
        }
    });

    ecs_struct(world, {
        .entity = ecs_id(EcsTiledTileset),
        .members = {
            { .name = "firstgid", .type = ecs_id(ecs_i32_t) },
            { .name = "name", .type = ecs_id(ecs_string_t) },
            { .name = "image", .type = ecs_id(ecs_string_t) },
            { .name = "imagewidth", .type = ecs_id(ecs_i32_t) },
            { .name = "imageheight", .type = ecs_id(ecs_i32_t) },
            { .name = "tilewidth", .type = ecs_id(ecs_i32_t) },
            { .name = "tileheight", .type = ecs_id(ecs_i32_t) },
            { .name = "columns", .type = ecs_id(ecs_i32_t) },
            { .name = "tilecount", .type = ecs_id(ecs_i32_t) },
            { .name = "margin", .type = ecs_id(ecs_i32_t) },
            { .name = "spacing", .type = ecs_id(ecs_i32_t) }
        }
    });

    ecs_struct(world, {
        .entity = ecs_id(EcsTiledLayer),
        .members = {
            { .name = "x", .type = ecs_id(ecs_i32_t) },
            { .name = "y", .type = ecs_id(ecs_i32_t) }
        }
    });

    spawn_map(world,
        "World",
        (Position) {0, 0},
        "./assets/WeirdValley/Map.json",
        4
    );

    ECS_SYSTEM(world, MarkTiledChunkVisibleSystem, EcsPreUpdate,
        [in] transform.Position,
        [in] transform.RectSize,
        [none] !TiledChunkVisible,
        [none] EcsTiledChunk
    );
    ECS_SYSTEM(world, MarkTiledChunkNotVisibleSystem, EcsPreUpdate,
        [in] transform.Position,
        [in] transform.RectSize,
        [none] TiledChunkVisible,
        [none] EcsTiledChunk
    );

    ECS_SYSTEM(world, TilesRenderSystem, EcsOnUpdate,
        [in] transform.Position,
        [in] sokol.Image,
        [in] sokol.ImageRect,
        [in] sokol.ImageOffset,
        [in] sokol.ImageSize,
        [inout] sokol.RenderCtx(sokol.RenderCtx),
        [none] TiledChunkVisible(up ChildOf),
        [none] sokol.NoRender,
        [none] EcsTile
    );

    ecs_add_pair(world, ecs_lookup(world, "sokol.DrawImageSystem"), EcsDependsOn, TilesRenderSystem);
    ecs_add_pair(world, ecs_lookup(world, "sokol.DrawImageWithAtlas"), EcsDependsOn, TilesRenderSystem);
    ecs_add_pair(world, ecs_lookup(world, "sokol.DrawRectangleShapeSystem"), EcsDependsOn, TilesRenderSystem);
}
