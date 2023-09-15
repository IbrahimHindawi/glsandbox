#pragma once

#include <math.h>
#include <glad/glad.h>
#include <cglm/vec3.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>

#include "core.h"
#include "hkArray.h"

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

#define M 22
#define N (M * M)
typedef struct {
    u32 vao[N];
    u32 vbo[N];
    u32 ebo[N];
    u32 vertex_count[N];
    u32 index_count[N];
    vec3 pos[N];
    vec3 vel[N];
    mat4 model[N];
} Meshes;

typedef struct {
    mat4 view;
    // mat4 proj;
} XForms;

void meshesCreate(Meshes *meshes, f32 *vertices, u32 vertex_count, i32 *indices, u32 index_count) {
    for(i32 i = 0; i < N; ++i) {
        meshes->vertex_count[i] = vertex_count;
        meshes->index_count[i] = index_count;

        glGenVertexArrays(1, &meshes->vao[i]);
        glGenBuffers(1, &meshes->vbo[i]);
        glGenBuffers(1, &meshes->ebo[i]);

        glBindVertexArray(meshes->vao[i]);

        glBindBuffer(GL_ARRAY_BUFFER, meshes->vbo[i]);
        glBufferData(GL_ARRAY_BUFFER, meshes->vertex_count[i] * sizeof(f32), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes->ebo[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshes->index_count[i] * sizeof(i32), indices, GL_STATIC_DRAW);

        // pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)0);
        glEnableVertexAttribArray(0);  

        // tex
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(sizeof(f32) * 3));
        glEnableVertexAttribArray(1);  

        glBindVertexArray(0);
    }
    return;
}

// Archetype V2
////////////////////////////////////////////////////////////////
typedef struct {
    hkArray vao; // u32
    hkArray vbo; // u32
    hkArray ebo; // u32
    hkArray vertex_count; // u32
    hkArray index_count;  // u32
    hkArray pos; // vec3
    hkArray vel; // vec3
    hkArray model; // mat4
} gameArchetype;

void gameArchetypeInitalize(gameArchetype *archetype, i32 n) {
    const u32 edge = n * n;
    archetype->vao = hkArrayCreate(sizeof(u32), edge);
    archetype->vbo = hkArrayCreate(sizeof(u32), edge);
    archetype->ebo = hkArrayCreate(sizeof(u32), edge);
    archetype->vertex_count = hkArrayCreate(sizeof(u32), edge);
    archetype->index_count = hkArrayCreate(sizeof(u32), edge);
    archetype->pos = hkArrayCreate(sizeof(vec3), edge);
    archetype->vel = hkArrayCreate(sizeof(vec3), edge);
    archetype->model = hkArrayCreate(sizeof(mat4), edge);
}

void gameArchetypeCreate(gameArchetype *archetype, f32 *vertices, u32 vertex_count, i32 *indices, u32 index_count) {
    const i64 n = archetype->index_count.length; 
    u32 *vao = ((u32 *)archetype->vao.data);
    u32 *vbo = ((u32 *)archetype->vbo.data);
    u32 *ebo = ((u32 *)archetype->ebo.data);
    for(i32 i = 0; i < n; ++i) {
        ((u32 *)archetype->vertex_count.data)[i] = vertex_count;
        ((u32 *)archetype->index_count.data)[i] = index_count;

        glGenVertexArrays(1, &vao[i]);
        glGenBuffers(1, &vbo[i]);
        glGenBuffers(1, &ebo[i]);

        glBindVertexArray(vao[i]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        glBufferData(GL_ARRAY_BUFFER, ((u32 *)archetype->vertex_count.data)[i] * sizeof(f32), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, ((u32 *)archetype->index_count.data)[i] * sizeof(i32), indices, GL_STATIC_DRAW);

        // pos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)0);
        glEnableVertexAttribArray(0);  

        // tex
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(sizeof(f32) * 3));
        glEnableVertexAttribArray(1);  

        glBindVertexArray(0);
    }
    return;
}

void gameArchetypeSetupPositions(gameArchetype *archetype) {
    // initalize positions
    f32 a = 0.f;
    f32 b = -1.f;
    const f32 s = 1.f;
    const i32 f = (i32)sqrt((f64)archetype->index_count.length);
    for(i32 i = 0; i < f; ++i) {
        for(i32 j = 0; j < f; ++j) {
            b += 1.f;
            vec3 *posvector = archetype->pos.data;
            posvector[i + j * f][0] = a * s - f/2;
            posvector[i + j * f][1] = b * s - f/2;
            // posvector[i + j * f][2] = -50.f;
            // archetype->pos.data[i + j * f][0] = a * s - f/2;
            // meshes.pos[i + j * f][1] = b * s - f/2;
            // meshes.pos[i + j * f][2] = 0.f;
            printf("{%f, %f}\n", a*s, b*s);
        }
        a += 1.f;
        b = -1.f;
    }
}

void gameArchetypeUpdate(gameArchetype *archetype, f32 deltaTime, f32 *angle) {
    vec3 *pos = ((vec3 *)archetype->pos.data);
    vec3 *vel = ((vec3 *)archetype->vel.data);
    *angle += .01f;
    const i64 n = archetype->index_count.length; 
    for(i32 i = 0; i < n; ++i) {
        // transformations
        pos[i][0] += vel[i][0] * deltaTime;
        pos[i][1] += vel[i][1] * deltaTime;

        // S T R
        glm_mat4_identity(&((mat4 *)archetype->model.data)[i][0]);
        glm_scale_uni(&((mat4 *)archetype->model.data)[i][0], 1.f);
        glm_translate(&((mat4 *)archetype->model.data)[i][0], pos[i]);
        glm_rotate(&((mat4 *)archetype->model.data)[i][0], *angle, (vec3){1.f, 1.f, 0.f});
        // glm_rotate(meshes.model[i], 3.141592f * .5f, (vec3){0.f, 1.f, 0.f});
    }
}

void gameArchetypeRender(gameArchetype *archetype, u32 shader_program, mat4 view, mat4 proj, u32 texture) {
    const i64 n = archetype->index_count.length; 
    for(i32 i = 0; i < n; ++i) {
        // uniforms
        uint32_t view_location = glGetUniformLocation(shader_program, "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
        uint32_t proj_location = glGetUniformLocation(shader_program, "proj");
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj[0]);
        uint32_t model_location = glGetUniformLocation(shader_program, "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, ((mat4 *)archetype->model.data)[i][0]);
        // draw
        // meshRender(&mesh, texture, shader_program);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUseProgram(shader_program);
        glBindVertexArray(((u32 *)archetype->vao.data)[i]);
        glDrawElements(GL_TRIANGLES, ((u32 *)archetype->index_count.data)[i], GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

