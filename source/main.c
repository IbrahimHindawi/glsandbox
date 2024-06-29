/*
 * SpaceShooter:
 * TODO(Ibrahim): implement collision system buffer (bump allocator) & processor (check tags and output collision indices)
 * TODO(Ibrahim): implement entity active/inactive flags optimization
 * TODO(Ibrahim): implement entity health
 * TODO(Ibrahim): implement config initialization
 */

#ifdef _MSC_VER
    #define SDL_MAIN_HANDLED
#endif
#include <SDL2/SDL.h>

#include <glad/glad.h>

#include <cglm/struct.h>

#include "texops.h"
#include "shops.h"
#include "fileops.h"
#include "renderops.h"
#include "meshops.h"

#include "gameArchetype.h"
#include <core.h>
#include <hkArray.h>

#include "rangeops.h"

// SYSTEM
///////////////////////////////////
// #define window_width_init 800
// #define window_height_init 640
#define window_width_init 1280
#define window_height_init 720
// #define window_width_init 1920
// #define window_height_init 1080
#define FOV 45
bool should_quit = false;
SDL_DisplayMode display_mode;
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
u32 shader_program_projectile_red;
u32 shader_program_box;
u32 shader_program_ui_bg;
u32 texture;
u32 texture2;
u32 texture_gradient;
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
    MeshRawDataArray[Ship] = MeshDataInitialize(ship_vertices, sizeofarray(ship_vertices), ship_indices, sizeofarray(ship_indices));
    MeshRawDataArray[Streak] = MeshDataInitialize(streak_vertices, sizeofarray(streak_vertices), streak_indices, sizeofarray(streak_indices));
    MeshRawDataArray[PlaneXY] = MeshDataInitialize(planexy_vertices, sizeofarray(planexy_vertices), planexy_indices, sizeofarray(planexy_indices));
    MeshRawDataArray[PlaneYZ] = MeshDataInitialize(planeyz_vertices, sizeofarray(planeyz_vertices), planeyz_indices, sizeofarray(planeyz_indices));
    MeshRawDataArray[PlaneZX] = MeshDataInitialize(planezx_vertices, sizeofarray(planezx_vertices), planezx_indices, sizeofarray(planezx_indices));
    MeshRawDataArray[Box] = MeshDataInitialize(box_vertices, sizeofarray(box_vertices), box_indices, sizeofarray(box_indices));

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
    shader_create(&shader_program_projectile_red, "resource/simple.vert", "resource/projectile_red.frag");
    shader_create(&shader_program_box, "resource/simple.vert", "resource/box.frag");
    shader_create(&shader_program_ui_bg, "resource/simple.vert", "resource/ui_bg.frag");

    // TEXTURE
    /////////////////////////////////////////////
    texture_create(&texture, "resource/Ship_BaseColor_purple_mid.jpg");
    texture_create(&texture2, "resource/Ship_BaseColor_orange_mid.jpg");
    texture_create(&texture_gradient, "resource/Gradient.jpg");

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
    glm_perspective(glm_rad((f32)FOV), (f32)display_mode.w / (f32)display_mode.h, 0.1f, 100.0f, proj_persp);
    glm_ortho_default((f32)display_mode.w / (f32)display_mode.h, proj_ortho);

    //  setup(allocations)
    //-------------------------------------------
    // probably can replace all of these allocs
    // with an arena allocator
    gameArchetypeAllocate(&game_archetype, MAX);
    graphicsArchetypeAllocate(&graphics_archetype, MAX);
    graphicsArchetypeAllocate(&static_archetype, MAX);
    range_arena_game = rangeArenaAllocate(MAX);
    range_arena_static = rangeArenaAllocate(MAX);

    // setup: game_archetype & graphics_archetype
    i32 hero_arena_index = rangeArenaAppend(range_arena_game, "hero", 1);
    rangeArenaPrint(range_arena_game, "hero");
    Range *hero_range = &((Range *)range_arena_game->ranges.data)[hero_arena_index];
    archetypeInitializeMesh(game_archetype.vaos, game_archetype.index_counts, *hero_range, Box);
    archetypeInitialize1u(game_archetype.shaders, *hero_range, shader_program_box);
    archetypeInitialize1u(game_archetype.textures, *hero_range, texture);
    archetypeInitialize3f(game_archetype.positions, *hero_range, (vec3s){0.f, -3.f, 0.f});
    archetypeInitialize3f(game_archetype.rotations, *hero_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(game_archetype.scales, *hero_range, (vec3s){0.35f, 0.35f, 0.35f});
    archetypeInitialize1i(game_archetype.lives, *hero_range, 3);
    archetypeInitialize1f(game_archetype.speeds, *hero_range, 8.f);
    archetypeInitializeMesh(graphics_archetype.vaos, graphics_archetype.index_counts, *hero_range, Ship);
    archetypeInitialize1u(graphics_archetype.shaders, *hero_range, shader_program_ship);
    archetypeInitialize1u(graphics_archetype.textures, *hero_range, texture);
    archetypeInitialize3f(graphics_archetype.positions, *hero_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(graphics_archetype.rotations, *hero_range, (vec3s){pi * .5f, pi, 0.f});
    archetypeInitialize3f(graphics_archetype.scales, *hero_range, (vec3s){0.15f, 0.15f, 0.15f});
    ((FireCore *)game_archetype.fire_cores.data)[hero_range->start].fire_rate = 5;
    ((FireCore *)game_archetype.fire_cores.data)[hero_range->start].fire_burst_count = 0;
    ((FireCore *)game_archetype.fire_cores.data)[hero_range->start].fire_burst_rate = 0;
    ((FireCore *)game_archetype.fire_cores.data)[hero_range->start].fire_counter = 10;

    i32 hero_projectiles_arena_index = rangeArenaAppend(range_arena_game, "hero_projectiles", 16);
    rangeArenaPrint(range_arena_game, "hero_projectiles");
    Range *projectile_range = &((Range *)range_arena_game->ranges.data)[hero_projectiles_arena_index];
    archetypeInitializeMesh(game_archetype.vaos, game_archetype.index_counts, *projectile_range, Box);
    archetypeInitialize1u(game_archetype.shaders, *projectile_range, shader_program_box);
    archetypeInitialize1u(game_archetype.textures, *projectile_range, texture);
    archetypeInitialize3f(game_archetype.positions, *projectile_range, (vec3s){-100.f, -100.f, -100.f});
    archetypeInitialize3f(game_archetype.rotations, *projectile_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(game_archetype.scales, *projectile_range, (vec3s){.15f, .5f, .15f});
    archetypeInitialize3f(game_archetype.velocities, *projectile_range, (vec3s){0.f, 1.f, 0.f});
    archetypeInitialize1f(game_archetype.speeds, *projectile_range, 8.f);
    archetypeInitializeMesh(graphics_archetype.vaos, graphics_archetype.index_counts, *projectile_range, Streak);
    archetypeInitialize1u(graphics_archetype.shaders, *projectile_range, shader_program_projectile);
    archetypeInitialize1u(graphics_archetype.textures, *projectile_range, texture_gradient);
    archetypeInitialize3f(graphics_archetype.positions, *projectile_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(graphics_archetype.rotations, *projectile_range, (vec3s){-1.f * pi * .5f, pi, 0.f});
    archetypeInitialize3f(graphics_archetype.scales, *projectile_range, (vec3s){.25f, 1.f, .25f});

    i32 enemy_arena_index = rangeArenaAppend(range_arena_game, "enemy", 10);
    rangeArenaPrint(range_arena_game, "enemy");
    Range *enemy_range = &((Range *)range_arena_game->ranges.data)[enemy_arena_index];
    archetypeInitializeMesh(game_archetype.vaos, game_archetype.index_counts, *enemy_range, Box);
    archetypeInitialize1u(game_archetype.shaders, *enemy_range, shader_program_box);
    archetypeInitialize1u(game_archetype.textures, *enemy_range, texture);
    archetypeInitialize3f(game_archetype.positions, *enemy_range, (vec3s){0.f, -10.f, 0.f});
    archetypeInitialize3f(game_archetype.rotations, *enemy_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(game_archetype.scales, *enemy_range, (vec3s){.35f, .35f, .35f});
    archetypeInitialize1i(game_archetype.lives, *enemy_range, 3);
    archetypeInitialize1f(game_archetype.speeds, *enemy_range, 4.f);
    archetypeInitialize3f(game_archetype.velocities, *enemy_range, (vec3s){0.f, -1.f, 0.f});
    archetypeInitializeMesh(graphics_archetype.vaos, graphics_archetype.index_counts, *enemy_range, Ship);
    archetypeInitialize1u(graphics_archetype.shaders, *enemy_range, shader_program_ship);
    archetypeInitialize1u(graphics_archetype.textures, *enemy_range, texture2);
    archetypeInitialize3f(graphics_archetype.positions, *enemy_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(graphics_archetype.rotations, *enemy_range, (vec3s){pi * .5f, 0.f, 0.f});
    archetypeInitialize3f(graphics_archetype.scales, *enemy_range, (vec3s){.15f, .15f, .15f});
    for(int i = 0; i < 10; ++i) {
        ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start + i].fire_rate = 10;
        ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start + i].fire_burst_count = 3;
        ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start + i].fire_burst_rate = 3;
        ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start + i].fire_counter = 10;
        game_archetype.positions.data[enemy_range->start + i].x = i * 0.4f -2.f;
    }
    // archetypeInitializePositionsAsLine((vec3 *)game_archetype.positions.data, 2.f, 1.f, ((Range *)range_arena_game->ranges.data)[enemy_arena_index]);

    i32 enemy_projectiles_arena_index = rangeArenaAppend(range_arena_game, "enemy_projectiles", 3 * 10);
    rangeArenaPrint(range_arena_game, "enemy_projectiles");
    Range *projectile_enemy_range = &((Range *)range_arena_game->ranges.data)[enemy_projectiles_arena_index];
    archetypeInitializeMesh(game_archetype.vaos, game_archetype.index_counts, *projectile_enemy_range, Box);
    archetypeInitialize1u(game_archetype.shaders, *projectile_enemy_range, shader_program_box);
    archetypeInitialize1u(game_archetype.textures, *projectile_enemy_range, texture);
    archetypeInitialize3f(game_archetype.positions, *projectile_enemy_range, (vec3s){-100.f, -100.f, -100.f});
    archetypeInitialize3f(game_archetype.rotations, *projectile_enemy_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(game_archetype.scales, *projectile_enemy_range, (vec3s){.15f, .5f, .15f});
    archetypeInitialize3f(game_archetype.velocities, *projectile_enemy_range, (vec3s){0.f, -1.f, 0.f});
    archetypeInitialize1f(game_archetype.speeds, *projectile_enemy_range, 8.f);
    archetypeInitializeMesh(graphics_archetype.vaos, graphics_archetype.index_counts, *projectile_enemy_range, Streak);
    archetypeInitialize1u(graphics_archetype.shaders, *projectile_enemy_range, shader_program_projectile_red);
    archetypeInitialize1u(graphics_archetype.textures, *projectile_enemy_range, texture_gradient);
    archetypeInitialize3f(graphics_archetype.positions, *projectile_enemy_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(graphics_archetype.rotations, *projectile_enemy_range, (vec3s){-1.f * pi * .5f, 0.f, 0.f});
    archetypeInitialize3f(graphics_archetype.scales, *projectile_enemy_range, (vec3s){.25f, 1.f, .25f});

    // setup(static_archetype)
    i32 background = rangeArenaAppend(range_arena_static, "background", 1);
    rangeArenaPrint(range_arena_static, "background");
    Range *background_range = &((Range *)range_arena_static->ranges.data)[background];
    // Graphics
    archetypeInitializeMesh(static_archetype.vaos, static_archetype.index_counts, *background_range, PlaneXY);
    archetypeInitialize1u(static_archetype.shaders, *background_range, shader_program_starfield);
    archetypeInitialize1u(static_archetype.textures, *background_range, texture);
    archetypeInitialize3f(static_archetype.positions, *background_range, (vec3s){0.f, 0.f, -89.f});
    archetypeInitialize3f(static_archetype.rotations, *background_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(static_archetype.scales, *background_range, (vec3s){10.f, 10.f, 10.f});

    i32 ui_l_plane = rangeArenaAppend(range_arena_static, "ui_l_plane", 1);
    rangeArenaPrint(range_arena_static, "ui_l_plane");
    Range *ui_l_plane_range = &((Range *)range_arena_static->ranges.data)[ui_l_plane];
    // Graphics
    archetypeInitializeMesh(static_archetype.vaos, static_archetype.index_counts, *ui_l_plane_range, PlaneXY);
    archetypeInitialize1u(static_archetype.shaders, *ui_l_plane_range, shader_program_ui_bg);
    archetypeInitialize1u(static_archetype.textures, *ui_l_plane_range, texture);
    archetypeInitialize3f(static_archetype.positions, *ui_l_plane_range, (vec3s){1.8f, 0.f, 0.f});
    archetypeInitialize3f(static_archetype.rotations, *ui_l_plane_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(static_archetype.scales, *ui_l_plane_range, (vec3s){2.f, 2.f, 1.f});

    i32 ui_r_plane = rangeArenaAppend(range_arena_static, "ui_r_plane", 1);
    rangeArenaPrint(range_arena_static, "ui_r_plane");
    Range *ui_r_plane_range = &((Range *)range_arena_static->ranges.data)[ui_r_plane];
    // Graphics
    archetypeInitializeMesh(static_archetype.vaos, static_archetype.index_counts, *ui_r_plane_range, PlaneXY);
    archetypeInitialize1u(static_archetype.shaders, *ui_r_plane_range, shader_program_ui_bg);
    archetypeInitialize1u(static_archetype.textures, *ui_r_plane_range, texture);
    archetypeInitialize3f(static_archetype.positions, *ui_r_plane_range, (vec3s){-1.8f, 0.f, 0.f});
    archetypeInitialize3f(static_archetype.rotations, *ui_r_plane_range, (vec3s){0.f, 0.f, 0.f});
    archetypeInitialize3f(static_archetype.scales, *ui_r_plane_range, (vec3s){2.f, 2.f, 1.f});

    return;
}

void input() {
    vec3s *velocities = game_archetype.velocities.data;
    i32 hero_arena_index = rangeArenaGetIndex(range_arena_game, "hero");
    i32 hero_projectiles_arena_index = rangeArenaGetIndex(range_arena_game, "hero_projectiles");
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
                        velocities[hero_start].x = -base;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        velocities[hero_start].x = base;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        velocities[hero_start].y = base;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        velocities[hero_start].y = -base;
                    }
                /*
                } else if(event.key.keysym.sym == SDLK_SPACE) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        // rangeArenaPrint(range_arena_game, "background");
                        archetypeSpawnProjectileAtEntity(
                            (vec3 *)game_archetype.positions.data,
                            &((FireCore *)game_archetype.fire_cores.data)[hero_arena_index].fire_index,
                            ((Range *)range_arena_game->ranges.data)[hero_projectiles_arena_index], hero_start);
                    }
                    */
                }
                break;
            }
            case SDL_KEYUP: {
                if(event.key.keysym.sym == SDLK_ESCAPE) {
                    should_quit = true;
                } else if(event.key.keysym.sym == SDLK_a) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        if(velocities[hero_start].x < 0.f) velocities[hero_start].x = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        if(velocities[hero_start].x > 0.f) velocities[hero_start].x = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        if(velocities[hero_start].y > 0.f) velocities[hero_start].y = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = hero_start; i < hero_end; ++i) {
                        if(velocities[hero_start].y < 0.f) velocities[hero_start].y = 0.f;
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    }
    const u8 *key_states = SDL_GetKeyboardState(NULL);
    if(key_states[SDL_SCANCODE_SPACE]) {
        archetypeSpawnProjectileAtEntityAuto(game_archetype.positions, /* &((FireCore *)game_archetype.fire_cores.data)[hero_arena_index].fire_index, */
            &((FireCore *)game_archetype.fire_cores.data)[hero_arena_index],
            ((Range *)range_arena_game->ranges.data)[hero_projectiles_arena_index], hero_start);
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

    // Interlude
    // update(game_archetype|graphics_archetype)
    const i32 hero_arena_index = rangeArenaGetIndex(range_arena_game, "hero");
    const Range hero_range = ((Range *)range_arena_game->ranges.data)[hero_arena_index];

    const i32 hero_projectiles_arena_index = rangeArenaGetIndex(range_arena_game, "hero_projectiles");

    const i32 enemy_arena_index = rangeArenaGetIndex(range_arena_game, "enemy");
    const Range enemy_range = ((Range *)range_arena_game->ranges.data)[enemy_arena_index];

    const i32 enemy_projectiles_arena_index = rangeArenaGetIndex(range_arena_game, "enemy_projectiles");
    const Range total_range = (Range){ .start = 0, .end = range_arena_game->border };

    for(int i = 0; i < ((Range *)range_arena_game->ranges.data)[enemy_arena_index].length; ++i) {
        // TODO(Ibrahim): fix function internals
        // TODO(Ibrahim): remove magic number ``3" -> number_of_projectile_ammo_per_enemy
        archetypeSpawnProjectileAtEntityAI(
            game_archetype.positions,
            (FireCore *)game_archetype.fire_cores.data, 
            ((Range *)range_arena_game->ranges.data)[enemy_projectiles_arena_index],
            ((Range *)range_arena_game->ranges.data)[enemy_arena_index].start + i, i * 3);
    }

    // keep hero in box
    vec3s *hero_position = &game_archetype.positions.data[hero_range.start];
    if ((*hero_position).x > 3.f){
        (*hero_position).x = 3.f;
    } else if ((*hero_position).x < -3.f) {
        (*hero_position).x = -3.f;
    } else if ((*hero_position).y > 4.f) {
        (*hero_position).y = 4.f;
    } else if ((*hero_position).y < -4.f) {
        (*hero_position).y = -4.f;
    }

    // Collision Detection
    u8 hit_hero_enemy = archetypeProcessCollisions(&game_archetype, range_arena_game, hero_arena_index, enemy_arena_index);
    u8 hit_hero_eproj = archetypeProcessCollisions(&game_archetype, range_arena_game, hero_arena_index, enemy_projectiles_arena_index);
    if (hit_hero_enemy || hit_hero_eproj) {
        // reset hero position
        (*hero_position).x = 0.f;
        (*hero_position).y = -3.f;
        // printf("hero lives = %d\n", game_archetype.lives.data[hero_range.start]);
        game_archetype.lives.data[hero_range.start] -= 1;
    }
    archetypeProcessCollisions(&game_archetype, range_arena_game, hero_projectiles_arena_index, enemy_arena_index);

    counter += 1;
    if(counter == 50) {
        // printf("OK\n");
        // printf("iter = %d\n", iter);
        // float random_x = (float)((rand() % (spawn_range - -spawn_range + 1)) + -spawn_range);
        // int sign = (rand() % (1 - -1 + 1)) + -1;
        int sign = (rand() % 2) == 1 ? 1 : -1;
        // printf("sign = %d\n", sign);
        float random_x = ((float)rand()/(float)(RAND_MAX)) * 3 * sign;
        // printf("random_x = %f\n", random_x);
        game_archetype.positions.data[enemy_range.start + iter].x = random_x;
        game_archetype.positions.data[enemy_range.start + iter].y = 8.f;
        game_archetype.positions.data[enemy_range.start + iter].z = 0.f;
        counter = 0;
        iter += 1;
        if(iter % 10 == 0) { iter = 0; }
    }

    // Epilogue
    // integrate movement
    archetypeIntegrateVelocity(game_archetype.positions, game_archetype.velocities, game_archetype.speeds, delta_time, total_range);
    archetypeCopyVector(game_archetype.positions, graphics_archetype.positions, total_range);

    // finalize transformation matrices
    archetypeUpdateTransforms(game_archetype.positions, game_archetype.rotations, game_archetype.scales, game_archetype.models, total_range);
    archetypeUpdateTransforms(graphics_archetype.positions, graphics_archetype.rotations, graphics_archetype.scales, graphics_archetype.models, total_range);

    // update(static_archetype)
    Range static_range = (Range){ .start = 0, .end = range_arena_static->border };
    archetypeUpdateTransforms(static_archetype.positions, static_archetype.rotations, static_archetype.scales, static_archetype.models, static_range);
}

void render() {
    // begin
    glClearColor(.05f, .05f, .05f, 1.f);
    // ropsCheckError_();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // bind
    // render(graphics_archetype|game_archetype)
    archetypeRender(graphics_archetype.vaos, graphics_archetype.shaders, graphics_archetype.textures, graphics_archetype.index_counts, graphics_archetype.models, view, proj_persp, (Range){0, range_arena_game->border});
    /*
    archetypeRenderWires(
        (u32 *)game_archetype.vaos.data,
        (u32 *)game_archetype.shaders.data,
        (u32 *)game_archetype.textures.data,
        (u32 *)game_archetype.index_counts.data,
        (mat4 *)game_archetype.models.data,
        view, proj_persp, (Range){0, range_arena_game->border});
    */
    // render(static_archetype)
    archetypeSetUniform1f(
        (u32 *)static_archetype.shaders.data,
        (Range){0, range_arena_static->border},
        "time", SDL_GetTicks() / 1000.f);
    archetypeRender(static_archetype.vaos, static_archetype.shaders, static_archetype.textures, static_archetype.index_counts, static_archetype.models, view, proj_ortho, (Range){0, range_arena_static->border});
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

    display_mode.h = window_height_init;
    display_mode.w = window_width_init;

    // Creates a SDL window
    //-------------------------------------------
    window = SDL_CreateWindow("glsandbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, display_mode.w, display_mode.h, SDL_WINDOW_OPENGL);

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
