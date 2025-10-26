#ifndef TILED_H
#define TILED_H

typedef struct {
    int *data;
    int height;
    int width;
    int x;
    int y;
} TiledChunk;

typedef struct {
    TiledChunk *chunks;
    int chunk_count;
    int height;
    int id;
    char *name;
    double opacity;
    int startx;
    int starty;
    char *type;
    int visible;
    int width;
    int x;
    int y;
} TiledLayer;

typedef struct {
    int firstgid;
    char *name;
    char *image;
    int imagewidth;
    int imageheight;
    int tilewidth;
    int tileheight;
    int columns;
    int tilecount;
    int margin;
    int spacing;
} TiledTileset;

typedef struct {
    int height;
    TiledLayer *layers;
    int layer_count;
    char *renderorder;
    int tileheight;
    TiledTileset *tilesets;
    int tileset_count;
    int tilewidth;
    char *type;
    int width;
} TiledMap;

TiledMap* load_tiled_map(const char *path);
void free_tiled_map(TiledMap *map);
void print_tiled_map(TiledMap *map);

#endif
