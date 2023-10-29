#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "core.h"
#include "glad/glad.h"

void texture_create(u32 *texture, const char *texture_path) {
        glGenTextures(1, texture);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        i32 width, height, n_channels;
        // u8 *data = stbi_load("resource/cgfx.png", &width, &height, &n_channels, 0);
        // u8 *data = stbi_load("resource/toylowres.jpg", &width, &height, &n_channels, 0);
        u8 *data = stbi_load(texture_path, &width, &height, &n_channels, 0);
        // u8 *data = stbi_load("resource/awesomeface.png", &width, &height, &n_channels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            printf("failed to load texture.");
        }
        STBI_FREE(data);
}
