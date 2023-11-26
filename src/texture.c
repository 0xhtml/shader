#include "texture.h"

#include <png.h>
#include <stdlib.h>

struct image {
    int width;
    int height;
    png_bytep buffer;
};

struct image loadPNG(const char* filename) {
    png_image image = { .version = PNG_IMAGE_VERSION };

    if (!png_image_begin_read_from_file(&image, filename)) {
        printf("%s\n", image.message);
        exit(1);
    }

    image.format = PNG_FORMAT_RGBA;

    png_bytep buffer = malloc(PNG_IMAGE_SIZE(image));
    if (buffer == NULL) {
        printf("image too big\n");
        exit(1);
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
        printf("%s\n", image.message);
        free(buffer);
        exit(1);
    }
    png_image_free(&image);

    uint32_t *buffer32 = (uint32_t *) buffer;
    for (int y = 0; y < image.height / 2; ++y) {
        for (int x = 0; x < image.width; ++x) {
            uint32_t h = buffer32[y * image.width + x];
            uint32_t t = buffer32[(image.height - 1 - y) * image.width + x];
            buffer32[y * image.width + x] = t;
            buffer32[(image.height - 1 - y) * image.width + x] = h;
        }
    }

    return (struct image){image.width, image.height, buffer};
}

GLuint loadTexture(const char* filename, GLint type) {
    struct image image = loadPNG(filename);

    GLuint texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, type, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.buffer);
    free(image.buffer);

    return texture;
}
