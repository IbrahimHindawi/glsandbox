/*
 * SpaceShooter:
 * TODO(Ibrahim): implement collision system buffer (bump allocator) & processor (check tags and output collision indices)
 * TODO(Ibrahim): implement entity active/inactive flags optimization
 * TODO(Ibrahim): implement config initialization
 */

#ifdef _MSC_VER
    #define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include <glad/glad.h>

#include <cglm/vec3.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>

#include "texops.h"
#include "shops.h"
#include "fileops.h"
#include "renderops.h"
#include "meshops.h"
#include "rangeops.h"

#include "gameArchetype.h"
#include "core.h"
#include "hkArray.h"

// SYSTEM
///////////////////////////////////
// #define window_width 800
// #define window_height 640
#define window_width 1280
#define window_height 720
// #define window_width 1920
// #define window_height 1080
#define FOV 45
bool should_quit = false;
SDL_Window *window = NULL;
char fops_buffer[fops_buffer_size];

// TIME
///////////////////////////////////
#define FPS 60
const int frameTime = 1000 / FPS;
const float frameTimef32 = 1000.f / FPS;
int framePrevTime;
int frameDelay;

// ECS
///////////////////////////////////
#define MAX 1024
RangeArena *range_arena_game;
GameArchetype game_archetype;
GraphicsArchetype graphics_archetype;

RangeArena *range_arena_static;
GraphicsArchetype static_archetype;

// RENDER
///////////////////////////////////
u32 shader_program_ship;
u32 shader_program_starfield;
u32 shader_program_projectile;
u32 shader_program_box;
u32 texture;
u32 texture2;
mat4 view;
mat4 proj_persp;
mat4 proj_ortho;
f32 angle;

// camera
vec3 camera_position = {0};
vec3 camera_forward = {0};
vec3 camera_up = {0};
vec3 camera_right = {0};
vec3 camera_target = {0};
vec3 camera_direction = {0};

// GAME
///////////////////////////////////
u16 counter = 0;
u8 iter = 0;
u8 spawn_range = 2;

void setup() {
    // MESH
    /////////////////////
    MeshRawDataArray[Ship] = MeshDataInitialize(ship_vertices, sizeofarray(ship_vertices, f32), ship_indices, sizeofarray(ship_indices, u32));
    MeshRawDataArray[Streak] = MeshDataInitialize(streak_vertices, sizeofarray(streak_vertices, f32), streak_indices, sizeofarray(streak_indices, u32));
    MeshRawDataArray[PlaneXY] = MeshDataInitialize(planexy_vertices, sizeofarray(planexy_vertices, f32), planexy_indices, sizeofarray(planexy_indices, u32));
    MeshRawDataArray[PlaneYZ] = MeshDataInitialize(planeyz_vertices, sizeofarray(planeyz_vertices, f32), planeyz_indices, sizeofarray(planeyz_indices, u32));
    MeshRawDataArray[PlaneZX] = MeshDataInitialize(planezx_vertices, sizeofarray(planezx_vertices, f32), planezx_indices, sizeofarray(planezx_indices, u32));
    MeshRawDataArray[Box] = MeshDataInitialize(box_vertices, sizeofarray(box_vertices, f32), box_indices, sizeofarray(box_indices, u32));

    MeshVAOGen(&MeshVAOArray[Ship], &MeshRawDataArray[Ship]);
    MeshVAOGen(&MeshVAOArray[Streak], &MeshRawDataArray[Streak]);
    MeshVAOGen(&MeshVAOArray[PlaneXY], &MeshRawDataArray[PlaneXY]);
    MeshVAOGen(&MeshVAOArray[PlaneYZ], &MeshRawDataArray[PlaneYZ]);
    MeshVAOGen(&MeshVAOArray[PlaneZX], &MeshRawDataArray[PlaneZX]);
    MeshVAOGen(&MeshVAOArray[Box], &MeshRawDataArray[Box]);

    //  SHADER
    /////////////////////////////////////////////
    shader_create(&shader_program_ship, "resource/simple.vert", "resource/simple.frag");
    shader_create(&shader_program_starfield, "resource/simple.vert", "resource/starfield.frag");
    shader_create(&shader_program_projectile, "resource/simple.vert", "resource/projectile.frag");
    shader_create(&shader_program_box, "resource/simple.vert", "resource/box.frag");

    // TEXTURE
    /////////////////////////////////////////////
    texture_create(&texture, "resource/Ship_BaseColor_purple_mid.jpg");
    texture_create(&texture2, "resource/Ship_BaseColor_orange_mid.jpg");

    // XFORMS
    //////////////////////////////////////////////
    camera_position[2] = 10.0f;
    // camera look at dir
    glm_vec3_sub(camera_direction, camera_position, camera_direction);
    glm_vec3_normalize(camera_direction);
    // camera right
    glm_vec3_cross((vec3){0.0f, 1.0f, 0.0f}, camera_direction, camera_right);
    glm_vec3_normalize(camera_right);
    // camera up
    glm_vec3_cross(camera_direction, camera_right, camera_up);
    camera_forward[2] = -1.0f;

    glm_mat4_identity(view);
    glm_mat4_identity(proj_persp);
    glm_perspective(glm_rad((f32)FOV), (f32)window_width / (f32)window_height, 0.1f, 100.0f, proj_persp);
    glm_ortho_default((f32)window_width / (f32)window_height, proj_ortho);

    //  setup(allocations)
    //-------------------------------------------
    // probably can replace all of these allocs
    // with an arena allocator
    gameArchetypeAllocate(&game_archetype, MAX);
    graphicsArchetypeAllocate(&graphics_archetype, MAX);
    graphicsArchetypeAllocate(&static_archetype, MAX);
    range_arena_game = rangeArenaAllocate(MAX);
    range_arena_static = rangeArenaAllocate(MAX);

    // simple hash indexing
    // const i32 table_length = 8;
    // hkHashTableLinearNode *nodes = hkHashTableLinearCreate(table_length);
    // hkHashTableLinearInitialize(nodes, table_length);
    // hkHashTableLinearSet(nodes, "hero", 0);
    // hkHashTableLinearSet(nodes, "enemy", 1);
    // hkHashTableLinearNode *result = hkHashTableLinearSearch(nodes, "hero", table_length);
    // printf("node={key:%s, value:%d}\n", result->key, result->value);
    // hkHashTableLinearNode *result2 = hkHashTableLinearSearch(nodes, "enemy", table_length);
    // printf("node={key:%s, value:%d}\n", result2->key, result2->value);

    // setup: game_archetype & graphics_archetype
    i32 hero_arena_index = rangeArenaAppend(range_arena_game, "hero", 1);
    rangeArenaPrint(range_arena_game, "hero");
    Range *hero_range = &((Range *)range_arena_game->ranges.data)[hero_arena_index];
    archetypeInitializeMesh((u32 *)game_archetype.vaos.data, 
            (u32 *)game_archetype.index_counts.data, *hero_range, Box);
    archetypeInitialize1u((u32 *)game_archetype.shaders.data, *hero_range, shader_program_box);
    archetypeInitialize1u((u32 *)game_archetype.textures.data, *hero_range, texture);
    archetypeInitialize3f((vec3 *)game_archetype.positions.data, *hero_range, (vec3){0.f, -3.f, 0.f});
    archetypeInitialize3f((vec3 *)game_archetype.rotations.data, *hero_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)game_archetype.scales.data, *hero_range, (vec3){0.35f, 0.35f, 0.35f});
    archetypeInitialize1f((f32 *)game_archetype.speeds.data, *hero_range, 6.f);
    archetypeInitializeMesh((u32 *)graphics_archetype.vaos.data, 
            (u32 *)graphics_archetype.index_counts.data, *hero_range, Ship);
    archetypeInitialize1u((u32 *)graphics_archetype.shaders.data, *hero_range, shader_program_ship);
    archetypeInitialize1u((u32 *)graphics_archetype.textures.data, *hero_range, texture);
    archetypeInitialize3f((vec3 *)graphics_archetype.positions.data, *hero_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)graphics_archetype.rotations.data, *hero_range, (vec3){pi * .5f, pi, 0.f});
    archetypeInitialize3f((vec3 *)graphics_archetype.scales.data, *hero_range, (vec3){0.15f, 0.15f, 0.15f});

    i32 hero_projectiles_arena_index = rangeArenaAppend(range_arena_game, "hero_projectiles", 8);
    rangeArenaPrint(range_arena_game, "hero_projectiles");
    Range *projectile_range = &((Range *)range_arena_game->ranges.data)[hero_projectiles_arena_index];
    archetypeInitializeMesh((u32 *)game_archetype.vaos.data, 
                            (u32 *)game_archetype.index_counts.data, *projectile_range, Box);
    archetypeInitialize1u((u32 *)game_archetype.shaders.data, *projectile_range, shader_program_box);
    archetypeInitialize1u((u32 *)game_archetype.textures.data, *projectile_range, texture);
    archetypeInitialize3f((vec3 *)game_archetype.positions.data, *projectile_range, (vec3){-100.f, -100.f, -100.f});
    archetypeInitialize3f((vec3 *)game_archetype.rotations.data, *projectile_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)game_archetype.scales.data, *projectile_range, (vec3){.15f, .5f, .15f});
    archetypeInitialize3f((vec3 *)game_archetype.velocities.data, *projectile_range, (vec3){0.f, 1.f, 0.f});
    archetypeInitialize1f((f32 *)game_archetype.speeds.data, *projectile_range, 8.f);
    archetypeInitializeMesh((u32 *)graphics_archetype.vaos.data, 
                            (u32 *)graphics_archetype.index_counts.data, *projectile_range, Streak);
    archetypeInitialize1u((u32 *)graphics_archetype.shaders.data, *projectile_range, shader_program_projectile);
    archetypeInitialize1u((u32 *)graphics_archetype.textures.data, *projectile_range, texture);
    archetypeInitialize3f((vec3 *)graphics_archetype.positions.data, *projectile_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)graphics_archetype.rotations.data, *projectile_range, (vec3){-1.f * pi * .5f, pi, 0.f});
    archetypeInitialize3f((vec3 *)graphics_archetype.scales.data, *projectile_range, (vec3){.25f, 1.f, .25f});

    i32 enemy_arena_index = rangeArenaAppend(range_arena_game, "enemy", 10);
    rangeArenaPrint(range_arena_game, "enemy");
    Range *enemy_range = &((Range *)range_arena_game->ranges.data)[enemy_arena_index];
    archetypeInitializeMesh((u32 *)game_archetype.vaos.data, 
        (u32 *)game_archetype.index_counts.data, *enemy_range, Box);
    archetypeInitialize1u((u32 *)game_archetype.shaders.data, *enemy_range, shader_program_box);
    archetypeInitialize1u((u32 *)game_archetype.textures.data, *enemy_range, texture);
    archetypeInitialize3f((vec3 *)game_archetype.positions.data, *enemy_range, (vec3){0.f, -10.f, 0.f});
    archetypeInitialize3f((vec3 *)game_archetype.rotations.data, *enemy_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)game_archetype.scales.data, *enemy_range, (vec3){.35f, .35f, .35f});
    archetypeInitialize3f((vec3 *)game_archetype.velocities.data, *enemy_range, (vec3){0.f, -1.f, 0.f});
    archetypeInitialize1f((f32 *)game_archetype.speeds.data, *enemy_range, 4.f);
    archetypeInitializeMesh((u32 *)graphics_archetype.vaos.data, 
        (u32 *)graphics_archetype.index_counts.data, *enemy_range, Ship);
    archetypeInitialize1u((u32 *)graphics_archetype.shaders.data, *enemy_range, shader_program_ship);
    archetypeInitialize1u((u32 *)graphics_archetype.textures.data, *enemy_range, texture2);
    archetypeInitialize3f((vec3 *)graphics_archetype.positions.data, *enemy_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)graphics_archetype.rotations.data, *enemy_range, (vec3){pi * .5f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)graphics_archetype.scales.data, *enemy_range, (vec3){.15f, .15f, .15f});
    for(int i = 0; i < 10; ++i) {
        ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start + i].fire_rate = 30;
        ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start + i].fire_burst_count = 3;
        ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start + i].fire_burst_rate = 3;
        ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start + i].fire_counter = 10;
        ((vec3 *)game_archetype.positions.data)[enemy_range->start + i][0] = i * 0.4f -2.f;
    }
    // archetypeInitializePositionsAsLine((vec3 *)game_archetype.positions.data, 2.f, 1.f, ((Range *)range_arena_game->ranges.data)[enemy_arena_index]);

    i32 enemy_projectiles_arena_index = rangeArenaAppend(range_arena_game, "enemy_projectiles", 3 * 10);
    rangeArenaPrint(range_arena_game, "enemy_projectiles");
    Range *projectile_enemy_range = &((Range *)range_arena_game->ranges.data)[enemy_projectiles_arena_index];
    archetypeInitializeMesh((u32 *)game_archetype.vaos.data, 
        (u32 *)game_archetype.index_counts.data, *projectile_enemy_range, Box);
    archetypeInitialize1u((u32 *)game_archetype.shaders.data, *projectile_enemy_range, shader_program_box);
    archetypeInitialize1u((u32 *)game_archetype.textures.data, *projectile_enemy_range, texture);
    archetypeInitialize3f((vec3 *)game_archetype.positions.data, *projectile_enemy_range, (vec3){-100.f, -100.f, -100.f});
    archetypeInitialize3f((vec3 *)game_archetype.rotations.data, *projectile_enemy_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)game_archetype.scales.data, *projectile_enemy_range, (vec3){.15f, .5f, .15f});
    archetypeInitialize3f((vec3 *)game_archetype.velocities.data, *projectile_enemy_range, (vec3){0.f, -1.f, 0.f});
    archetypeInitialize1f((f32 *)game_archetype.speeds.data, *projectile_enemy_range, 8.f);
    archetypeInitializeMesh((u32 *)graphics_archetype.vaos.data, 
        (u32 *)graphics_archetype.index_counts.data, *projectile_enemy_range, Streak);
    archetypeInitialize1u((u32 *)graphics_archetype.shaders.data, *projectile_enemy_range, shader_program_projectile);
    archetypeInitialize1u((u32 *)graphics_archetype.textures.data, *projectile_enemy_range, texture);
    archetypeInitialize3f((vec3 *)graphics_archetype.positions.data, *projectile_enemy_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)graphics_archetype.rotations.data, *projectile_enemy_range, (vec3){-1.f * pi * .5f, pi, 0.f});
    archetypeInitialize3f((vec3 *)graphics_archetype.scales.data, *projectile_enemy_range, (vec3){.25f, 1.f, .25f});

    // setup(static_archetype)
    i32 background = rangeArenaAppend(range_arena_static, "background", 1);
    rangeArenaPrint(range_arena_static, "background");
    Range *static_range = &((Range *)range_arena_static->ranges.data)[background];
    // Graphics
    archetypeInitializeMesh((u32 *)static_archetype.vaos.data, 
        (u32 *)static_archetype.index_counts.data, *static_range, PlaneXY);
    archetypeInitialize1u((u32 *)static_archetype.shaders.data, *static_range, shader_program_starfield);
    archetypeInitialize1u((u32 *)static_archetype.textures.data, *static_range, texture);
    archetypeInitialize3f((vec3 *)static_archetype.positions.data, *static_range, (vec3){0.f, 0.f, -89.f});
    archetypeInitialize3f((vec3 *)static_archetype.rotations.data, *static_range, (vec3){0.f, 0.f, 0.f});
    archetypeInitialize3f((vec3 *)static_archetype.scales.data, *static_range, (vec3){10.f, 10.f, 10.f});

    return;
}

void input() {
    vec3 *velocities = ((vec3 *)game_archetype.velocities.data);
    i32 hero_arena_index = rangeArenaGet(range_arena_game, "hero");
    i32 hero_projectiles_arena_index = rangeArenaGet(range_arena_game, "hero_projectiles");
    const i32 hero_start = ((Range *)range_arena_game->ranges.data)[hero_arena_index].start;
    const i32 hero_end = ((Range *)range_arena_game->ranges.data)[hero_arena_index].end;
    const f32 base = 1.f;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                should_quit = true;
                break;
            }
            case SDL_KEYDOWN: {
                if(event.key.keysym.sym == SDLK_ESCAPE) {
                    should_quit = true;
                } else if(event.key.keysym.sym == SDLK_a) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        velocities[hero_start][0] = -base;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        velocities[hero_start][0] = base;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        velocities[hero_start][1] = base;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        velocities[hero_start][1] = -base;
                    }
                } else if(event.key.keysym.sym == SDLK_SPACE) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        // rangeArenaPrint(range_arena_game, "background");
                        archetypeSpawnProjectileAtEntity(
                            (vec3 *)game_archetype.positions.data,
                            &((FireCore *)game_archetype.fire_cores.data)[hero_arena_index].fire_index,
                            ((Range *)range_arena_game->ranges.data)[hero_projectiles_arena_index],
                            hero_start);
                    }
                }
                break;
            }
            case SDL_KEYUP: {
                if(event.key.keysym.sym == SDLK_ESCAPE) {
                    should_quit = true;
                } else if(event.key.keysym.sym == SDLK_a) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        if(velocities[hero_start][0] < 0.f) velocities[hero_start][0] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        if(velocities[hero_start][0] > 0.f) velocities[hero_start][0] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        if(velocities[hero_start][1] > 0.f) velocities[hero_start][1] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        if(velocities[hero_start][1] < 0.f) velocities[hero_start][1] = 0.f;
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    }
    return;
}

void update() {
    // Prologue
    // delta time
    frameDelay = frameTime - (SDL_GetTicks() - framePrevTime);
    if(frameDelay > 0) {
        SDL_Delay(frameDelay);
    }
    float delta_time = (SDL_GetTicks() - framePrevTime) / 1000.f;
    if (delta_time > frameTime) {
        delta_time = frameTimef32;
    }
    // printf("ticks: %d, ", SDL_GetTicks());
    framePrevTime = SDL_GetTicks();
    // printf("FPS: %f.\n", 999.f / (1000.f * delta_time));

    // camera
    vec3 camera_new_location;
    glm_vec3_add(camera_position, camera_forward, camera_new_location);
    glm_lookat(camera_position, camera_new_location, camera_up, view);

    // printf("%f\n", SDL_GetTicks() / 1000.f);
    // printf("%d\n", (int)(SDL_GetTicks() / 1000.f) % 3);
    // printf("%d\n", (int)(SDL_GetTicks()));
    // sec = (int)(SDL_GetTicks() / 1000.f) % 3;
    // if(sec == 2) {
    //     printf("OK\n");
    // }

    // update attributes
    // update(game_archetype|graphics_archetype)
    i32 hero_arena_index = rangeArenaGet(range_arena_game, "hero");
    i32 hero_projectiles_arena_index = rangeArenaGet(range_arena_game, "hero_projectiles");
    i32 enemy_arena_index = rangeArenaGet(range_arena_game, "enemy");
    i32 enemy_projectiles_arena_index = rangeArenaGet(range_arena_game, "enemy_projectiles");

    for(int i = 0; i < 10; ++i) {
        // TODO(Ibrahim): fix function internals
        archetypeSpawnProjectileAtEntityAI(
            (vec3 *)game_archetype.positions.data,
            (FireCore *)game_archetype.fire_cores.data, 
            ((Range *)range_arena_game->ranges.data)[enemy_projectiles_arena_index],
            ((Range *)range_arena_game->ranges.data)[enemy_arena_index].start + i, i * 3);
    }

    Range total_range = (Range){ .start = 0, .end = range_arena_game->border };

    // keep hero in box
    vec3 *hero_position = &((vec3 *)game_archetype.positions.data)[((Range *)range_arena_game->ranges.data)[hero_arena_index].start];
    if ((*hero_position)[0] > 3.f){
        (*hero_position)[0] = 3.f;
    } else if ((*hero_position)[0] < -3.f) {
        (*hero_position)[0] = -3.f;
    } else if ((*hero_position)[1] > 4.f) {
        (*hero_position)[1] = 4.f;
    } else if ((*hero_position)[1] < -4.f) {
        (*hero_position)[1] = -4.f;
    }

    // Collision Detection
    u8 hit_hero_enemy = archetypeProcessCollisions(&game_archetype, range_arena_game, hero_arena_index, enemy_arena_index);
    u8 hit_hero_eproj = archetypeProcessCollisions(&game_archetype, range_arena_game, hero_arena_index, enemy_projectiles_arena_index);
    if (hit_hero_enemy || hit_hero_eproj) {
        printf("Hero Lives - 1\n");
        // hero_lives -= 1;
        // reset hero position
    }
    archetypeProcessCollisions(&game_archetype, range_arena_game, hero_projectiles_arena_index, enemy_arena_index);

    counter += 1;
    if(counter == 50) {
        // printf("OK\n");
        // printf("iter = %d\n", iter);
        ((vec3 *)game_archetype.positions.data)[((Range *)range_arena_game->ranges.data)[enemy_arena_index].start + iter][0] = (float)((rand() % (spawn_range - -spawn_range + 1)) + -spawn_range);
        ((vec3 *)game_archetype.positions.data)[((Range *)range_arena_game->ranges.data)[enemy_arena_index].start + iter][1] = 8.f;
        ((vec3 *)game_archetype.positions.data)[((Range *)range_arena_game->ranges.data)[enemy_arena_index].start + iter][2] = 0.f;
        counter = 0;
        iter += 1;
        if(iter % 10 == 0) { iter = 0; }
    }

    // Epilogue
    // integrate movement
    archetypeIntegrateVelocity(
        (vec3 *)game_archetype.positions.data,
        (vec3 *)game_archetype.velocities.data,
        (f32 *)game_archetype.speeds.data,
        delta_time,
        total_range);
    archetypeCopyVector(
        (vec3 *)game_archetype.positions.data,
        (vec3 *)graphics_archetype.positions.data,
        total_range);

    // finalize transformation matrices
    archetypeUpdateTransforms(
        (vec3 *)game_archetype.positions.data,
        (vec3 *)game_archetype.rotations.data,
        (vec3 *)game_archetype.scales.data,
        (mat4 *)game_archetype.models.data,
        total_range);
    archetypeUpdateTransforms(
        (vec3 *)graphics_archetype.positions.data,
        (vec3 *)graphics_archetype.rotations.data,
        (vec3 *)graphics_archetype.scales.data,
        (mat4 *)graphics_archetype.models.data,
        total_range);

    // update(static_archetype)
    Range static_range = (Range){ .start = 0, .end = range_arena_static->border };
    archetypeUpdateTransforms(
        (vec3 *)static_archetype.positions.data,
        (vec3 *)static_archetype.rotations.data,
        (vec3 *)static_archetype.scales.data,
        (mat4 *)static_archetype.models.data,
        static_range);
}

void render() {
    // begin
    glClearColor(.05f, .05f, .05f, 1.f);
    // ropsCheckError_();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // bind
    // render(graphics_archetype|game_archetype)
    archetypeRender(
        (u32 *)graphics_archetype.vaos.data,
        (u32 *)graphics_archetype.shaders.data,
        (u32 *)graphics_archetype.textures.data,
        (u32 *)graphics_archetype.index_counts.data,
        (mat4 *)graphics_archetype.models.data,
        view, proj_persp, (Range){0, range_arena_game->border});
    archetypeRenderWires(
        (u32 *)game_archetype.vaos.data,
        (u32 *)game_archetype.shaders.data,
        (u32 *)game_archetype.textures.data,
        (u32 *)game_archetype.index_counts.data,
        (mat4 *)game_archetype.models.data,
        view, proj_persp, (Range){0, range_arena_game->border});
    // render(static_archetype)
    archetypeSetUniform1f(
        (u32 *)static_archetype.shaders.data,
        (Range){0, range_arena_static->border},
        "time", SDL_GetTicks() / 1000.f);
    archetypeRender(
        (u32 *)static_archetype.vaos.data,
        (u32 *)static_archetype.shaders.data,
        (u32 *)static_archetype.textures.data,
        (u32 *)static_archetype.index_counts.data,
        (mat4 *)static_archetype.models.data,
        view, proj_ortho, (Range){0, range_arena_static->border});
    // end
    SDL_GL_SwapWindow(window);
}

int main(int argc, char *argv[]) {
    /*
     * Initialises the SDL video subsystem (as well as the events subsystem).
     * Returns 0 on success or a negative error code on failure using SDL_GetError().
     */
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    // INITIALIZEOPENGL
    //-------------------------------------------
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);


    // Creates a SDL window
    //-------------------------------------------
    window = SDL_CreateWindow("glsandbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_OPENGL);

    // Checks if window has been created; if not, exits program
    //-------------------------------------------
    if (window == NULL) {
        fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "Failed to load GL: %s\n", SDL_GetError());
        return 1;
    }

    puts("OpenGL Loaded Successfully.");
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));

    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        printf("GL Debug Mode Enabled!\n");
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(glDebugOutput, NULL);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }

    setup();
    while (!should_quit) {
        input();
        update();
        render();
    }

    /* Frees memory */
    SDL_DestroyWindow(window);
    gameArchetypeDeallocate(&game_archetype);
    // gameArchetypeDeallocate(&game_archetype);
    // gameArchetypeDeallocate(&game_archetype);

    /* Shuts down all SDL subsystems */
    SDL_Quit(); 
    return 0;
}
