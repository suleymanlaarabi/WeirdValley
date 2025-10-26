#include "transform.h"
#include "sokol_flecs.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "sokol_gfx.h"
#include "sokol_gp.h"
#include "sokol_app.h"
#include "sokol_glue.h"
#include "sokol_log.h"

ECS_COMPONENT_DECLARE(RectangleShape);
ECS_COMPONENT_DECLARE(Color);
ECS_COMPONENT_DECLARE(Image);
ECS_COMPONENT_DECLARE(ImageSource);
ECS_COMPONENT_DECLARE(ImageRect);
ECS_COMPONENT_DECLARE(ImageOffset);
ECS_COMPONENT_DECLARE(ImageSize);
ECS_COMPONENT_DECLARE(SokolCamera);
ECS_TAG_DECLARE(NoRender);
ECS_TAG_DECLARE(RenderCtx);

void DrawRectangleShapeSystem(ecs_iter_t *it) {
    const SokolCamera *camera = ecs_singleton_get(it->world, SokolCamera);
    Position *positions = ecs_field(it, Position, 0);
    RectangleShape *shapes = ecs_field(it, RectangleShape, 1);
    Color *colors = ecs_field(it, Color, 2);

    for (int i = 0; i < it->count; i++) {
        Position pos = positions[i];
        RectangleShape shape = shapes[i];
        Color color = colors[i];

        sgp_set_color(color.r, color.g, color.b, color.a);
        sgp_draw_filled_rect(pos.x - camera->position.x, pos.y - camera->position.y, shape.width, shape.height);
    }
}

void DrawImageSystem(ecs_iter_t *it) {
    sgp_set_image(0, *ecs_field(it, Image, 1));
    sgp_draw_filled_rects_from_pos_size(
        (sgp_vec2 *) ecs_field(it, Position, 0),
        (sgp_vec2 *) ecs_field(it, ImageSize, 2),
        it->count
    );
}

void DrawImageWithAtlas(ecs_iter_t *it) {
    const SokolCamera *camera = ecs_singleton_get(it->world, SokolCamera);
    const Position *positions = ecs_field(it, Position, 0);
    const Image *images = ecs_field(it, Image, 1);
    const ImageRect *rects = ecs_field(it, ImageRect, 2);
    const ImageOffset *offsets = ecs_field(it, ImageOffset, 3);
    const ImageSize *sizes = ecs_field(it, ImageSize, 4);

    sgp_set_image(0, images[0]);
    sgp_draw_textured_rects_sep(0, -camera->position.x, -camera->position.y, (sgp_vec2 *) positions, (sgp_vec2 *) sizes, offsets, (sgp_vec2 *) rects, it->count);
}

void EndRenderSystem(ecs_iter_t *_) {
    sg_pass pass = {.swapchain = sglue_swapchain()};
    sg_begin_pass(&pass);
    sgp_flush();
    sgp_end();
    sg_end_pass();
    sg_commit();
}

void SokolImport(ecs_world_t *world) {
    ECS_MODULE(world, Sokol);
    ECS_IMPORT(world, Transform);

    ECS_TAG_DEFINE(world, NoRender);
    ECS_TAG_DEFINE(world, RenderCtx);
    ECS_COMPONENT_DEFINE(world, RectangleShape);
    ECS_COMPONENT_DEFINE(world, ImageRect);
    ECS_COMPONENT_DEFINE(world, ImageOffset);
    ECS_COMPONENT_DEFINE(world, ImageSize);
    ECS_COMPONENT_DEFINE(world, Color);
    ECS_COMPONENT_DEFINE(world, Image);
    ECS_COMPONENT_DEFINE(world, ImageSource);
    ECS_COMPONENT_DEFINE(world, SokolCamera);

    sg_desc sgdesc = {
        .environment = sglue_environment(),
        .logger.func = slog_func,
    };
    sg_setup(&sgdesc);
    sgp_desc sgpdesc = {0};
    sgp_setup(&sgpdesc);


    ecs_singleton_set(world, SokolCamera, {
        .zoom = 1.0f,
        .position = {0, 0}
    });

    ecs_add_id(world, RenderCtx, RenderCtx);

    #define REGISTER_SIZE(component) \
        ecs_struct(world, { \
            .entity = ecs_id(component), \
            .members = { \
                { .name = "width", .type = ecs_id(ecs_f32_t) }, \
                { .name = "height", .type = ecs_id(ecs_f32_t) } \
            } \
        });

    REGISTER_SIZE(RectangleShape);
    REGISTER_SIZE(ImageSize);
    REGISTER_SIZE(ImageRect);
    REGISTER_VECTOR2_COMPONENT(world, ImageOffset);

    ecs_struct(world, {
        .entity = ecs_id(Color),
        .members = {
            { .name = "r", .type = ecs_id(ecs_f32_t) },
            { .name = "g", .type = ecs_id(ecs_f32_t) },
            { .name = "b", .type = ecs_id(ecs_f32_t) },
            { .name = "a", .type = ecs_id(ecs_f32_t) }
        }
    });
    ecs_struct(world, {
        .entity = ecs_id(ImageSource),
        .members = {
            { .name = "src", .type = ecs_id(ecs_string_t) },
        }
    });

    ecs_primitive(world, {
        .entity = ecs_id(Image),
        .kind = EcsU32
    });
    ECS_SYSTEM(world, DrawRectangleShapeSystem, EcsOnUpdate,
        [in] transform.Position,
        [in] RectangleShape,
        [in] Color,
        [inout] RenderCtx(RenderCtx)
    );
    ECS_SYSTEM(world, DrawImageSystem, EcsOnUpdate,
        [in] transform.Position,
        [in] Image,
        [in] ImageSize,
        [inout] RenderCtx(RenderCtx),
        [none] !ImageOffset,
        [none] !ImageRect,
    );
    ECS_SYSTEM(world, DrawImageWithAtlas, EcsOnUpdate,
        [in] transform.Position,
        [in] Image,
        [in] ImageRect,
        [in] ImageOffset,
        [in] ImageSize,
        [inout] RenderCtx(RenderCtx),
        [none] !NoRender
    );
    ECS_SYSTEM(world, EndRenderSystem, EcsPostUpdate, [inout] RenderCtx(RenderCtx));

}
