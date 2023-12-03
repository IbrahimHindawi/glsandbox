#pragma once

#include <glad/glad.h>

#include <cglm/vec3.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>

#include "core.h"

#include "models/ship.h"
#include "models/streak.h"
#include "models/box.h"
#include "models/planexy.h"
#include "models/planeyz.h"
#include "models/planezx.h"

/*
// box
u32 plane_indices[] = { 
    0, 1, 3,
    1, 2, 3
};
f32 plane_vertices[] = {
    //  x     y      z     s     t     u    nx    ny    nz
     1.0f, 0.0f,  1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
     1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
    -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
    -1.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
};
*/

enum MeshName { 
    // #include "config.ini"
    Ship,
    Streak,
    PlaneXY,
    PlaneYZ,
    PlaneZX,
    Box,
    MeshCount
};

typedef struct {
    f32 *vertices;
    u32 vertices_count;
    u32 *indices;
    u32 indices_count;
} MeshRawData;

MeshRawData MeshRawDataArray[MeshCount];
u32 MeshVAOArray[MeshCount];

MeshRawData MeshDataInitialize(f32 *vertices, u32 vertices_count, u32 *indices, u32 indices_count) {
    MeshRawData result = {0};
    result.vertices = vertices;
    result.vertices_count = vertices_count;
    result.indices = indices;
    result.indices_count = indices_count;
    return result;
}

void MeshVAOGen(u32 *vao, MeshRawData *mesh_data) {

    u32 vbo = 0;
    u32 ebo = 0;

    glGenVertexArrays(1, vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(*vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_data->vertices_count * sizeof(f32), mesh_data->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_data->indices_count * sizeof(i32), mesh_data->indices, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);  

    // tex
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(sizeof(f32) * 3));
    glEnableVertexAttribArray(1);  

    // nor
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(f32), (void*)(sizeof(f32) * 6));
    glEnableVertexAttribArray(2);  

    return;
}
