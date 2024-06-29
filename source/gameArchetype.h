#pragma once

#include <core.h>
#include <math.h>
#include <glad/glad.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/struct.h>
#include <cglm/struct/affine-pre.h>
#include <SDL2/SDL.h>

typedef struct {
    i32 fire_active;
    i32 fire_index;
    i32 fire_rate;
    i32 fire_counter;
    i32 fire_burst_count;
    i32 fire_burst_rate;
} FireCore;

#include <hkArray.h>
#include "rangeops.h"
#include "meshops.h"

#define getComponent(reference, type, attribute) (type *)reference.attribute.data
#define getComponentPtr(reference, type, attribute) (type *)reference->attribute.data

typedef struct {
    // debug-graphics
    hkArray_u32 vaos;
    hkArray_u32 index_counts;
    hkArray_u32 shaders;
    hkArray_u32 textures;
    // game logic
    hkArray_i32 lives;
    hkArray_f32 speeds;
    hkArray_vec3s velocities;
    hkArray_vec3s positions;
    hkArray_vec3s rotations;
    hkArray_vec3s scales;
    hkArray_mat4s models;
    hkArray_u32 fire_cores;
} GameArchetype;

void gameArchetypeAllocate(GameArchetype *archetype, i32 n) {
    // game
    archetype->vaos = hkarray_u32_create(n);
    archetype->index_counts = hkarray_u32_create(n);
    archetype->shaders = hkarray_u32_create(n);
    archetype->textures = hkarray_u32_create(n);
    // graphics
    archetype->lives = hkarray_i32_create(n);
    archetype->speeds = hkarray_f32_create(n);
    archetype->velocities = hkarray_vec3s_create(n);
    archetype->positions = hkarray_vec3s_create(n);
    archetype->rotations = hkarray_vec3s_create(n);
    archetype->scales = hkarray_vec3s_create(n);
    archetype->models = hkarray_mat4s_create(n);
    archetype->fire_cores = hkarray_u32_create(n);
}

void gameArchetypeDeallocate(GameArchetype *archetype) {
    // game
    hkarray_u32_destroy(&archetype->vaos);
    hkarray_u32_destroy(&archetype->index_counts);
    hkarray_u32_destroy(&archetype->shaders);
    hkarray_u32_destroy(&archetype->textures);
    // graphics
    hkarray_i32_destroy(&archetype->lives);
    hkarray_f32_destroy(&archetype->speeds);
    hkarray_vec3s_destroy(&archetype->velocities);
    hkarray_vec3s_destroy(&archetype->positions);
    hkarray_vec3s_destroy(&archetype->rotations);
    hkarray_vec3s_destroy(&archetype->scales);
    hkarray_mat4s_destroy(&archetype->models);
}

typedef struct {
    hkArray_u32 vaos; // u32
    hkArray_u32 index_counts; // u32
    hkArray_u32 shaders; // u32
    hkArray_u32 textures; // u32
    hkArray_vec3s positions; // vec3
    hkArray_vec3s rotations; // vec3
    hkArray_vec3s scales; // vec3
    hkArray_mat4s models; // mat4
} GraphicsArchetype;

void graphicsArchetypeAllocate(GraphicsArchetype *archetype, i32 n) {
    // game
    archetype->vaos = hkarray_u32_create(n);
    archetype->index_counts = hkarray_u32_create(n);
    archetype->shaders = hkarray_u32_create(n);
    archetype->textures = hkarray_u32_create(n);
    // graphics
    archetype->positions = hkarray_vec3s_create(n);
    archetype->rotations = hkarray_vec3s_create(n);
    archetype->scales = hkarray_vec3s_create(n);
    archetype->models = hkarray_mat4s_create(n);
}

void graphicsArchetypeDeallocate(GraphicsArchetype *archetype) {
    // game
    hkarray_u32_destroy(&archetype->vaos);
    hkarray_u32_destroy(&archetype->index_counts);
    hkarray_u32_destroy(&archetype->shaders);
    hkarray_u32_destroy(&archetype->textures);
    // graphics
    hkarray_vec3s_destroy(&archetype->positions);
    hkarray_vec3s_destroy(&archetype->rotations);
    hkarray_vec3s_destroy(&archetype->scales);
    hkarray_mat4s_destroy(&archetype->models);
}

void archetypeInitalizeMeshes(u32 *vao_data, u32 vao, u32 *index_count_data, u32 index_count, const Range range) {
    // u32 *vao = ((u32 *)archetype->vao.data);
    for(i32 i = range.start; i < range.end; ++i) {
        vao_data[i] = vao;
        index_count_data[i] = index_count;
    }
    return;
}

void archetypeInitializeMesh(hkArray_u32 meshes, hkArray_u32 index_counts, const Range range, u32 mesh_id) {
    for(i32 i = range.start; i < range.end; ++i) {
        meshes.data[i] = MeshVAOArray[mesh_id];
        index_counts.data[i] = MeshRawDataArray[mesh_id].indices_count;
    }
    return;
}

void archetypeInitialize1u(hkArray_u32 numbers, const Range range, u32 value) {
    for(i32 i = range.start; i < range.end; ++i) {
        numbers.data[i] = value;
    }
    return;
}

void archetypeInitialize1i(hkArray_i32 numbers, const Range range, i32 value) {
    for(i32 i = range.start; i < range.end; ++i) {
        numbers.data[i] = value;
    }
    return;
}

void archetypeInitialize1f(hkArray_f32 numbers, const Range range, f32 value) {
    for(i32 i = range.start; i < range.end; ++i) {
        numbers.data[i] = value;
        // printf("step %f\n", a);
    }
}

void archetypeInitialize3f(hkArray_vec3s vectors, const Range range, vec3s vector_value) {
    for(i32 i = range.start; i < range.end; ++i) {
        vectors.data[i].x = vector_value.x;
        vectors.data[i].y = vector_value.y;
        vectors.data[i].z = vector_value.z;
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

void archetypeCopyVector(hkArray_vec3s source_positions, hkArray_vec3s dest_positions, const Range range) {
    // vec4 *box = (vec4 *)archetype->box.data;
    // const vec3 *positions = (vec3 *)archetype->positions.data;
    for(i32 i = range.start; i < range.end; ++i) {
        dest_positions.data[i].x = source_positions.data[i].x;
        dest_positions.data[i].y = source_positions.data[i].y;
        dest_positions.data[i].z = source_positions.data[i].z;
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
i32 gameArchetypeCheckCollisions(i32 na, vec4 *boxa, i32 nb, vec4 *boxb) {
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

i32 archetypeCheckCollision(vec3 *posa, vec3 *scla, const Range na, vec3 *posb, vec3 *sclb, const Range nb) {
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

/*
 * archetypeCheckCollision(A, B) -> bool
 * returns true if collision happened with A
 * side effect: does something to the collided B
 */
u8 archetypeProcessCollisions(GameArchetype *game_archetype, RangeArena *range_arena_game, i32 a_index, i32 b_index) {
    // check collisions
    i32 coll_id = archetypeCheckCollision(
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
        return true;
    }
    return false;
}

void archetypeUpdateTransforms(hkArray_vec3s positions, hkArray_vec3s rotations, hkArray_vec3s scales, hkArray_mat4s models, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        // S T R
        models.data[i] = glms_mat4_identity();
        models.data[i] = glms_translate(models.data[i], positions.data[i]);
        models.data[i] = glms_rotate(models.data[i], rotations.data[i].x, (vec3s){1.f, 0.f, 0.f});
        models.data[i] = glms_rotate(models.data[i], rotations.data[i].y, (vec3s){0.f, 1.f, 0.f});
        models.data[i] = glms_rotate(models.data[i], rotations.data[i].z, (vec3s){0.f, 0.f, 1.f});
        models.data[i] = glms_scale(models.data[i], scales.data[i]);
    }
    return;
}

void archetypeIntegrateVelocity(hkArray_vec3s positions, hkArray_vec3s velocities, hkArray_f32 speeds, f32 delta_time, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        // transformations
        positions.data[i].x += velocities.data[i].x * speeds.data[i] * delta_time;
        positions.data[i].y += velocities.data[i].y * speeds.data[i] * delta_time;
    }
}

void archetypeRender(hkArray_u32 vaos, hkArray_u32 shader_programs, hkArray_u32 textures, hkArray_u32 index_counts, hkArray_mat4s models, mat4 view, mat4 proj, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        glUseProgram(shader_programs.data[i]);
        // uniforms
        u32 view_location = glGetUniformLocation(shader_programs.data[i], "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
        u32 proj_location = glGetUniformLocation(shader_programs.data[i], "proj");
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj[0]);
        u32 model_location = glGetUniformLocation(shader_programs.data[i], "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, &models.data[i].m00);
        // draw
        glBindVertexArray(vaos.data[i]);
        glBindTexture(GL_TEXTURE_2D, textures.data[i]);
        glDrawElements(GL_TRIANGLES, index_counts.data[i], GL_UNSIGNED_INT, 0);
    }
}

void archetypeSetUniform1f(u32 *shader_program_data, const Range range, const char *uniform_name, f32 uniform_value) {
    for(i32 i = range.start; i < range.end; ++i) {
        glUseProgram(shader_program_data[i]);
        // uniforms
        u32 uniform_location = glGetUniformLocation(shader_program_data[i], uniform_name);
        glUniform1f(uniform_location, uniform_value);
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

void archetypeSpawnProjectileAtEntityAuto(hkArray_vec3s positions, FireCore *fire_cores, const Range projectile_range, i32 source_entity_id) {
    // printf("fire_cores[source_entity_id].fire_index[i] = %d. ", *fire_cores[source_entity_id].fire_index);
    // printf("fire_cores[source_entity_id].fire_index[i + projectile_range.start] = %d. ", *fire_cores[source_entity_id].fire_index+projectile_range.start);
    // printf("address: %p\n", fire_cores[source_entity_id].fire_index);
    vec3s *source_position = positions.data;
    vec3s *dest_position = positions.data;

    if((fire_cores[source_entity_id].fire_counter % fire_cores[source_entity_id].fire_rate) == 0) {
        fire_cores[source_entity_id].fire_counter = 0;
        fire_cores[source_entity_id].fire_index = (fire_cores[source_entity_id].fire_index + 1) % projectile_range.length;
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start].x = source_position[source_entity_id].x;
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start].y = source_position[source_entity_id].y;
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start].z = source_position[source_entity_id].z;
    }
    fire_cores[source_entity_id].fire_counter += 1;
    return;
}


void archetypeSpawnProjectileAtEntityAI(hkArray_vec3s positions, FireCore *fire_cores, const Range projectile_range, i32 source_entity_id, i32 off) {
    vec3s *source_position = positions.data;
    vec3s *dest_position = positions.data;

    if((fire_cores[source_entity_id].fire_counter % fire_cores[source_entity_id].fire_rate) == 0 && fire_cores[source_entity_id].fire_active == true) {
        fire_cores[source_entity_id].fire_counter = 0;
        // fire start
        // printf("%d\n", projectile_range.start + off);
        // printf("%d\n", projectile_range.start);
        // printf("%d\n", projectile_range.length);
        fire_cores[source_entity_id].fire_index = (fire_cores[source_entity_id].fire_index + 1) % 3; // + projectile_range.length;
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start + off].x = source_position[source_entity_id].x;
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start + off].y = source_position[source_entity_id].y;
        dest_position[fire_cores[source_entity_id].fire_index + projectile_range.start + off].z = source_position[source_entity_id].z;
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
