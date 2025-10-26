#include "tiled.h"
#include "jsmn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    return (tok->type == JSMN_STRING &&
            (int)strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0);
}

static const char *json_strdup(const char *json, jsmntok_t *tok) {
    int len = tok->end - tok->start;
    char *out = malloc(len + 1);
    memcpy(out, json + tok->start, len);
    out[len] = '\0';
    return out;
}

static int json_int(const char *json, jsmntok_t *tok) {
    char buf[32];
    int len = tok->end - tok->start;
    if (len >= 32) len = 31;
    memcpy(buf, json + tok->start, len);
    buf[len] = 0;
    return atoi(buf);
}

static double json_double(const char *json, jsmntok_t *tok) {
    char buf[64];
    int len = tok->end - tok->start;
    if (len >= 63) len = 63;
    memcpy(buf, json + tok->start, len);
    buf[len] = 0;
    return atof(buf);
}

static TiledChunk parse_chunk(const char *json, jsmntok_t *tokens, int *i) {
    TiledChunk chunk = {0};
    int start = *i, obj_size = tokens[start].size;
    (*i)++;

    for (int k = 0; k < obj_size; k++) {
        jsmntok_t *key = &tokens[(*i)++];
        if (jsoneq(json, key, "data")) {
            jsmntok_t *arr = &tokens[*i];
            int n = arr->size;
            (*i)++;
            chunk.data = malloc(sizeof(int) * n);
            for (int j = 0; j < n; j++) {
                chunk.data[j] = json_int(json, &tokens[(*i)++]);
            }
        } else if (jsoneq(json, key, "height")) chunk.height = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "width")) chunk.width = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "x")) chunk.x = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "y")) chunk.y = json_int(json, &tokens[(*i)++]);
        else (*i)++;
    }
    return chunk;
}

static TiledLayer parse_layer(const char *json, jsmntok_t *tokens, int *i) {
    TiledLayer layer = {0};
    int start = *i, obj_size = tokens[start].size;
    (*i)++;

    for (int k = 0; k < obj_size; k++) {
        jsmntok_t *key = &tokens[(*i)++];
        if (jsoneq(json, key, "chunks")) {
            jsmntok_t *arr = &tokens[*i];
            int n = arr->size;
            (*i)++;
            layer.chunk_count = n;
            layer.chunks = malloc(sizeof(TiledChunk) * n);
            for (int j = 0; j < n; j++) {
                layer.chunks[j] = parse_chunk(json, tokens, i);
            }
        } else if (jsoneq(json, key, "id")) layer.id = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "name")) layer.name = (char *) json_strdup(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "type")) layer.type = (char *) json_strdup(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "width")) layer.width = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "height")) layer.height = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "x")) layer.x = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "y")) layer.y = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "opacity")) layer.opacity = json_double(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "visible")) layer.visible = json_int(json, &tokens[(*i)++]);
        else (*i)++;
    }
    return layer;
}

static TiledTileset parse_tileset(const char *json, jsmntok_t *tokens, int *i) {
    TiledTileset t = {0};
    int start = *i, obj_size = tokens[start].size;
    (*i)++;
    for (int k = 0; k < obj_size; k++) {
        jsmntok_t *key = &tokens[(*i)++];
        if (jsoneq(json, key, "firstgid")) t.firstgid = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "name")) t.name = (char *) json_strdup(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "image")) t.image = (char *) json_strdup(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "imagewidth")) t.imagewidth = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "imageheight")) t.imageheight = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "tilewidth")) t.tilewidth = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "tileheight")) t.tileheight = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "columns")) t.columns = json_int(json, &tokens[(*i)++]);
        else if (jsoneq(json, key, "tilecount")) t.tilecount = json_int(json, &tokens[(*i)++]);
        else (*i)++;
    }
    return t;
}

TiledMap *load_tiled_map(const char *path) {
    FILE *f = fopen(path, "r");
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *json = malloc(len + 1);
    fread(json, 1, len, f);
    json[len] = '\0';
    fclose(f);

    jsmn_parser p;
    jsmn_init(&p);
    int tokcount = 8192;
    jsmntok_t *tokens = calloc(tokcount, sizeof(jsmntok_t));
    int r = jsmn_parse(&p, json, strlen(json), tokens, tokcount);
    if (r < 0) {
        free(tokens);
        free(json);
        return NULL;
    }

    TiledMap *map = calloc(1, sizeof(TiledMap));
    int i = 1;

    for (int k = 0; k < tokens[0].size; k++) {
        jsmntok_t *key = &tokens[i++];
        if (jsoneq(json, key, "height")) map->height = json_int(json, &tokens[i++]);
        else if (jsoneq(json, key, "width")) map->width = json_int(json, &tokens[i++]);
        else if (jsoneq(json, key, "tileheight")) map->tileheight = json_int(json, &tokens[i++]);
        else if (jsoneq(json, key, "tilewidth")) map->tilewidth = json_int(json, &tokens[i++]);
        else if (jsoneq(json, key, "renderorder")) map->renderorder = (char *) json_strdup(json, &tokens[i++]);
        else if (jsoneq(json, key, "type")) map->type = (char *) json_strdup(json, &tokens[i++]);
        else if (jsoneq(json, key, "layers")) {
            jsmntok_t *arr = &tokens[i];
            int n = arr->size;
            i++;
            map->layer_count = n;
            map->layers = malloc(sizeof(TiledLayer) * n);
            for (int j = 0; j < n; j++) map->layers[j] = parse_layer(json, tokens, &i);
        } else if (jsoneq(json, key, "tilesets")) {
            jsmntok_t *arr = &tokens[i];
            int n = arr->size;
            i++;
            map->tileset_count = n;
            map->tilesets = malloc(sizeof(TiledTileset) * n);
            for (int j = 0; j < n; j++) map->tilesets[j] = parse_tileset(json, tokens, &i);
        } else i++;
    }

    free(tokens);
    free(json);
    return map;
}


void print_tiled_map(TiledMap *map) {
    printf("Size: %dx%d tiles\n", map->width, map->height);
    printf("Tile size: %dx%d pixels\n", map->tilewidth, map->tileheight);
    printf("Render order: %s\n", map->renderorder);

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
