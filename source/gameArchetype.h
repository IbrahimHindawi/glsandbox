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
    hkArray box; // vec4
    hkArray model; // mat4
} gameArchetype;

void gameArchetypeInitializeMemory(gameArchetype *archetype, i32 n) {
    archetype->vao = hkArrayCreate(sizeof(u32), n);
    archetype->vbo = hkArrayCreate(sizeof(u32), n);
    archetype->ebo = hkArrayCreate(sizeof(u32), n);
    archetype->vertex_count = hkArrayCreate(sizeof(u32), n);
    archetype->index_count = hkArrayCreate(sizeof(u32), n);
    archetype->pos = hkArrayCreate(sizeof(vec3), n);
    archetype->vel = hkArrayCreate(sizeof(vec3), n);
    archetype->box = hkArrayCreate(sizeof(vec4), n);
    archetype->model = hkArrayCreate(sizeof(mat4), n);
}

void gameArchetypeInitializeMemoryGrid(gameArchetype *archetype, i32 n) {
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

void gameArchetypeDeinitializeMemory(gameArchetype *archetype) {
    hkArrayDestroy(&archetype->vao);
    hkArrayDestroy(&archetype->vbo);
    hkArrayDestroy(&archetype->ebo);
    hkArrayDestroy(&archetype->vertex_count);
    hkArrayDestroy(&archetype->index_count);
    hkArrayDestroy(&archetype->pos);
    hkArrayDestroy(&archetype->vel);
    hkArrayDestroy(&archetype->box);
    hkArrayDestroy(&archetype->model);
}

void gameArchetypeInitializeMemoryRenderer(gameArchetype *archetype, f32 *vertices, u32 vertex_count, i32 *indices, u32 index_count) {
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

void gameArchetypeSetupPositionsAsGrid(gameArchetype *archetype) {
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
            // ((vec3 *)archetype->vel.data)[i][0] = -1.f;
            // ((vec3 *)archetype->vel.data)[i][1] = 0.f;
            // archetype->pos.data[i + j * f][0] = a * s - f/2;
            // meshes.pos[i + j * f][1] = b * s - f/2;
            // meshes.pos[i + j * f][2] = 0.f;
            printf("{%f, %f}\n", a*s, b*s);
        }
        a += 1.f;
        b = -1.f;
    }
}

void gameArchetypeSetupCollisionBoxes(gameArchetype *archetype, const f32 box_width, const f32 box_height) {
    const i64 n = archetype->index_count.length;
    vec4 *box = (vec4 *)archetype->box.data;
    const vec3 *pos = (vec3 *)archetype->pos.data;
    for(i32 i = 0; i < n; ++i) {
        box[i][0] = pos[i][0];
        box[i][1] = pos[i][1];
        box[i][2] = box_width;
        box[i][3] = box_height;
        // printf("{%f, %f, %f, %f}\n", box[i][0], box[i][1], box[i][2], box[i][3]);
    }
}

void gameArchetypeSetupPositions(gameArchetype *archetype, vec3 p) {
    // initalize velocities
    vec3 *pos = (vec3 *)archetype->pos.data;
    const i32 n = (i32)archetype->index_count.length;
    for(i32 i = 0; i < n; ++i) {
        pos[i][0] = p[0];
        pos[i][1] = p[1];
        pos[i][2] = p[2];
        // printf("step %f\n", a);
    }
}

void gameArchetypeSetupVelocities(gameArchetype *archetype, vec3 v) {
    // initalize velocities
    vec3 *vel = (vec3 *)archetype->vel.data;
    const i32 n = (i32)archetype->index_count.length;
    for(i32 i = 0; i < n; ++i) {
        vel[i][0] = v[0];
        vel[i][1] = v[1];
        vel[i][2] = v[2];
        // printf("step %f\n", a);
    }
}

void gameArchetypeSetupPositionsAsLine(gameArchetype *archetype, const f32 s) {
    // initalize positions
    f32 a = -1.f;
    vec3 *pos = archetype->pos.data;
    const i32 n = (i32)archetype->index_count.length;
    const f32 f = (2.f / (n - 1));
    // printf("factor %f\n", f);
    for(i32 i = 0; i < n; ++i) {
        pos[i][0] = a * s;
        // printf("step %f\n", a);
        a += f;
    }
}

void gameArchetypeUpdateVelocities(gameArchetype *archetype, f32 time) {
    const i64 n = archetype->index_count.length;
    for(i32 i = 0; i < n; ++i) {
        f32 new_vel = (f32)sin(time);
        // printf("%f\n", new_vel);
        ((vec3 *)archetype->vel.data)[i][0] = new_vel;
    }
}

void gameArchetypeUpdateColliders(gameArchetype *archetype) {
    const i64 n = archetype->index_count.length;
    vec4 *box = (vec4 *)archetype->box.data;
    const vec3 *pos = (vec3 *)archetype->pos.data;
    for(i32 i = 0; i < n; ++i) {
        box[i][0] = pos[i][0];
        box[i][1] = pos[i][1];
        // printf("{%f, %f, %f, %f}\n", box[i][0], box[i][1], box[i][2], box[i][3]);
    }
    // printf("\n");
}

int boxAABBCollision(vec4 boxa, vec4 boxb) {
    if (boxa[0] < boxb[0] + boxb[2] &&
        boxa[0] + boxa[2] > boxb[0] &&
        boxa[1] < boxb[1] + boxb[3] &&
        boxa[1] + boxa[3] > boxb[1]) {
        return 1;
    } else {
        return 0;
    }
}

i32 gameArchetypeCheckCollisions(gameArchetype *archetypeA, gameArchetype *archetypeB) {
    const i64 na = archetypeA->index_count.length;
    vec4 *boxa = (vec4 *)archetypeA->box.data;

    const i64 nb = archetypeB->index_count.length;
    vec4 *boxb = (vec4 *)archetypeB->box.data;
    // printf("%lld, %lld\n", na, nb);

    i32 coll_id = -1;
    for(i32 i = 0; i < na; ++i) {
        for(i32 j = 0; j < nb; ++j) {
            if(boxAABBCollision(boxa[i], boxb[j])) {
                // printf("boxa {%f, %f, %f, %f}\n", boxa[i][0], boxa[i][1], boxa[i][2], boxa[i][3]);
                // printf("boxb {%f, %f, %f, %f}\n", boxb[j][0], boxb[j][1], boxb[j][2], boxb[j][3]);
                // printf("HIT!\n");
                coll_id = j;
                // printf("collision id = %d\n", coll_id);
                // break;
                goto collision_exit;
            } else {
                coll_id = -1;
            }
        }
        // printf("{%f, %f, %f, %f}\n", box[i][0], box[i][1], box[i][2], box[i][3]);
    }
collision_exit:
    // printf("\n");
    // printf("returned collision id = %d\n", coll_id);
    return coll_id;
}

void gameArchetypeUpdate(gameArchetype *archetype, f32 deltaTime, f32 speed) {
    vec3 *pos = ((vec3 *)archetype->pos.data);
    vec3 *vel = ((vec3 *)archetype->vel.data);
    mat4 *model = ((mat4 *)archetype->model.data);
    const i64 n = archetype->index_count.length; 
    for(i32 i = 0; i < n; ++i) {
        // transformations
        pos[i][0] += vel[i][0] * speed * deltaTime;
        pos[i][1] += vel[i][1] * speed * deltaTime;

        // S T R
        glm_mat4_identity(&model[i][0]);
        glm_scale_uni(&model[i][0], .15f);
        glm_translate(&model[i][0], pos[i]);
        glm_rotate(&model[i][0], pi * 0.5f, (vec3){1.f, 0.f, 0.f});
    }
}

void gameArchetypeUpdatePlayer(gameArchetype *archetype, f32 deltaTime, f32 speed) {
    vec3 *pos = ((vec3 *)archetype->pos.data);
    vec3 *vel = ((vec3 *)archetype->vel.data);
    mat4 *model = ((mat4 *)archetype->model.data);
    const i64 n = archetype->index_count.length; 
    for(i32 i = 0; i < n; ++i) {
        // transformations
        pos[i][0] += vel[i][0] * speed * deltaTime;
        pos[i][1] += vel[i][1] * speed * deltaTime;

        // S T R
        glm_mat4_identity(&model[i][0]);
        glm_scale_uni(&model[i][0], .15f);
        glm_translate(&model[i][0], pos[i]);
        glm_rotate(&model[i][0], pi * .5f, (vec3){1.f, 0.f, 0.f});
        glm_rotate(&model[i][0], pi, (vec3){0.f, 1.f, 0.f});
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

void gameSpawnProjectileAtEntity(gameArchetype *archetypeHero, gameArchetype *archetypeProjectile, i32 id) {
    vec3 *pos = ((vec3 *)archetypeHero->pos.data);
    vec3 *projectile_pos = ((vec3 *)archetypeProjectile->pos.data);
    const i64 projectile_length = archetypeProjectile->index_count.length; 
    static i32 current_projectile_pool_index = 0;
    projectile_pos[current_projectile_pool_index][0] = pos[id][0];
    projectile_pos[current_projectile_pool_index][1] = pos[id][1];
    projectile_pos[current_projectile_pool_index][2] = pos[id][2];
    current_projectile_pool_index = (current_projectile_pool_index + 1) % projectile_length;
    // printf("%d\n", current_projectile_pool_index);
    return;
}
