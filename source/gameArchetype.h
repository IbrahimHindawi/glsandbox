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
#include "meshops.h"

#define getComponent(reference, type, attribute) (type *)reference.attribute.data
#define getComponentPtr(reference, type, attribute) (type *)reference->attribute.data

typedef struct {
    i32 fire_active;
    i32 fire_index;
    i32 fire_rate;
    i32 fire_counter;
    i32 fire_burst_count;
    i32 fire_burst_rate;
} FireCore;

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
    hkArray fire_cores; // u32
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
    archetype->fire_cores = hkArrayCreate(sizeof(FireCore), n);
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

void graphicsArchetypeAllocate(GraphicsArchetype *archetype, i32 n) {
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

void graphicsArchetypeDeallocate(GraphicsArchetype *archetype) {
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

void archetypeInitializeMesh(u32 *mesh_data, u32 *index_count_data, const Range range, u32 mesh_id) {
    for(i32 i = range.start; i < range.end; ++i) {
        mesh_data[i] = MeshVAOArray[mesh_id];
        index_count_data[i] = MeshRawDataArray[mesh_id].indices_count;
    }
    return;
}

void archetypeInitialize1u(u32 *data, const Range range, u32 value) {
    for(i32 i = range.start; i < range.end; ++i) {
        data[i] = value;
    }
    return;
}

void archetypeInitialize1i(i32 *data, const Range range, u32 value) {
    for(i32 i = range.start; i < range.end; ++i) {
        data[i] = value;
    }
    return;
}

void archetypeInitialize1f(f32 *data, const Range range, f32 value) {
    for(i32 i = range.start; i < range.end; ++i) {
        data[i] = value;
        // printf("step %f\n", a);
    }
}

void archetypeInitialize3f(vec3 *vector_data, const Range range, vec3 vector_value) {
    for(i32 i = range.start; i < range.end; ++i) {
        vector_data[i][0] = vector_value[0];
        vector_data[i][1] = vector_value[1];
        vector_data[i][2] = vector_value[2];
    }
    return;
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
int boxAABBCollision2d(vec3 posa, vec3 scla, vec3 posb, vec3 sclb) {
    if (posa[0] < posb[0] +       sclb[0] * 2.f &&
        posa[0] + scla[0] * 2.f > posb[0]       &&
        posa[1] < posb[1] +       sclb[2] * 2.f &&
        posa[1] + scla[2] * 2.f > posb[1]) {
        return 1;
    } else {
        return 0;
    }
}
//
// vec4 box {x, y, w, h}
// function intersect(a, b) {
//   return (
//     a.minX <= b.maxX &&
//     a.maxX >= b.minX &&
//     a.minY <= b.maxY &&
//     a.maxY >= b.minY &&
//     a.minZ <= b.maxZ &&
//     a.maxZ >= b.minZ
//   );
// }
int boxAABBCollision3d(vec3 posa, vec3 scla, vec3 posb, vec3 sclb) {
    float a_minX = posa[0] + -(scla[0] * 0.5f);
    float a_maxX = posa[0] +   scla[0] * 0.5f;
    float a_minY = posa[1] + -(scla[1] * 0.5f);
    float a_maxY = posa[1] +   scla[1] * 0.5f;
    float a_minZ = posa[2] + -(scla[2] * 0.5f);
    float a_maxZ = posa[2] +   scla[2] * 0.5f;
    float b_minX = posb[0] + -(sclb[0] * 0.5f);
    float b_maxX = posb[0] +   sclb[0] * 0.5f;
    float b_minY = posb[1] + -(sclb[1] * 0.5f);
    float b_maxY = posb[1] +   sclb[1] * 0.5f;
    float b_minZ = posb[2] + -(sclb[2] * 0.5f);
    float b_maxZ = posb[2] +   sclb[2] * 0.5f;
    if (
        a_minX <= b_maxX &&
        a_maxX >= b_minX &&
        a_minY <= b_maxY &&
        a_maxY >= b_minY &&
        a_minZ <= b_maxZ &&
        a_maxZ >= b_minZ
    ) {
        return 1;
    } else {
        return 0;
    }
}

i32 archetypeCheckCollisions(vec3 *posa, vec3 *scla, const Range na, vec3 *posb, vec3 *sclb, const Range nb) {// u32 na, vec4 *boxa, u32 nb, vec4 *boxb) {
    i32 coll_id = -1;
    for(i32 i = na.start; i < na.end; ++i) {
        for(i32 j = nb.start; j < nb.end; ++j) {
            if(boxAABBCollision3d(posa[i], scla[i], posb[j], sclb[j])) {
                // printf("boxa {%f, %f, %f, %f}\n", boxa[i][0], boxa[i][1], boxa[i][2], boxa[i][3]);
                // printf("boxb {%f, %f, %f, %f}\n", boxb[j][0], boxb[j][1], boxb[j][2], boxb[j][3]);
                // printf("HIT!\n");
                // printf("collision id = %d\n", coll_id);
                // break;
                coll_id = j;
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

void archetypeProcessCollisions(GameArchetype *game_archetype, RangeArena *range_arena_game, i32 a_index, i32 b_index) {
    // check collisions
    i32 coll_id = archetypeCheckCollisions(
        (vec3 *)game_archetype->positions.data,
        (vec3 *)game_archetype->scales.data,
        ((Range *)range_arena_game->ranges.data)[a_index],
        (vec3 *)game_archetype->positions.data,
        (vec3 *)game_archetype->scales.data,
        ((Range *)range_arena_game->ranges.data)[b_index]);
    // printf("collision id = %d\n", coll_id);
    if (coll_id != -1) {
        ((vec3 *)game_archetype->positions.data)[coll_id][0] = -1000.f;
        ((vec3 *)game_archetype->positions.data)[coll_id][1] = -1000.f;
        // printf("collision id = %d\n", coll_id);
        coll_id = -1;
    }
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

void archetypeSetUniform1f(u32 *shader_program_data, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        glUseProgram(shader_program_data[i]);
        // uniforms
        u32 time_location = glGetUniformLocation(shader_program_data[i], "time");
        glUniform1f(time_location, SDL_GetTicks() / 1000.f);
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

void archetypeSpawnProjectileAtEntity(vec3 *positions, i32 *fire_index_data, const Range projectile_range, i32 source_entity_id) {
    // printf("fire_index_data[i] = %d. ", *fire_index_data);
    // printf("fire_index_data[i + projectile_range.start] = %d. ", *fire_index_data+projectile_range.start);
    // printf("address: %p\n", fire_index_data);
    vec3 *source_position = positions;
    vec3 *dest_position = positions;
    *fire_index_data = (*fire_index_data + 1) % projectile_range.length; // + projectile_range.start;
    dest_position[*fire_index_data + projectile_range.start][0] = source_position[source_entity_id][0];
    dest_position[*fire_index_data + projectile_range.start][1] = source_position[source_entity_id][1];
    dest_position[*fire_index_data + projectile_range.start][2] = source_position[source_entity_id][2];
    return;
}

void archetypeSpawnProjectileAtEntityAI(vec3 *positions, FireCore *fire_cores, const Range projectile_range, i32 source_entity_id) {
    vec3 *source_position = positions;
    vec3 *dest_position = positions;

    if((fire_cores[source_entity_id].fire_counter % fire_cores[source_entity_id].fire_rate) == 0 && fire_cores[source_entity_id].fire_active == true) {
        fire_cores[source_entity_id].fire_counter = 0;
        // fire start
        fire_cores[source_entity_id].fire_index = (fire_cores[source_entity_id].fire_index + 1) % projectile_range.length; // + projectile_range.start;
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start][0] = source_position[source_entity_id][0];
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start][1] = source_position[source_entity_id][1];
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start][2] = source_position[source_entity_id][2];
        // fire end
        fire_cores[source_entity_id].fire_burst_count -= 1;
        if(fire_cores[source_entity_id].fire_burst_count == 0) { 
            fire_cores[source_entity_id].fire_active = false; 
            fire_cores[source_entity_id].fire_burst_count = fire_cores[source_entity_id].fire_burst_rate; 
        }
    }
    fire_cores[source_entity_id].fire_counter += 1;
    if(fire_cores[source_entity_id].fire_counter >= 100) { 
        fire_cores[source_entity_id].fire_active = true; 
    }

    return;
}
//    archetypeInitalizeGameAndGraphics(
//       game_archetype, 
//       graphics_archetype,
//       Plane, shader_program_box, texture,
//       (vec3){0.f, -3.f, 0.f}, 
//       (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
//       (vec3){.35f, 1.f, .35f},
//       Ship, shader_program, texture,
//       (vec3){0.f, 0.f, 0.f}, 
//       (vec3){pi * 0.5f, pi, 0.f}, 
//       (vec3){.15f, .15f, .15f},
//       hero_range);

void archetypeInitalizeGameAndGraphics(
    GameArchetype game_archetype, 
    GraphicsArchetype graphics_archetype,
    i32 gm_mesh, i32 gm_shader, i32 gm_texture,
    vec3 gm_pos, vec3 gm_rot, vec3 gm_scale,
    i32 gfx_mesh, i32 gfx_shader, i32 gfx_texture,
    vec3 gfx_pos, vec3 gfx_rot, vec3 gfx_scale,
    const Range *range) {
    // Game
    archetypeInitalizeMeshesShadersTextures(
        (u32 *)game_archetype.vaos.data, MeshVAOArray[gm_mesh], 
        (u32 *)game_archetype.index_counts.data, MeshRawDataArray[gm_mesh].indices_count, 
        (u32 *)game_archetype.shaders.data, gm_shader,
        (u32 *)game_archetype.textures.data, gm_texture,
        *range);
    archetypeInitializeTransforms(
        (vec3 *)game_archetype.positions.data,
        (vec3 *)game_archetype.rotations.data,
        (vec3 *)game_archetype.scales.data,
        gm_pos, gm_rot, gm_scale,
        *range);
    // Graphics
    archetypeInitalizeMeshesShadersTextures(
        (u32 *)graphics_archetype.vaos.data, MeshVAOArray[gfx_mesh],
        (u32 *)graphics_archetype.index_counts.data, MeshRawDataArray[gfx_mesh].indices_count,
        (u32 *)graphics_archetype.shaders.data, gfx_shader,
        (u32 *)graphics_archetype.textures.data, gfx_texture,
        *range);
    archetypeInitializeTransforms(
        (vec3 *)graphics_archetype.positions.data,
        (vec3 *)graphics_archetype.rotations.data,
        (vec3 *)graphics_archetype.scales.data,
        gfx_pos, gfx_rot, gfx_scale,
        *range);
    return;
}
