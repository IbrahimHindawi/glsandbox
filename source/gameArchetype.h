#pragma once

#include <math.h>
#include <glad/glad.h>
#include <cglm/vec3.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <SDL2/SDL.h>

#include "core.h"
#include "hkArray.h"
#include "rangeops.h"

#define getComponent(reference, type, attribute) (type *)reference.attribute.data
#define getComponentPtr(reference, type, attribute) (type *)reference->attribute.data

typedef struct {
    // debug-graphics
    hkArray vaos; // u32
    hkArray index_counts; // u32
    hkArray shaders; // u32
    hkArray textures; // u32
    // game logic
    hkArray speeds; // f32 
    hkArray velocities; // vec3
    hkArray positions; // vec3
    hkArray rotations; // vec3
    hkArray scales; // vec3
    hkArray models; // mat4
    hkArray fire_indices; // u32
} GameArchetype;

void gameArchetypeAllocate(GameArchetype *archetype, i32 n) {
    // game
    archetype->vaos = hkArrayCreate(sizeof(u32), n);
    archetype->index_counts = hkArrayCreate(sizeof(u32), n);
    archetype->shaders = hkArrayCreate(sizeof(u32), n);
    archetype->textures = hkArrayCreate(sizeof(u32), n);
    // graphics
    archetype->speeds = hkArrayCreate(sizeof(f32), n);
    archetype->velocities = hkArrayCreate(sizeof(vec3), n);
    archetype->positions = hkArrayCreate(sizeof(vec3), n);
    archetype->rotations = hkArrayCreate(sizeof(vec3), n);
    archetype->scales = hkArrayCreate(sizeof(vec3), n);
    archetype->models = hkArrayCreate(sizeof(mat4), n);
    archetype->fire_indices = hkArrayCreate(sizeof(i32), n);
}

void gameArchetypeDeallocate(GameArchetype *archetype) {
    // game
    hkArrayDestroy(&archetype->vaos);
    hkArrayDestroy(&archetype->index_counts);
    hkArrayDestroy(&archetype->shaders);
    hkArrayDestroy(&archetype->textures);
    // graphics
    hkArrayDestroy(&archetype->speeds);
    hkArrayDestroy(&archetype->velocities);
    hkArrayDestroy(&archetype->positions);
    hkArrayDestroy(&archetype->rotations);
    hkArrayDestroy(&archetype->scales);
    hkArrayDestroy(&archetype->models);
}

typedef struct {
    hkArray vaos; // u32
    hkArray index_counts; // u32
    hkArray shaders; // u32
    hkArray textures; // u32
    hkArray positions; // vec3
    hkArray rotations; // vec3
    hkArray scales; // vec3
    hkArray models; // mat4
} GraphicsArchetype;

void boxArchetypeAllocate(GraphicsArchetype *archetype, i32 n) {
    // game
    archetype->vaos = hkArrayCreate(sizeof(u32), n);
    archetype->index_counts = hkArrayCreate(sizeof(u32), n);
    archetype->shaders = hkArrayCreate(sizeof(u32), n);
    archetype->textures = hkArrayCreate(sizeof(u32), n);
    // graphics
    archetype->positions = hkArrayCreate(sizeof(vec3), n);
    archetype->rotations = hkArrayCreate(sizeof(vec3), n);
    archetype->scales = hkArrayCreate(sizeof(vec3), n);
    archetype->models = hkArrayCreate(sizeof(mat4), n);
}

void boxArchetypeDeallocate(GraphicsArchetype *archetype) {
    // game
    hkArrayDestroy(&archetype->vaos);
    hkArrayDestroy(&archetype->index_counts);
    hkArrayDestroy(&archetype->shaders);
    hkArrayDestroy(&archetype->textures);
    // graphics
    hkArrayDestroy(&archetype->positions);
    hkArrayDestroy(&archetype->rotations);
    hkArrayDestroy(&archetype->scales);
    hkArrayDestroy(&archetype->models);
}

void archetypeInitalizeMeshes(u32 *vao_data, u32 vao, u32 *index_count_data, u32 index_count, const Range range) {
    // u32 *vao = ((u32 *)archetype->vao.data);
    for(i32 i = range.start; i < range.end; ++i) {
        vao_data[i] = vao;
        index_count_data[i] = index_count;
    }
    return;
}

void archetypeInitalizeMeshesShadersTextures(u32 *vao_data, u32 vao, 
                                             u32 *index_count_data, u32 index_count, 
                                             u32 *shader_program_data, u32 shader_program, 
                                             u32 *texture_data, u32 texture, 
                                             const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        vao_data[i] = vao;
        index_count_data[i] = index_count;
        shader_program_data[i] = shader_program;
        texture_data[i] = texture;
    }
    return;
}

void archetypeInitializePositionsAsGrid(GameArchetype *archetype) {
    // initalize positionitions
    f32 a = 0.f;
    f32 b = -1.f;
    const f32 s = 1.f;
    const i32 f = (i32)sqrt((f64)archetype->index_counts.length);
    for(i32 i = 0; i < f; ++i) {
        for(i32 j = 0; j < f; ++j) {
            b += 1.f;
            vec3 *positionvector = archetype->positions.data;
            positionvector[i + j * f][0] = a * s - f/2;
            positionvector[i + j * f][1] = b * s - f/2;
            // positionvector[i + j * f][2] = -50.f;
            // ((vec3 *)archetype->velocities.data)[i][0] = -1.f;
            // ((vec3 *)archetype->velocities.data)[i][1] = 0.f;
            // archetype->positions.data[i + j * f][0] = a * s - f/2;
            // meshes.positions[i + j * f][1] = b * s - f/2;
            // meshes.positions[i + j * f][2] = 0.f;
            printf("{%f, %f}\n", a*s, b*s);
        }
        a += 1.f;
        b = -1.f;
    }
}

void archetypeSetPositions(vec3 *position_data, vec3 p, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        position_data[i][0] = p[0];
        position_data[i][1] = p[1];
        position_data[i][2] = p[2];
    }
}

void gameArchetypeInitializeRotations(GameArchetype *archetype, vec3 r) {
    vec3 *rotations = ((vec3 *)archetype->rotations.data);
    const i32 n = (i32)archetype->index_counts.length;
    for(i32 i = 0; i < n; ++i) {
        rotations[i][0] = r[0];
        rotations[i][1] = r[1];
        rotations[i][2] = r[2];
        // printf("step %f\n", a);
    }
}

void gameArchetypeInitializeScales(GameArchetype *archetype, vec3 s) {
    vec3 *scales = ((vec3 *)archetype->scales.data);
    const i32 n = (i32)archetype->index_counts.length;
    for(i32 i = 0; i < n; ++i) {
        scales[i][0] = s[0];
        scales[i][1] = s[1];
        scales[i][2] = s[2];
        // printf("step %f\n", a);
    }
}

void archetypeInitializeTransforms(vec3 *position_data, vec3 *rotation_data, vec3 *scale_data, 
                                       vec3 p, vec3 r, vec3 s, const Range range) {
    // initalize velocityocities
    for(i32 i = range.start; i < range.end; ++i) {
        position_data[i][0] = p[0];
        position_data[i][1] = p[1];
        position_data[i][2] = p[2];
        rotation_data[i][0] = r[0];
        rotation_data[i][1] = r[1];
        rotation_data[i][2] = r[2];
        scale_data[i][0] = s[0];
        scale_data[i][1] = s[1];
        scale_data[i][2] = s[2];
        // printf("step %f\n", a);
    }
}

void archetypeInitializeSpeeds(f32 *speeds, f32 input_speed, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        speeds[i] = input_speed;
        // printf("step %f\n", a);
    }
}

void archetypeInitializeVelocities(vec3 *velocities, vec3 v, const Range range) {
    // initalize velocity
    for(i32 i = range.start; i < range.end; ++i) {
        velocities[i][0] = v[0];
        velocities[i][1] = v[1];
        velocities[i][2] = v[2];
        // printf("step %f\n", a);
    }
}

void archetypeInitializePositionsAsLine(vec3 *position_data, const f32 s, const f32 y, const Range range) {
    f32 a = -1.f;
    const i32 n = range.length;
    const f32 f = (2.f / (range.end - 1));
    // printf("factor %f\n", f);
    for(i32 i = range.start; i < range.end; ++i) {
        position_data[i][0] = a * s;
        position_data[i][1] = y;
        // printf("step %f\n", a);
        a += f;
    }
}

void archetypeUpdateVelocities(vec3 *velocity_data, f32 time, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        // printf("%f\n", velocity_data[i][0]);
        velocity_data[i][0] = (f32)sin(time);
    }
}

void archetypeCopyVector(vec3 *source_position_data, vec3 *dest_position_data, const Range range) {
    // vec4 *box = (vec4 *)archetype->box.data;
    // const vec3 *positions = (vec3 *)archetype->positions.data;
    for(i32 i = range.start; i < range.end; ++i) {
        dest_position_data[i][0] = source_position_data[i][0];
        dest_position_data[i][1] = source_position_data[i][1];
        dest_position_data[i][2] = source_position_data[i][2];
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

// i32 gameArchetypeCheckCollisions(GameArchetype *archetypeA, GameArchetype *archetypeB) {
i32 gameArchetypeCheckCollisions(u32 na, vec4 *boxa, u32 nb, vec4 *boxb) {
    /*
    const i64 na = archetypeA->index_counts.length;
    vec4 *boxa = (vec4 *)archetypeA->box.data;

    const i64 nb = archetypeB->index_counts.length;
    vec4 *boxb = (vec4 *)archetypeB->box.data;
    */
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

// vec4 box {x, y, w, h}
int boxAABBCollision2(vec3 posa, vec3 scla, vec3 posb, vec3 sclb) {
    if (posa[0] < posb[0] +       sclb[0] * 2.f &&
        posa[0] + scla[0] * 2.f > posb[0]       &&
        posa[1] < posb[1] +       sclb[2] * 2.f &&
        posa[1] + scla[2] * 2.f > posb[1]) {
        return 1;
    } else {
        return 0;
    }
}

i32 gameArchetypeCheckCollisions2(vec3 *posa, vec3 *scla, const Range na, vec3 *posb, vec3 *sclb, const Range nb) {// u32 na, vec4 *boxa, u32 nb, vec4 *boxb) {
    i32 coll_id = -1;
    for(i32 i = na.start; i < na.end; ++i) {
        for(i32 j = nb.start; j < nb.end; ++j) {
            if(boxAABBCollision2(posa[i], scla[i], posb[j], sclb[j])) {
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


void archetypeUpdateTransforms(vec3 *position_data, vec3 *rotation_data, vec3 *scale_data, mat4 *model_data, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        // S T R
        glm_mat4_identity(&model_data[i][0]);
        glm_translate(&model_data[i][0], position_data[i]);
        glm_rotate(&model_data[i][0], rotation_data[i][0], (vec3){1.f, 0.f, 0.f});
        glm_rotate(&model_data[i][0], rotation_data[i][1], (vec3){0.f, 1.f, 0.f});
        glm_rotate(&model_data[i][0], rotation_data[i][2], (vec3){0.f, 0.f, 1.f});
        glm_scale(&model_data[i][0], scale_data[i]);
    }
    return;
}

void archetypeIntegrateVelocity(vec3 *position_data, vec3 *velocity_data, f32 *speed_data, f32 delta_time, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        // transformations
        position_data[i][0] += velocity_data[i][0] * speed_data[i] * delta_time;
        position_data[i][1] += velocity_data[i][1] * speed_data[i] * delta_time;
    }
}

void archetypeRender(u32 *vao_data, u32 *shader_program_data, u32 *texture_data, u32* index_count_data, mat4 *model_data, mat4 view, mat4 proj, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        glUseProgram(shader_program_data[i]);
        // uniforms
        u32 view_location = glGetUniformLocation(shader_program_data[i], "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
        u32 proj_location = glGetUniformLocation(shader_program_data[i], "proj");
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj[0]);
        u32 model_location = glGetUniformLocation(shader_program_data[i], "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, model_data[i][0]);
        // draw
        glBindVertexArray(vao_data[i]);
        glBindTexture(GL_TEXTURE_2D, texture_data[i]);
        glDrawElements(GL_TRIANGLES, index_count_data[i], GL_UNSIGNED_INT, 0);
    }
}

void archetypeRenderWires(u32 *vao_data, u32 *shader_program_data, u32 *texture_data, u32* index_count_data, mat4 *model_data, mat4 view, mat4 proj, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        glUseProgram(shader_program_data[i]);
        // uniforms
        u32 view_location = glGetUniformLocation(shader_program_data[i], "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
        u32 proj_location = glGetUniformLocation(shader_program_data[i], "proj");
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj[0]);
        u32 model_location = glGetUniformLocation(shader_program_data[i], "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, model_data[i][0]);
        // draw
        glBindVertexArray(vao_data[i]);
        glBindTexture(GL_TEXTURE_2D, texture_data[i]);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, index_count_data[i], GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void gameArchetypeRenderBG(GameArchetype *archetype, u32 shader_program, mat4 view, mat4 proj) {
    const i64 n = archetype->index_counts.length; 
    for(i32 i = 0; i < n; ++i) {
        glUseProgram(shader_program);
        // uniforms
        u32 time_location = glGetUniformLocation(shader_program, "time");
        glUniform1f(time_location, SDL_GetTicks() / 1000.f);
        u32 view_location = glGetUniformLocation(shader_program, "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
        u32 proj_location = glGetUniformLocation(shader_program, "proj");
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj[0]);
        u32 model_location = glGetUniformLocation(shader_program, "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, ((mat4 *)archetype->models.data)[i][0]);
        // draw
        glBindVertexArray(((u32 *)archetype->vaos.data)[i]);
        // glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, ((u32 *)archetype->index_counts.data)[i], GL_UNSIGNED_INT, 0);
    }
}

void archetypeSpawnProjectileAtEntity(i32 *fire_index_data, i32 id, vec3 *positions, const i32 offset, const i32 buffer_length) {
    printf("fire_index_data[i] = %d. ", *fire_index_data);
    printf("fire_index_data[i + offset] = %d. ", *fire_index_data+offset);
    printf("address: %p\n", fire_index_data);
    vec3 *source_position = positions;
    vec3 *dest_position = positions;
    *fire_index_data = (*fire_index_data + 1) % buffer_length; // + offset;
    dest_position[*fire_index_data + offset][0] = source_position[id][0];
    dest_position[*fire_index_data + offset][1] = source_position[id][1];
    dest_position[*fire_index_data + offset][2] = source_position[id][2];
    return;
}

void archetypeSpawnProjectileAtEntityAI(i32 *fire_index_data, i32 id, vec3 *positions, const i32 offset, const i32 buffer_length, f32 time) {
    // printf("fire_index_data[i] = %d. ", *fire_index_data);
    // printf("fire_index_data[i + offset] = %d. ", *fire_index_data+offset);
    // printf("address: %p\n", fire_index_data);
    // typedef struct { 
    //     u8 fire_active;
    //     u8 fire_index;
    //     u8 fire_counter;
    //     u8 fire_rate;
    // } fire_core;
    static i32 fire_active = false;
    static i32 fire_counter = 0;
    static i32 fire_rate = 20;

    vec3 *source_position = positions;
    vec3 *dest_position = positions;

    if ((fire_counter % fire_rate) == 0 && fire_active == true) {
        static i32 fire_burst_count = 3;
        *fire_index_data = (*fire_index_data + 1) % buffer_length; // + offset;
        fire_counter = 0;
        // fire start
        dest_position[*fire_index_data + offset][0] = source_position[id][0];
        dest_position[*fire_index_data + offset][1] = source_position[id][1];
        dest_position[*fire_index_data + offset][2] = source_position[id][2];
        // fire end
        fire_burst_count -= 1;
        if (fire_burst_count == 0) { fire_active = false; fire_burst_count = 3; }
    }
    fire_counter += 1;
    if (fire_counter >= 100) { fire_active = true; }

    return;
}
