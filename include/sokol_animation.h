#ifndef SOKOL_ANIMATION_H
    #define SOKOL_ANIMATION_H
    #include <flecs.h>


typedef struct {
    int index;
    int cols;
    int rows;
} SpriteSheetAtlas;

typedef struct {
    int start;
    int end;
} PlayingAnimation;

typedef struct {
    float current_time;
    float delay;
} SpriteSheetAnimationTimer;

extern ECS_COMPONENT_DECLARE(SpriteSheetAtlas);
extern ECS_COMPONENT_DECLARE(PlayingAnimation);
extern ECS_COMPONENT_DECLARE(SpriteSheetAnimationTimer);

void SokolAnimationImport(ecs_world_t *world);

#endif
