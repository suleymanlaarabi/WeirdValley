#include "sokol_flecs.h"
#include "flecs.h"
#include "tiled_flecs.h"
#include "tiled.h"
#include "transform.h"
#include <stdio.h>

#define TILE_SIZE 16

typedef struct {
    ecs_entity_t image;
    int firstgid;
    int endgid;
    int tiles_per_row;
} TilesetImage;

static void spawn_chunk(
    ecs_world_t *world,
    ecs_entity_t chunk_entity,
    Position chunk_position,
    TiledChunk *chunk,
    TilesetImage *tileset_images,
    int tileset_count,
    float scale
) {
    char entity_name[20];
    int tile_counter = 0;
    for (int row = 0; row < chunk->height; row++) {
        for (int column = 0; column < chunk->width; column++) {
            tile_counter++;
            int data_index = row * chunk->width + column;
            int tile_gid = chunk->data[data_index];
            if (tile_gid == 0) continue;
            TilesetImage selected_tileset = {0};
            for (int tileset_index = 0; tileset_index < tileset_count; tileset_index++) {
                if (tile_gid >= tileset_images[tileset_index].firstgid && tile_gid < tileset_images[tileset_index].endgid) {
                    selected_tileset = tileset_images[tileset_index];
                    break;
                }
            }

            int local_tile_id = tile_gid - selected_tileset.firstgid;
            int source_x = (local_tile_id % selected_tileset.tiles_per_row) * TILE_SIZE;
            int source_y = (local_tile_id / selected_tileset.tiles_per_row) * TILE_SIZE;

            sprintf(entity_name, "Tile(%d)", tile_counter);
            ecs_entity_t tile_entity = ecs_entity(world, {
                .name = entity_name,
                .parent = chunk_entity,
                .add = ecs_ids(EcsTile, NoRender),
                .set = ecs_values(
                    ecs_value(Position, {
                        .x = (column + chunk_position.x) * TILE_SIZE * scale,
                        .y = (row + chunk_position.y) * TILE_SIZE * scale
                    }),
                    ecs_value(ImageRect, {
                        .height = TILE_SIZE,
                        .width = TILE_SIZE
                    }),
                    ecs_value(ImageSize, {
                        .height = TILE_SIZE * scale,
                        .width = TILE_SIZE * scale
                    }),
                    ecs_value(ImageOffset, {
                        .x = source_x,
                        .y = source_y
                    })
                )
            });
            ecs_set_image(world, selected_tileset.image, tile_entity);
        }
    }
}

#define CHUNK_DEBUG

static void spawn_layer(

    ecs_world_t *world,
    Position layer_position,
    ecs_entity_t layer_entity,
    TiledLayer *layer,
    TilesetImage *tileset_images,
    int tileset_count,
    float scale
) {
    char entity_name[20];
    for (int chunk_index = 0; chunk_index < layer->chunk_count; chunk_index++) {
        TiledChunk *current_chunk = &layer->chunks[chunk_index];
        sprintf(entity_name, "Chunk(%d)", chunk_index);

        Position chunk_position_tiles = {
            .x = current_chunk->x + layer_position.x,
            .y = current_chunk->y + layer_position.y
        };

        Position chunk_world_position = {
            .x = chunk_position_tiles.x * TILE_SIZE * scale,
            .y = chunk_position_tiles.y * TILE_SIZE * scale
        };

        ecs_entity_t chunk_entity = ecs_entity(world, {
            .name = entity_name,
            .parent = layer_entity,
            .set = ecs_values(
                ecs_value_ptr(EcsTiledChunk, current_chunk),
                ecs_value_ptr(Position, &chunk_world_position)
            )
        });
        #ifdef CHUNK_DEBUG
        ecs_set(world, chunk_entity, RectangleShape, {10, 10});
        ecs_set(world, chunk_entity, Color, { .r = 1, .g = 0, .b = 0, .a = 1});
        #endif
        spawn_chunk(world, chunk_entity, chunk_position_tiles, current_chunk, tileset_images, tileset_count, scale);
    }
}

void spawn_map(
    ecs_world_t *world,
    const char *map_name,
    Position map_position,
    const char *map_path,
    float scale
) {
    TiledMap *loaded_map = load_tiled_map(map_path);
    ecs_entity_t map_entity = ecs_entity(world, {
        .name = map_name,
        .set = ecs_values(
            ecs_value(EcsTiledMap, {
                .height = loaded_map->height,
                .width = loaded_map->width,
            }),
            ecs_value_ptr(Position, &map_position)
        )
    });

    TilesetImage tileset_images[loaded_map->tileset_count + 1];

    ecs_entity_t tilesets_container = ecs_entity(world, { .name = "Tilesets", .parent = map_entity });
    char tileset_name[20];
    for (int tileset_index = 0; tileset_index < loaded_map->tileset_count; tileset_index++) {
        sprintf(tileset_name, "Tileset(%d)", tileset_index);
        ecs_entity(world, {
            .name = tileset_name,
            .parent = tilesets_container,
            .set = ecs_values(
                ecs_value_ptr(EcsTiledTileset, &loaded_map->tilesets[tileset_index])
            )
        });
        char *image_source = strdup(loaded_map->tilesets[tileset_index].image);
        sg_image sokol_image = load_image(image_source);
        ecs_entity_t image_entity = ecs_entity(world, {
            .name = strdup(loaded_map->tilesets[tileset_index].name),
            .add = ecs_ids(EcsPrefab),
            .set = ecs_values(
                ecs_value(Image, { sokol_image.id }),
                ecs_value(ImageSource, { .src = image_source })
            )
        });

        tileset_images[tileset_index] = (TilesetImage) {
            .image = image_entity,
            .firstgid = loaded_map->tilesets[tileset_index].firstgid,
            .endgid = loaded_map->tilesets[tileset_index].firstgid + loaded_map->tilesets[tileset_index].tilecount,
            .tiles_per_row = loaded_map->tilesets[tileset_index].columns
        };
    }

    tileset_images[loaded_map->tileset_count] = (TilesetImage) {0};

    ecs_entity_t layers_container = ecs_entity(world, { .name = "Layers", .parent = map_entity });
    char layer_name[20];
    for (int layer_index = 0; layer_index < loaded_map->layer_count; layer_index++) {
        sprintf(layer_name, "Layer(%d)", layer_index);
        Position layer_world_position = {
            .x = loaded_map->layers[layer_index].x + map_position.x,
            .y = loaded_map->layers[layer_index].y + map_position.y
        };
        ecs_entity_t layer_entity = ecs_entity(world, {
            .name = layer_name,
            .parent = layers_container,
            .set = ecs_values(
                ecs_value(EcsTiledLayer, {
                    .x = loaded_map->layers[layer_index].x,
                    .y = loaded_map->layers[layer_index].y
                }),
                ecs_value_ptr(Position, &layer_world_position)
            )
        });
        spawn_layer(world,
            layer_world_position,
            layer_entity,
            &loaded_map->layers[layer_index],
            tileset_images,
            loaded_map->tileset_count,
            scale
        );
    }
}
