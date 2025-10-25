

#include "tiled.h"
#include "json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_tiled_map(TiledMap *map) {
    printf("Size: %dx%d tiles\n", map->width, map->height);
    printf("Tile size: %dx%d pixels\n", map->tilewidth, map->tileheight);
    printf("Infinite: %s\n", map->infinite ? "true" : "false");
    printf("Render order: %s\n", map->renderorder);
    printf("Next layer ID: %d\n", map->nextlayerid);
    printf("Next object ID: %d\n", map->nextobjectid);

    printf("\n=== LAYERS (%d) ===\n", map->layer_count);
    for (int i = 0; i < map->layer_count; i++) {
        TiledLayer *layer = &map->layers[i];
        printf("\nLayer %d: \"%s\"\n", layer->id, layer->name);
        printf("  Type: %s\n", layer->type);
        printf("  Size: %dx%d\n", layer->width, layer->height);
        printf("  Position: (%d, %d)\n", layer->x, layer->y);
        printf("  Start: (%d, %d)\n", layer->startx, layer->starty);
        printf("  Opacity: %.2f\n", layer->opacity);
        printf("  Visible: %s\n", layer->visible ? "true" : "false");

        printf("  Chunks: %d\n", layer->chunk_count);
        for (int j = 0; j < layer->chunk_count; j++) {
            TiledChunk *chunk = &layer->chunks[j];
            printf("    Chunk %d: pos(%d,%d) size(%dx%d)\n",
                   j, chunk->x, chunk->y, chunk->width, chunk->height);

            int non_zero_tiles = 0;
            for (int k = 0; k < chunk->width * chunk->height; k++) {
                if (chunk->data[k] != 0) non_zero_tiles++;
            }
            printf("      Non-zero tiles: %d/%d\n", non_zero_tiles, chunk->width * chunk->height);

            printf("      Data: ");
            for (int k = 0; k < 10 && k < chunk->width * chunk->height; k++) {
                printf("%d ", chunk->data[k]);
            }
            if (chunk->width * chunk->height > 10) printf("...");
            printf("\n");
        }
    }

    printf("\n=== TILESETS (%d) ===\n", map->tileset_count);
    for (int i = 0; i < map->tileset_count; i++) {
        TiledTileset *tileset = &map->tilesets[i];
        printf("Tileset %d:\n", i);
        printf("  First GID: %d\n", tileset->firstgid);
        printf("  Name: %s\n", tileset->name);
        printf("  Image: %s\n", tileset->image);
        printf("  Image size: %dx%d\n", tileset->imagewidth, tileset->imageheight);
        printf("  Tile size: %dx%d\n", tileset->tilewidth, tileset->tileheight);
        printf("  Columns: %d\n", tileset->columns);
        printf("  Tile count: %d\n", tileset->tilecount);
        printf("  Margin: %d, Spacing: %d\n", tileset->margin, tileset->spacing);
    }
    printf("\n=================\n");
}

static TiledChunk parse_chunk(cJSON *chunk_json) {
    TiledChunk chunk = {0};

    cJSON *data = cJSON_GetObjectItem(chunk_json, "data");
    cJSON *height = cJSON_GetObjectItem(chunk_json, "height");
    cJSON *width = cJSON_GetObjectItem(chunk_json, "width");
    cJSON *x = cJSON_GetObjectItem(chunk_json, "x");
    cJSON *y = cJSON_GetObjectItem(chunk_json, "y");

    chunk.height = height->valueint;
    chunk.width = width->valueint;
    chunk.x = x->valueint;
    chunk.y = y->valueint;

    int data_size = cJSON_GetArraySize(data);
    chunk.data = malloc(sizeof(int) * data_size);

    for (int i = 0; i < data_size; i++) {
        cJSON *item = cJSON_GetArrayItem(data, i);
        chunk.data[i] = item->valueint;
    }

    return chunk;
}

static TiledLayer parse_layer(cJSON *layer_json) {
    TiledLayer layer = {0};

    cJSON *chunks = cJSON_GetObjectItem(layer_json, "chunks");
    cJSON *height = cJSON_GetObjectItem(layer_json, "height");
    cJSON *id = cJSON_GetObjectItem(layer_json, "id");
    cJSON *name = cJSON_GetObjectItem(layer_json, "name");
    cJSON *opacity = cJSON_GetObjectItem(layer_json, "opacity");
    cJSON *startx = cJSON_GetObjectItem(layer_json, "startx");
    cJSON *starty = cJSON_GetObjectItem(layer_json, "starty");
    cJSON *type = cJSON_GetObjectItem(layer_json, "type");
    cJSON *visible = cJSON_GetObjectItem(layer_json, "visible");
    cJSON *width = cJSON_GetObjectItem(layer_json, "width");
    cJSON *x = cJSON_GetObjectItem(layer_json, "x");
    cJSON *y = cJSON_GetObjectItem(layer_json, "y");

    layer.height = height->valueint;
    layer.id = id->valueint;
    layer.name = strdup(name->valuestring);
    layer.opacity = opacity->valuedouble;
    layer.startx = startx->valueint;
    layer.starty = starty->valueint;
    layer.type = strdup(type->valuestring);
    layer.visible = visible->valueint;
    layer.width = width->valueint;
    layer.x = x->valueint;
    layer.y = y->valueint;

    if (chunks) {
        layer.chunk_count = cJSON_GetArraySize(chunks);
        layer.chunks = malloc(sizeof(TiledChunk) * layer.chunk_count);

        for (int i = 0; i < layer.chunk_count; i++) {
            cJSON *chunk = cJSON_GetArrayItem(chunks, i);
            layer.chunks[i] = parse_chunk(chunk);
        }
    }

    return layer;
}

static TiledTileset parse_tileset(cJSON *tileset_json) {
    TiledTileset tileset = {0};

    cJSON *firstgid = cJSON_GetObjectItem(tileset_json, "firstgid");

    tileset.firstgid = firstgid->valueint;

    cJSON *name = cJSON_GetObjectItem(tileset_json, "name");
    cJSON *image = cJSON_GetObjectItem(tileset_json, "image");
    cJSON *imagewidth = cJSON_GetObjectItem(tileset_json, "imagewidth");
    cJSON *imageheight = cJSON_GetObjectItem(tileset_json, "imageheight");
    cJSON *tilewidth = cJSON_GetObjectItem(tileset_json, "tilewidth");
    cJSON *tileheight = cJSON_GetObjectItem(tileset_json, "tileheight");
    cJSON *columns = cJSON_GetObjectItem(tileset_json, "columns");
    cJSON *tilecount = cJSON_GetObjectItem(tileset_json, "tilecount");
    cJSON *margin = cJSON_GetObjectItem(tileset_json, "margin");
    cJSON *spacing = cJSON_GetObjectItem(tileset_json, "spacing");

    if (name) tileset.name = strdup(name->valuestring);
    if (image) tileset.image = strdup(image->valuestring);
    if (imagewidth) tileset.imagewidth = imagewidth->valueint;
    if (imageheight) tileset.imageheight = imageheight->valueint;
    if (tilewidth) tileset.tilewidth = tilewidth->valueint;
    if (tileheight) tileset.tileheight = tileheight->valueint;
    if (columns) tileset.columns = columns->valueint;
    if (tilecount) tileset.tilecount = tilecount->valueint;
    if (margin) tileset.margin = margin->valueint;
    if (spacing) tileset.spacing = spacing->valueint;

    return tileset;
}

TiledMap* load_tiled_map(const char *path) {
    FILE *file = fopen(path, "r");
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(buffer);
    free(buffer);

    TiledMap *map = malloc(sizeof(TiledMap));
    memset(map, 0, sizeof(TiledMap));

    cJSON *height = cJSON_GetObjectItem(json, "height");
    cJSON *infinite = cJSON_GetObjectItem(json, "infinite");
    cJSON *layers = cJSON_GetObjectItem(json, "layers");
    cJSON *nextlayerid = cJSON_GetObjectItem(json, "nextlayerid");
    cJSON *nextobjectid = cJSON_GetObjectItem(json, "nextobjectid");
    cJSON *renderorder = cJSON_GetObjectItem(json, "renderorder");
    cJSON *tileheight = cJSON_GetObjectItem(json, "tileheight");
    cJSON *tilesets = cJSON_GetObjectItem(json, "tilesets");
    cJSON *tilewidth = cJSON_GetObjectItem(json, "tilewidth");
    cJSON *type = cJSON_GetObjectItem(json, "type");
    cJSON *width = cJSON_GetObjectItem(json, "width");

    map->height = height->valueint;
    map->infinite = cJSON_IsTrue(infinite) ? 1 : 0;
    map->nextlayerid = nextlayerid->valueint;
    map->nextobjectid = nextobjectid->valueint;
    map->renderorder = strdup(renderorder->valuestring);
    map->tileheight = tileheight->valueint;
    map->tilewidth = tilewidth->valueint;
    map->type = strdup(type->valuestring);
    map->width = width->valueint;

    map->layer_count = cJSON_GetArraySize(layers);
    map->layers = malloc(sizeof(TiledLayer) * map->layer_count);
    for (int i = 0; i < map->layer_count; i++) {
        cJSON *layer = cJSON_GetArrayItem(layers, i);
        map->layers[i] = parse_layer(layer);
    }

    map->tileset_count = cJSON_GetArraySize(tilesets);
    map->tilesets = malloc(sizeof(TiledTileset) * map->tileset_count);
    for (int i = 0; i < map->tileset_count; i++) {
        cJSON *tileset = cJSON_GetArrayItem(tilesets, i);
        map->tilesets[i] = parse_tileset(tileset);
    }

    cJSON_Delete(json);

    return map;
}

void free_tiled_map(TiledMap *map) {
    for (int i = 0; i < map->layer_count; i++) {
        TiledLayer *layer = &map->layers[i];
        for (int j = 0; j < layer->chunk_count; j++) {
            free(layer->chunks[j].data);
        }
        free(layer->chunks);
        free(layer->name);
        free(layer->type);
    }
    free(map->layers);

    for (int i = 0; i < map->tileset_count; i++) {
        if (map->tilesets[i].name) free(map->tilesets[i].name);
        if (map->tilesets[i].image) free(map->tilesets[i].image);
    }
    free(map->tilesets);

    free(map->renderorder);
    free(map->type);
    free(map);
}
