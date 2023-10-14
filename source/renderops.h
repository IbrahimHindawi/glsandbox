#include <stdio.h>
#include <glad/glad.h>
#include <string.h>

#include "core.h"

#define error_string_create(error_string, error_string_buffer) { \
    const char* error_string = stringify(error_string); \
    u64 error_string_len = strlen(stringify(error_string)); \
    memcpy_s((error_string_buffer), error_string_len, stringify(error_string), error_string_len); \
}


GLenum ropsCheckError(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        char error_buffer[128] = {0};
        switch (errorCode)
        {
            case GL_INVALID_ENUM: {
                error_string_create(ROPS_GL_INVALID_ENUM, error_buffer);
            } break;
            case GL_INVALID_VALUE: {
                error_string_create(ROPS_GL_INVALID_VALUE, error_buffer);
            } break;
            case GL_INVALID_OPERATION: {
                error_string_create(ROPS_GL_INVALID_OPERATION, error_buffer);
            } break;
            case GL_OUT_OF_MEMORY: {
                error_string_create(ROPS_GL_OUT_OF_MEMORY, error_buffer);
            } break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: {
                error_string_create(ROPS_GL_INVALID_FRAMEBUFFER_OPERATION, error_buffer);
            } break;
        }
        printf("RopsError: %s File: %s Line: %d\n", error_buffer, file, line);
    }
    return errorCode;
}

#define ropsCheckError_() ropsCheckError(__FILE__, __LINE__)
