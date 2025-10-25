#include "tiled.h"
#include "sokol_flecs.h"
#include "tiled_flecs.h"
#include "transform.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>

ECS_COMPONENT_DECLARE(EcsTiledMap);
ECS_COMPONENT_DECLARE(EcsTiledLayer);
ECS_COMPONENT_DECLARE(EcsTiledChunk);
ECS_COMPONENT_DECLARE(EcsTiledTileset);
ECS_TAG_DECLARE(EcsTile);
ECS_TAG_DECLARE(TiledVisibleChunk);

void TilesRenderSystem(ecs_iter_t *it) {
    const Position *positions = ecs_field(it, Position, 0);
    const Image *images = ecs_field(it, Image, 1);
    const ImageRect *rects = ecs_field(it, ImageRect, 2);
    const ImageOffset *offsets = ecs_field(it, ImageOffset, 3);
    const ImageSize *sizes = ecs_field(it, ImageSize, 4);

    sgp_set_color(1, 1, 1, 1);
    sgp_set_image(0, images[0]);
    sgp_draw_textured_rects_sep(0, positions, (sgp_vec2 *) sizes, offsets, (sgp_vec2 *) rects, it->count);
    sgp_reset_image(0);
}

void MarkTiledChunVisiblekSystem(ecs_iter_t *it) {
    UNUSED Position *positions = ecs_field(it, Position, 0);
}

void MarkTiledChunNotVisiblekSystem(ecs_iter_t *it) {
    UNUSED Position *positions = ecs_field(it, Position, 0);
}

void TiledImport(ecs_world_t *world) {
    ECS_MODULE(world, Tiled);
    ECS_COMPONENT_DEFINE(world, EcsTiledMap);
    ECS_COMPONENT_DEFINE(world, EcsTiledLayer);
    ECS_COMPONENT_DEFINE(world, EcsTiledChunk);
    ECS_COMPONENT_DEFINE(world, EcsTiledTileset);
    ECS_TAG_DEFINE(world, EcsTile);
    ECS_TAG_DEFINE(world, TiledVisibleChunk);

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

    ecs_struct(world, {
        .entity = ecs_id(EcsTiledChunk),
        .members = {
            { .name = "array", .type = ecs_id(ecs_uptr_t) },
            { .name = "height", .type = ecs_id(ecs_i32_t) },
            { .name = "width", .type = ecs_id(ecs_i32_t) },
            { .name = "x", .type = ecs_id(ecs_i32_t) },
            { .name = "y", .type = ecs_id(ecs_i32_t) }
        }
    });

    spawn_map(world,
        "World",
        (Position) {100, 55},
        "./assets/WeirdValley/Map.json",
        1
    );

    ECS_SYSTEM(world, MarkTiledChunVisiblekSystem, EcsPreUpdate,
        [in] transform.Position,
        [none] !TiledVisibleChunk,
        [none] EcsTiledChunk
    );
    ECS_SYSTEM(world, MarkTiledChunNotVisiblekSystem, EcsPreUpdate,
        [in] transform.Position,
        [none] TiledVisibleChunk,
        [none] EcsTiledChunk
    );

    ECS_SYSTEM(world, TilesRenderSystem, EcsOnUpdate,
        [in] transform.Position,
        [in] sokol.Image,
        [in] sokol.ImageRect,
        [in] sokol.ImageOffset,
        [in] sokol.ImageSize,
        [inout] sokol.RenderCtx(sokol.RenderCtx),
        [none] sokol.NoRender,
        [none] EcsTile
    );

    ecs_add_pair(world, ecs_lookup(world, "sokol.DrawImageSystem"), EcsDependsOn, TilesRenderSystem);
    ecs_add_pair(world, ecs_lookup(world, "sokol.DrawImageWithAtlas"), EcsDependsOn, TilesRenderSystem);
    ecs_add_pair(world, ecs_lookup(world, "sokol.DrawRectangleShapeSystem"), EcsDependsOn, TilesRenderSystem);
}
