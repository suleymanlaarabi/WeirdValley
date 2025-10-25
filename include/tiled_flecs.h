#ifndef TILED_FLECS_H
    #define TILED_FLECS_H
    #include <flecs.h>
    #include "tiled.h"
    #include "transform.h"

typedef struct {
    int height;
    int width;
} EcsTiledMap;

typedef struct {
    int x;
    int y;
} EcsTiledLayer;

typedef TiledChunk EcsTiledChunk;
typedef TiledTileset EcsTiledTileset;

extern ECS_COMPONENT_DECLARE(EcsTiledMap);
extern ECS_COMPONENT_DECLARE(EcsTiledLayer);
extern ECS_COMPONENT_DECLARE(EcsTiledChunk);
extern ECS_COMPONENT_DECLARE(EcsTiledTileset);
extern ECS_TAG_DECLARE(EcsTile);


void spawn_map(
    ecs_world_t *world,
    const char *map_name,
    Position map_position,
    const char *map_path,
    float scale
);
void TiledImport(ecs_world_t *world);

#endif
