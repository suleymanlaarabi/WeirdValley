#include "sokol_flecs.h"
#include "utils.h"
#include <math.h>

sg_image load_image(const char *filename) {
    int width, height, channels;
    uint8_t* data = stbi_load(filename, &width, &height, &channels, 4);
    sg_image img = {SG_INVALID_ID};
    if (!data) {
        return img;
    }

    sg_image_desc image_desc = {
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .width = width,
        .height = height,
        .data.subimage = { { { .ptr = data, .size = (size_t)(width * height * 4) } } }
    };
    img = sg_make_image(&image_desc);
    stbi_image_free(data);
    return img;
}
