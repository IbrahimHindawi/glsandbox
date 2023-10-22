#include <stdio.h>
#include <glad/glad.h>
#include <string.h>

#include "core.h"

#define error_string_create(error_string, error_string_buffer) { \
    const char* error_string = stringify(error_string); \
    u64 error_string_len = strlen(stringify(error_string)); \
    memcpy((error_string_buffer), stringify(error_string), error_string_len); \
}


GLenum ropsCheckError(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        char error_buffer[128] = {0};
        switch (errorCode)
        {
            case GL_INVALID_ENUM: error_string_create(ROPS_GL_INVALID_ENUM, error_buffer); break;
            case GL_INVALID_VALUE: error_string_create(ROPS_GL_INVALID_VALUE, error_buffer); break;
            case GL_INVALID_OPERATION: error_string_create(ROPS_GL_INVALID_OPERATION, error_buffer); break;
            case GL_STACK_OVERFLOW: error_string_create(ROPS_GL_STACK_OVERFLOW, error_buffer); break;
            case GL_STACK_UNDERFLOW: error_string_create(ROPS_GL_STACK_UNDERFLOW, error_buffer); break;
            case GL_OUT_OF_MEMORY: error_string_create(ROPS_GL_OUT_OF_MEMORY, error_buffer); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error_string_create(ROPS_GL_INVALID_FRAMEBUFFER_OPERATION, error_buffer); break;
        }
        printf("RopsError: %s File: %s Line: %d\n", error_buffer, file, line);
    }
    return errorCode;
}


void GLAPIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    printf("---------------\n");
    printf("Debug message (%d):  %s \n", id, message);
    char error_buffer[256] = {0};

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             error_string_create(Source_API, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   error_string_create(Source_Window_System, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: error_string_create(Source_Shader_Compiler, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     error_string_create(Source_Third_Party, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_SOURCE_APPLICATION:     error_string_create(Source_Application, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_SOURCE_OTHER:           error_string_create(Source_Other, error_buffer); printf("%s\n", error_buffer); break;
    } printf("\n");

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               error_string_create(Type_Error, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: error_string_create(Type_Deprecated_Behaviour, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  error_string_create(Type_Undefined_Behaviour, error_buffer); printf("%s\n", error_buffer); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         error_string_create(Type_Portability, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         error_string_create(Type_Performance, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_TYPE_MARKER:              error_string_create(Type_Marker, error_buffer); printf("%s\n", error_buffer);break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          error_string_create(Type_Push_Group, error_buffer); printf("%s\n", error_buffer);break;
        case GL_DEBUG_TYPE_POP_GROUP:           error_string_create(Type_Pop_Group, error_buffer); printf("%s\n", error_buffer);break;
        case GL_DEBUG_TYPE_OTHER:               error_string_create(Type_Other, error_buffer); printf("%s\n", error_buffer); break;
    } printf("\n");
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         error_string_create(Severity_high, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       error_string_create(Severity_medium, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_SEVERITY_LOW:          error_string_create(Severity_low, error_buffer); printf("%s\n", error_buffer); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: error_string_create(Severity_notification, error_buffer); printf("%s\n", error_buffer); break;
    } printf("\n");
    printf("\n");
}

#define ropsCheckError_() ropsCheckError(__FILE__, __LINE__)
