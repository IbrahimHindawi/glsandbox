#include "SDL2/SDL_events.h"
#include "SDL2/SDL_video.h"
#ifdef _MSC_VER
    #define SDL_MAIN_HANDLED
    // #define _CRT_SECURE_NO_WARNINGS
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

// SYSTEM
///////////////////////////////////
#define window_width 800
#define window_height 640
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
GameArchetype game_archetype;
GraphicsArchetype graphics_archetype;
RangeArena *range_arena_game;

// GAME
///////////////////////////////////
typedef struct {
    u32 hero;
    u32 enemy;
    u32 projectile_hero;
    u32 projectile_enemy;
} Id;
Id id;
i32 *current_projectile_pool_index;

u32 shader_program;
u32 shader_program_starfield;
u32 shader_program_projectile;
u32 shader_program_box;
u32 texture;
u32 texture2;
mat4 view;
mat4 proj;
f32 angle;

// camera
vec3 camera_position = {0};
vec3 camera_forward = {0};
vec3 camera_up = {0};
vec3 camera_right = {0};
vec3 camera_target = {0};
vec3 camera_direction = {0};

void setup() {
    // MESH
    /////////////////////
    current_projectile_pool_index = malloc(sizeof(i32));
    // printf("current_projectile_pool_index address: %p\n", current_projectile_pool_index);
    *current_projectile_pool_index = 0;
    MeshRawDataArray[Ship] = MeshDataInitialize(ship_vertices, sizeofarray(ship_vertices, f32), ship_indices, sizeofarray(ship_indices, u32));
    MeshRawDataArray[Streak] = MeshDataInitialize(streak_vertices, sizeofarray(streak_vertices, f32), streak_indices, sizeofarray(streak_indices, u32));
    MeshRawDataArray[Plane] = MeshDataInitialize(box_vertices, sizeofarray(box_vertices, f32), box_indices, sizeofarray(box_indices, u32));

    MeshVAOGen(&MeshVAOArray[Ship], &MeshRawDataArray[Ship]);
    MeshVAOGen(&MeshVAOArray[Streak], &MeshRawDataArray[Streak]);
    MeshVAOGen(&MeshVAOArray[Plane], &MeshRawDataArray[Plane]);

    //  SHADER
    /////////////////////////////////////////////
    shader_create(&shader_program, "resource/simple.vert", "resource/simple.frag");
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
    glm_mat4_identity(proj);
    glm_perspective(glm_rad((f32)FOV), (f32)window_width / (f32)window_height, 0.1f, 100.0f, proj);

    //  SETUP_GAME_ARCHETYPES
    //-------------------------------------------
    gameArchetypeAllocate(&game_archetype, MAX);
    boxArchetypeAllocate(&graphics_archetype, MAX);
    range_arena_game = rangeArenaAllocate(MAX);

    id.hero = rangeArenaInitalize(range_arena_game, 1);
    rangeArenaIndexPrint("hero", range_arena_game, id.hero);
    Range *hero_range = &((Range *)range_arena_game->ranges.data)[id.hero];
    // Game
    archetypeInitalizeMeshesShadersTextures(
            (u32 *)game_archetype.vaos.data, MeshVAOArray[Plane], 
            (u32 *)game_archetype.index_counts.data, MeshRawDataArray[Plane].indices_count, 
            (u32 *)game_archetype.shaders.data, shader_program_box,
            (u32 *)game_archetype.textures.data, texture,
            *hero_range);
    archetypeInitializeTransforms(
            (vec3 *)game_archetype.positions.data,
            (vec3 *)game_archetype.rotations.data,
            (vec3 *)game_archetype.scales.data,
            (vec3){0.f, -3.f, 0.f}, 
            (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
            (vec3){.35f, 1.f, .35f},
            *hero_range);
    archetypeInitializeSpeeds(
            (f32 *)game_archetype.speeds.data, 6.0f, 
            *hero_range);
    // Graphics
    archetypeInitalizeMeshesShadersTextures(
            (u32 *)graphics_archetype.vaos.data, MeshVAOArray[Ship],
            (u32 *)graphics_archetype.index_counts.data, MeshRawDataArray[Ship].indices_count,
            (u32 *)graphics_archetype.shaders.data, shader_program,
            (u32 *)graphics_archetype.textures.data, texture,
            *hero_range);
    archetypeInitializeTransforms(
            (vec3 *)graphics_archetype.positions.data,
            (vec3 *)graphics_archetype.rotations.data,
            (vec3 *)graphics_archetype.scales.data,
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){pi * 0.5f, pi, 0.f}, 
            (vec3){.15f, .15f, .15f},
            *hero_range);

    id.projectile_hero = rangeArenaAppend(range_arena_game, 8);
    rangeArenaIndexPrint("projectile_hero", range_arena_game, id.projectile_hero);
    Range *projectile_range = &((Range *)range_arena_game->ranges.data)[id.projectile_hero];
    // Game
    archetypeInitalizeMeshesShadersTextures(
            (u32 *)game_archetype.vaos.data, MeshVAOArray[Plane],
            (u32 *)game_archetype.index_counts.data, MeshRawDataArray[Plane].indices_count,
            (u32 *)game_archetype.shaders.data, shader_program_box,
            (u32 *)game_archetype.textures.data, texture,
            *projectile_range);
    archetypeInitializeTransforms(
            (vec3 *)game_archetype.positions.data,
            (vec3 *)game_archetype.rotations.data,
            (vec3 *)game_archetype.scales.data,
            (vec3){-100.f, -100.f, -100.f}, 
            (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
            (vec3){.05f, 1.f, .15f},
            *projectile_range);
    archetypeInitializeSpeeds(
            (f32 *)game_archetype.speeds.data, 
            8.0f, 
            *projectile_range);
    archetypeInitializeVelocities(
            game_archetype.velocities.data, 
            (vec3){0.f, 1.f, 0.f}, 
            *projectile_range);
    // Graphics
    archetypeInitalizeMeshesShadersTextures(
            (u32 *)graphics_archetype.vaos.data, MeshVAOArray[Streak],
            (u32 *)graphics_archetype.index_counts.data, MeshRawDataArray[Streak].indices_count,
            (u32 *)graphics_archetype.shaders.data, shader_program_projectile,
            (u32 *)graphics_archetype.textures.data, texture,
            *projectile_range);
    archetypeInitializeTransforms(
            (vec3 *)graphics_archetype.positions.data,
            (vec3 *)graphics_archetype.rotations.data,
            (vec3 *)graphics_archetype.scales.data,
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
            (vec3){.25f, 1.f, .25f},
            *projectile_range);

    id.enemy = rangeArenaAppend(range_arena_game, 1);
    rangeArenaIndexPrint("enemy", range_arena_game, id.enemy);
    Range *enemy_range = &((Range *)range_arena_game->ranges.data)[id.enemy];
    // Game
    archetypeInitalizeMeshesShadersTextures(
            (u32 *)game_archetype.vaos.data, MeshVAOArray[Plane], 
            (u32 *)game_archetype.index_counts.data, MeshRawDataArray[Plane].indices_count, 
            (u32 *)game_archetype.shaders.data, shader_program_box,
            (u32 *)game_archetype.textures.data, texture,
            *enemy_range);
    archetypeInitializeTransforms(
            (vec3 *)game_archetype.positions.data,
            (vec3 *)game_archetype.rotations.data,
            (vec3 *)game_archetype.scales.data,
            (vec3){0.f, 3.f, 0.f}, 
            (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
            (vec3){.35f, 1.f, .35f},
            *enemy_range);
    archetypeInitializeSpeeds(
            (f32 *)game_archetype.speeds.data, 
            3.0f, 
            *enemy_range);
    ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start].fire_rate = 20;
    ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start].fire_burst_count = 3;
    ((FireCore *)game_archetype.fire_cores.data)[enemy_range->start].fire_burst_rate = 3;
    /*
    archetypeInitializePositionsAsLine(
            (vec3 *)game_archetype.positions.data, 2.f, 1.f, 
            ((Range *)range_arena_game->ranges.data)[id.enemy]);
    archetypeInitializeVelocities(
            game_archetype.velocities.data, 
            (vec3){0.f, -1.f, 0.f}, 
            *enemy_range);
    */
    // Graphics
    archetypeInitalizeMeshesShadersTextures(
            (u32 *)graphics_archetype.vaos.data, MeshVAOArray[Ship],
            (u32 *)graphics_archetype.index_counts.data, MeshRawDataArray[Ship].indices_count,
            (u32 *)graphics_archetype.shaders.data, shader_program,
            (u32 *)graphics_archetype.textures.data, texture2,
            *enemy_range);
    archetypeInitializeTransforms(
            (vec3 *)graphics_archetype.positions.data,
            (vec3 *)graphics_archetype.rotations.data,
            (vec3 *)graphics_archetype.scales.data,
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){pi * 0.5, 0.f, 0.f}, 
            (vec3){.15f, .15f, .15f},
            *enemy_range);

    id.projectile_enemy = rangeArenaAppend(range_arena_game, 8);
    rangeArenaIndexPrint("projectile_enemy", range_arena_game, id.projectile_enemy);
    Range *projectile_enemy_range = &((Range *)range_arena_game->ranges.data)[id.projectile_enemy];
    // Game
    archetypeInitalizeMeshesShadersTextures(
            (u32 *)game_archetype.vaos.data, MeshVAOArray[Plane],
            (u32 *)game_archetype.index_counts.data, MeshRawDataArray[Plane].indices_count, 
            (u32 *)game_archetype.shaders.data, shader_program_box,
            (u32 *)game_archetype.textures.data, texture,
            *projectile_enemy_range);
    archetypeInitializeTransforms(
            (vec3 *)game_archetype.positions.data,
            (vec3 *)game_archetype.rotations.data,
            (vec3 *)game_archetype.scales.data,
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
            (vec3){.05f, 1.f, .15f},
            *projectile_enemy_range);
    archetypeInitializeSpeeds(
            (f32 *)game_archetype.speeds.data, 6.0f, 
            *projectile_enemy_range);
    archetypeInitializeVelocities(
            (vec3 *)game_archetype.velocities.data, 
            (vec3){0.0f, -1.f, 0.f},
            *projectile_enemy_range);
    // archetypeInitializeVelocities(&game_archetype, (vec3){0.f, -1.f, 0.f}, ((Range *)range_arena_game->ranges.data)[id.enemy]);
    // Graphics
    archetypeInitalizeMeshesShadersTextures(
            (u32 *)graphics_archetype.vaos.data, MeshVAOArray[Streak],
            (u32 *)graphics_archetype.index_counts.data, MeshRawDataArray[Streak].indices_count,
            (u32 *)graphics_archetype.shaders.data, shader_program_projectile,
            (u32 *)graphics_archetype.textures.data, texture,
            *projectile_enemy_range);
    archetypeInitializeTransforms(
            (vec3 *)graphics_archetype.positions.data,
            (vec3 *)graphics_archetype.rotations.data,
            (vec3 *)graphics_archetype.scales.data,
            (vec3){0.f, 0.f, 0.f}, 
            (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
            (vec3){.25f, 1.f, .25f},
            *projectile_enemy_range);
    return;
}

void input() {
    vec3 *velocities = ((vec3 *)game_archetype.velocities.data);
    const i32 hero_start = ((Range *)range_arena_game->ranges.data)[id.hero].start;
    const i32 hero_end = ((Range *)range_arena_game->ranges.data)[id.hero].end;
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
                        // printf("%s", "enemy: ");
                        // rangeArenaIndexPrint(range_arena_game, id.enemy);
                        // printf("%s", "hero: ");
                        // rangeArenaIndexPrint(range_arena_game, id.hero);
                        // printf("%s", "proj: ");
                        // rangeArenaIndexPrint(range_arena_game, id.projectile_hero);
                        // printf("%s", "coll: ");
                        // rangeArenaIndexPrint(range_arena_game, id.colliders);
                        archetypeSpawnProjectileAtEntity(
                            hero_start,
                            &((FireCore *)game_archetype.fire_cores.data)[id.hero].fire_index,
                            (vec3 *)game_archetype.positions.data,
                            ((Range *)range_arena_game->ranges.data)[id.projectile_hero]);
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

    // update attributes
    /*
    archetypeUpdateVelocities(
        (vec3 *)game_archetype.velocities.data,
        SDL_GetTicks() / 1000.f, 
        ((Range *)range_arena_game->ranges.data)[id.enemy]);
    */
    archetypeSpawnProjectileAtEntityAI(
        ((Range *)range_arena_game->ranges.data)[id.enemy].start,
        (FireCore *)game_archetype.fire_cores.data, 
        (vec3 *)game_archetype.positions.data,
        ((Range *)range_arena_game->ranges.data)[id.projectile_enemy]);

    Range total_range = (Range){ .start = 0, .end = range_arena_game->border };
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

    {
        // check collisions
        i32 coll_id = gameArchetypeCheckCollisions2(
                // A
                (vec3 *)game_archetype.positions.data,
                (vec3 *)game_archetype.scales.data,
                ((Range *)range_arena_game->ranges.data)[id.hero],
                // B
                (vec3 *)game_archetype.positions.data,
                (vec3 *)game_archetype.scales.data,
                ((Range *)range_arena_game->ranges.data)[id.enemy]);
        // printf("collision id = %d\n", coll_id);
        if (coll_id != -1) {
            ((vec3 *)game_archetype.positions.data)[coll_id][0] = -1000.f;
            ((vec3 *)game_archetype.positions.data)[coll_id][1] = -1000.f;
            // printf("collision id = %d\n", coll_id);
            coll_id = -1;
        }
    }
    {
        // check collisions
        i32 coll_id = gameArchetypeCheckCollisions2(
                // A
                (vec3 *)game_archetype.positions.data,
                (vec3 *)game_archetype.scales.data,
                ((Range *)range_arena_game->ranges.data)[id.projectile_hero],
                // B
                (vec3 *)game_archetype.positions.data,
                (vec3 *)game_archetype.scales.data,
                ((Range *)range_arena_game->ranges.data)[id.enemy]);
        // printf("collision id = %d\n", coll_id);
        if (coll_id != -1) {
            // printf("hit collision id = %d\n", coll_id);
            ((vec3 *)game_archetype.positions.data)[coll_id][0] = -1000.f;
            ((vec3 *)game_archetype.positions.data)[coll_id][1] = -1000.f;
            // printf("collision id = %d\n", coll_id);
            coll_id = -1;
        }
    }
}

void render() {
    // begin
    glClearColor(.05f, .05f, .05f, 1.f);
    // ropsCheckError_();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // bind
    // gameArchetypeRenderBG(&archetype_plane, shader_program_starfield, view, proj);
    archetypeRender(
            getComponent(graphics_archetype, u32, vaos),
            getComponent(graphics_archetype, u32, shaders),
            getComponent(graphics_archetype, u32, textures),
            getComponent(graphics_archetype, u32, index_counts),
            getComponent(graphics_archetype, mat4, models),
            view, proj, (Range){0, range_arena_game->border});
    archetypeRenderWires(
            getComponent(game_archetype, u32, vaos),
            getComponent(game_archetype, u32, shaders),
            getComponent(game_archetype, u32, textures),
            getComponent(game_archetype, u32, index_counts),
            getComponent(game_archetype, mat4, models),
            view, proj, (Range){0, range_arena_game->border});
    // gameArchetypeRenderBoxes(&game_archetype, shader_program_projectile, view, proj, texture2);
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
