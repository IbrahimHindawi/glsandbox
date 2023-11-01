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
    // graphics
    hkArray vao; // u32
    hkArray index_count; // u32
    hkArray shader_program; // u32
    hkArray texture; // u32
    // game
    hkArray speed; // f32 
    hkArray velocity; // vec3
    hkArray position; // vec3
    hkArray rotation; // vec3
    hkArray scale; // vec3
    hkArray model; // mat4
    hkArray box; // mat4
    hkArray vao_box_collider; // mat4
    hkArray box_index_count; // mat4
} GameArchetype;

void gameArchetypeAllocate(GameArchetype *archetype, i32 n) {
    // game
    archetype->vao = hkArrayCreate(sizeof(u32), n);
    archetype->index_count = hkArrayCreate(sizeof(u32), n);
    archetype->shader_program = hkArrayCreate(sizeof(u32), n);
    archetype->texture = hkArrayCreate(sizeof(u32), n);
    // graphics
    archetype->speed = hkArrayCreate(sizeof(f32), n);
    archetype->velocity = hkArrayCreate(sizeof(vec3), n);
    archetype->position = hkArrayCreate(sizeof(vec3), n);
    archetype->rotation = hkArrayCreate(sizeof(vec3), n);
    archetype->scale = hkArrayCreate(sizeof(vec3), n);
    archetype->model = hkArrayCreate(sizeof(mat4), n);
}

void gameArchetypeDeallocate(GameArchetype *archetype) {
    // game
    hkArrayDestroy(&archetype->vao);
    hkArrayDestroy(&archetype->index_count);
    hkArrayDestroy(&archetype->shader_program);
    hkArrayDestroy(&archetype->texture);
    // graphics
    hkArrayDestroy(&archetype->speed);
    hkArrayDestroy(&archetype->velocity);
    hkArrayDestroy(&archetype->position);
    hkArrayDestroy(&archetype->rotation);
    hkArrayDestroy(&archetype->scale);
    hkArrayDestroy(&archetype->model);
}

typedef struct {
    // graphics
    hkArray vao; // u32
    hkArray index_count; // u32
    hkArray shader_program; // u32
    hkArray texture; // u32
    // game
    hkArray position; // vec3
    hkArray rotation; // vec3
    hkArray scale; // vec3
    hkArray model; // mat4
} BoxArchetype;

void boxArchetypeAllocate(BoxArchetype *archetype, i32 n) {
    // game
    archetype->vao = hkArrayCreate(sizeof(u32), n);
    archetype->index_count = hkArrayCreate(sizeof(u32), n);
    archetype->shader_program = hkArrayCreate(sizeof(u32), n);
    archetype->texture = hkArrayCreate(sizeof(u32), n);
    // graphics
    archetype->position = hkArrayCreate(sizeof(vec3), n);
    archetype->rotation = hkArrayCreate(sizeof(vec3), n);
    archetype->scale = hkArrayCreate(sizeof(vec3), n);
    archetype->model = hkArrayCreate(sizeof(mat4), n);
}

void boxArchetypeDeallocate(BoxArchetype *archetype) {
    // game
    hkArrayDestroy(&archetype->vao);
    hkArrayDestroy(&archetype->index_count);
    hkArrayDestroy(&archetype->shader_program);
    hkArrayDestroy(&archetype->texture);
    // graphics
    hkArrayDestroy(&archetype->position);
    hkArrayDestroy(&archetype->rotation);
    hkArrayDestroy(&archetype->scale);
    hkArrayDestroy(&archetype->model);
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
    const i32 f = (i32)sqrt((f64)archetype->index_count.length);
    for(i32 i = 0; i < f; ++i) {
        for(i32 j = 0; j < f; ++j) {
            b += 1.f;
            vec3 *positionvector = archetype->position.data;
            positionvector[i + j * f][0] = a * s - f/2;
            positionvector[i + j * f][1] = b * s - f/2;
            // positionvector[i + j * f][2] = -50.f;
            // ((vec3 *)archetype->velocity.data)[i][0] = -1.f;
            // ((vec3 *)archetype->velocity.data)[i][1] = 0.f;
            // archetype->position.data[i + j * f][0] = a * s - f/2;
            // meshes.position[i + j * f][1] = b * s - f/2;
            // meshes.position[i + j * f][2] = 0.f;
            printf("{%f, %f}\n", a*s, b*s);
        }
        a += 1.f;
        b = -1.f;
    }
}

void gameArchetypeInitializeCollisionBoxes(GameArchetype *archetype, const f32 box_width, const f32 box_height, const f32 box_xoffset, const f32 box_yoffset) {
    const i64 n = archetype->index_count.length;
    vec4 *box = (vec4 *)archetype->box.data;
    vec3 *scale = (vec3 *)archetype->scale.data;
    const vec3 *position = (vec3 *)archetype->position.data;
    for(i32 i = 0; i < n; ++i) {
        box[i][0] = position[i][0] + box_xoffset;
        box[i][1] = position[i][1] + box_yoffset;
        box[i][2] = box_width * scale[i][0];
        box[i][3] = box_height * scale[i][0];
// #ifdef DEBUG
// #endif DEBUG
        // printf("{%f, %f, %f, %f}\n", box[i][0], box[i][1], box[i][2], box[i][3]);
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
    vec3 *rotation = ((vec3 *)archetype->rotation.data);
    const i32 n = (i32)archetype->index_count.length;
    for(i32 i = 0; i < n; ++i) {
        rotation[i][0] = r[0];
        rotation[i][1] = r[1];
        rotation[i][2] = r[2];
        // printf("step %f\n", a);
    }
}

void gameArchetypeInitializeScales(GameArchetype *archetype, vec3 s) {
    vec3 *scale = ((vec3 *)archetype->scale.data);
    const i32 n = (i32)archetype->index_count.length;
    for(i32 i = 0; i < n; ++i) {
        scale[i][0] = s[0];
        scale[i][1] = s[1];
        scale[i][2] = s[2];
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

void archetypeInitializeVelocities(GameArchetype *archetype, vec3 v, const Range range) {
    // initalize velocity
    vec3 *velocity = (vec3 *)archetype->velocity.data;
    const i32 n = (i32)archetype->index_count.length;
    for(i32 i = range.start; i < range.end; ++i) {
        velocity[i][0] = v[0];
        velocity[i][1] = v[1];
        velocity[i][2] = v[2];
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

void archetypeUpdateVelocities(GameArchetype *archetype, f32 time, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        // printf("%f\n", new_velocity);
        ((vec3 *)archetype->velocity.data)[i][0] = (f32)sin(time);
    }
}

void archetypeCopyVector(vec3 *source_position_data, vec3 *dest_position_data, const Range range) {
    // vec4 *box = (vec4 *)archetype->box.data;
    // const vec3 *position = (vec3 *)archetype->position.data;
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

// vec4 box {x, y, w, h}
int boxAABBCollision2(vec3 posa, vec3 scla, vec3 posb, vec3 sclb) {
    if (posa[0] < posb[0] + sclb[0] &&
        posa[0] + scla[0] > posb[0] &&
        posa[1] < posb[1] + sclb[2] &&
        posa[1] + scla[2] > posb[1]) {
        return 1;
    } else {
        return 0;
    }
}

// i32 gameArchetypeCheckCollisions(GameArchetype *archetypeA, GameArchetype *archetypeB) {
i32 gameArchetypeCheckCollisions(u32 na, vec4 *boxa, u32 nb, vec4 *boxb) {
    /*
    const i64 na = archetypeA->index_count.length;
    vec4 *boxa = (vec4 *)archetypeA->box.data;

    const i64 nb = archetypeB->index_count.length;
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

i32 gameArchetypeCheckCollisions2(vec3 *posa, vec3 *scla, Range na, vec3 *posb, vec3 *sclb, Range nb) {// u32 na, vec4 *boxa, u32 nb, vec4 *boxb) {
    /*
    const i64 na = archetypeA->index_count.length;
    vec4 *boxa = (vec4 *)archetypeA->box.data;

    const i64 nb = archetypeB->index_count.length;
    vec4 *boxb = (vec4 *)archetypeB->box.data;
    */
    // printf("%lld, %lld\n", na, nb);
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

void archetypeIntegrateVelocity(vec3 *position_data, vec3 *velocity_data, f32 *speed_data, f32 deltaTime, const Range range) {
    for(i32 i = range.start; i < range.end; ++i) {
        // transformations
        position_data[i][0] += velocity_data[i][0] * speed_data[i] * deltaTime;
        position_data[i][1] += velocity_data[i][1] * speed_data[i] * deltaTime;
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

void gameArchetypeRenderBoxes(GameArchetype *archetype, u32 shader_program, mat4 view, mat4 proj, u32 texture) {
    const i64 n = archetype->index_count.length; 
    for(i32 i = 0; i < n; ++i) {
        glUseProgram(shader_program);
        // uniforms
        u32 view_location = glGetUniformLocation(shader_program, "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
        u32 proj_location = glGetUniformLocation(shader_program, "proj");
        mat4 *model = ((mat4 *)archetype->model.data);
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj[0]);
        u32 model_location = glGetUniformLocation(shader_program, "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, ((mat4 *)archetype->model.data)[i][0]);
        // draw
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBindVertexArray(((u32 *)archetype->vao_box_collider.data)[i]);
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void gameArchetypeRenderBG(GameArchetype *archetype, u32 shader_program, mat4 view, mat4 proj) {
    const i64 n = archetype->index_count.length; 
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
        glUniformMatrix4fv(model_location, 1, GL_FALSE, ((mat4 *)archetype->model.data)[i][0]);
        // draw
        glBindVertexArray(((u32 *)archetype->vao.data)[i]);
        // glBindTexture(GL_TEXTURE_2D, texture);
        glDrawElements(GL_TRIANGLES, ((u32 *)archetype->index_count.data)[i], GL_UNSIGNED_INT, 0);
    }
}

void archetypeSpawnProjectileAtEntity(const vec3 *source_position, i32 id, vec3 *dest_position, i32 offset, const i32 projectile_length) {
    static i32 current_projectile_pool_index = 0;
    current_projectile_pool_index = (current_projectile_pool_index + 1) % projectile_length + offset;
    dest_position[current_projectile_pool_index][0] = source_position[id][0];
    dest_position[current_projectile_pool_index][1] = source_position[id][1];
    dest_position[current_projectile_pool_index][2] = source_position[id][2];
    return;
}

/*
void gameSpawnProjectileAtEntity(const GameArchetype *archetype, GameArchetype *archetypeProjectile, i32 id) {
    vec3 *archetypeposition = ((vec3 *)archetype->position.data);
    vec3 *projectile_position = ((vec3 *)archetypeProjectile->position.data);
    const i64 projectile_length = archetypeProjectile->index_count.length; 
    static i32 current_projectile_pool_index = 0;
    projectile_position[current_projectile_pool_index][0] = archetypeposition[id][0];
    projectile_position[current_projectile_pool_index][1] = archetypeposition[id][1];
    projectile_position[current_projectile_pool_index][2] = archetypeposition[id][2];
    current_projectile_pool_index = (current_projectile_pool_index + 1) % projectile_length;
    // printf("%d\n", current_projectile_pool_index);
    return;
}

void gameArchetypeSpawnProjectile(const GameArchetype *archetype, GameArchetype *archetypeProjectile) {
    const i64 n = archetype->index_count.length; 
    for(i32 i = 0; i < n; ++i) {
        gameSpawnProjectileAtEntity(archetype, archetypeProjectile, i);
    }
    return;
}
*/
