#pragma once

#include "core.h"
#include "fileops.h"

#include <glad/glad.h>

void shader_compile_check(u32 shader); 
void shader_link_check(u32 shader); 

u32 shader_compile(const char* shader_source, GLenum shader_type);
u32 shader_link(u32 vertex_shader, u32 fragment_shader);

void shader_create(u32 *shader_id, const char *vertex_shader_path, const char *fragment_shader_path);
