#pragma once

#include <glad/glad.h>
#include <cglm/vec3.h>

#include "core.h"

typedef struct {
    u32 vao;
    u32 vbo;
    u32 ebo;
    u32 vertex_count;
    u32 index_count;
    vec3 pos;
    vec3 vel;
} Mesh;

/*
 * Create a mesh that has Pos & UVs.
 */
Mesh meshCreate(f32 *vertices, u32 vertex_count, i32 *indices, u32 index_count) {
    Mesh mesh = {0};
    mesh.vertex_count = vertex_count;
    mesh.index_count = index_count;

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertex_count * sizeof(f32), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.index_count * sizeof(i32), indices, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);  

    // tex
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(sizeof(f32) * 3));
    glEnableVertexAttribArray(1);  

    glBindVertexArray(0);

    return mesh;
}

void meshRender(Mesh *mesh, u32 texture, u32 shader_program) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(shader_program);
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
