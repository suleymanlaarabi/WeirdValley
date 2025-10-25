#ifndef SOKOL_FLECS_H
    #define SOKOL_FLECS_H
    #include <stdint.h>
    #include "flecs.h"
    #include "sokol_gfx.h"
    #include "sokol_gp.h"
    #include "sokol_app.h"
    #include "sokol_glue.h"
    #include "sokol_log.h"
    #include "stb_image.h"

    #define ecs_image(world, entity_name, path) \
        sg_image entity_name##_img = load_image(path); \
        ecs_entity_t entity_name = ecs_entity(world, { \
            .name = #entity_name, \
            .add = ecs_ids(EcsPrefab) \
        }); \
        ecs_set_ptr(world, entity_name, Image, &entity_name##_img);


    #define ecs_set_image(world, image_name, entity) \
        ecs_add_pair(world, entity, EcsIsA, image_name);

typedef struct {
    float width;
    float height;
} ImageSize, ImageRect, RectangleShape;

typedef struct {
    bool keys[348];
} InputState;

typedef struct {
    float zoom;
    sgp_vec2 position;
} SokolCamera;

extern InputState input_state;

typedef sgp_vec2 ImageOffset;

typedef struct {
    float r;
    float g;
    float b;
    float a;
} Color;

typedef sg_image Image;

typedef struct {
    char *src;
} ImageSource;

extern ECS_TAG_DECLARE(NoRender);
extern ECS_TAG_DECLARE(RenderCtx);
extern ECS_COMPONENT_DECLARE(ImageRect);
extern ECS_COMPONENT_DECLARE(ImageOffset);
extern ECS_COMPONENT_DECLARE(ImageSize);
extern ECS_COMPONENT_DECLARE(RectangleShape);
extern ECS_COMPONENT_DECLARE(Color);
extern ECS_COMPONENT_DECLARE(Image);
extern ECS_COMPONENT_DECLARE(ImageSource);
extern ECS_COMPONENT_DECLARE(SokolCamera);

sg_image load_image(const char *filename);
void SokolImport(ecs_world_t *world);

#endif
