#include "SDL2/SDL_events.h"
#include "SDL2/SDL_video.h"
#ifdef _MSC_VER
    #define SDL_MAIN_HANDLED
    // #define _CRT_SECURE_NO_WARNINGS
#endif

#include <glad/glad.h>
#include <SDL2/SDL.h>

#include <cglm/vec3.h>
#include <cglm/affine.h>
#include <cglm/cam.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>


#include "texops.h"
#include "core.h"
#include "shops.h"
#include "fileops.h"
#include "renderops.h"
#include "meshops.h"

#include "rangeops.h"
// #include "mesh.h"
#include "gameArchetype.h"

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
GameArchetype game_archetype;
RangeArena *range_arena_game;

BoxArchetype box_archetype;
RangeArena *range_arena_box;

GameArchetype archetype_plane;

// GAME
///////////////////////////////////
#define MaxEntityCount 1024
typedef struct {
    u32 enemy;
    u32 hero;
    u32 projectile;
    u32 colliders;
} Id;
Id id;

u32 shader_program;
u32 shader_program_starfield;
u32 shader_program_projectile;
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
    MeshRawDataArray[Ship] = MeshDataInitialize(ship_vertices, sizeofarray(ship_vertices, f32), ship_indices, sizeofarray(ship_indices, u32));
    MeshRawDataArray[Streak] = MeshDataInitialize(streak_vertices, sizeofarray(streak_vertices, f32), streak_indices, sizeofarray(streak_indices, u32));
    MeshRawDataArray[Plane] = MeshDataInitialize(box_vertices, sizeofarray(box_vertices, f32), box_indices, sizeofarray(box_indices, u32));

    MeshVAOGen(&MeshVAOArray[Ship], &MeshRawDataArray[Ship]);
    MeshVAOGen(&MeshVAOArray[Streak], &MeshRawDataArray[Streak]);
    MeshVAOGen(&MeshVAOArray[Plane], &MeshRawDataArray[Plane]);

    //  SHADER
    /////////////////////////////////////////////
    shader_create(&shader_program, "resource/simple.vert", "resource/simple.frag");
    shader_create(&shader_program_starfield, "resource/starfield.vert", "resource/starfield.frag");
    shader_create(&shader_program_projectile, "resource/projectile.vert", "resource/projectile.frag");

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

    // glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(proj);
    glm_perspective(glm_rad((f32)FOV), (f32)window_width / (f32)window_height, 0.1f, 100.0f, proj);

    //  SETUP_GAME_ARCHETYPES
    //-------------------------------------------
    gameArchetypeAllocate(&game_archetype, MaxEntityCount);
    range_arena_game = rangeArenaAllocate(MaxEntityCount);

    id.enemy = rangeArenaInitalize(range_arena_game, 6);
    // rangeArenaIndexPrint(range_arena_game, id.enemy);
    archetypeInitalizeMeshesShadersTextures((u32 *)game_archetype.vao.data, MeshVAOArray[Ship], 
                                            (u32 *)game_archetype.index_count.data, MeshRawDataArray[Ship].indices_count, 
                                            (u32 *)game_archetype.shader_program.data, shader_program,
                                            (u32 *)game_archetype.texture.data, texture2,
                                            ((Range *)range_arena_game->ranges.data)[id.enemy]);
    archetypeInitializeTransforms((vec3 *)game_archetype.position.data,
                                  (vec3 *)game_archetype.rotation.data,
                                  (vec3 *)game_archetype.scale.data,
                                  (vec3){0.f, 0.f, 0.f}, 
                                  (vec3){pi * 0.5, 0.f, 0.f}, 
                                  (vec3){.15f, .15f, .15f},
                                  ((Range *)range_arena_game->ranges.data)[id.enemy]);
    archetypeInitializePositionsAsLine((vec3 *)game_archetype.position.data, 2.f, 1.f, ((Range *)range_arena_game->ranges.data)[id.enemy]);
    archetypeInitializeSpeeds((f32 *)game_archetype.speed.data, 3.0f, ((Range *)range_arena_game->ranges.data)[id.enemy]);
    // archetypeInitializeVelocities(&game_archetype, (vec3){0.f, -1.f, 0.f}, ((Range *)range_arena_game->ranges.data)[id.enemy]);

    id.hero = rangeArenaAppend(range_arena_game, 1);
    // rangeArenaIndexPrint(range_arena_game, id.hero);
    archetypeInitalizeMeshesShadersTextures((u32 *)game_archetype.vao.data, MeshVAOArray[Ship], 
                                            (u32 *)game_archetype.index_count.data, MeshRawDataArray[Ship].indices_count, 
                                            (u32 *)game_archetype.shader_program.data, shader_program,
                                            (u32 *)game_archetype.texture.data, texture,
                                            ((Range *)range_arena_game->ranges.data)[id.hero]);
    archetypeInitializeTransforms((vec3 *)game_archetype.position.data,
                                  (vec3 *)game_archetype.rotation.data,
                                  (vec3 *)game_archetype.scale.data,
                                  (vec3){0.f, -3.f, 0.f}, 
                                  (vec3){pi * 0.5f, pi, 0.f}, 
                                  (vec3){.15f, .15f, .15f},
                                  ((Range *)range_arena_game->ranges.data)[id.hero]);
    archetypeInitializeSpeeds((f32 *)game_archetype.speed.data, 6.0f, ((Range *)range_arena_game->ranges.data)[id.hero]);

    id.projectile = rangeArenaAppend(range_arena_game, 100);
    // rangeArenaIndexPrint(range_arena_game, id.projectile);
    archetypeInitalizeMeshesShadersTextures((u32 *)game_archetype.vao.data, MeshVAOArray[Streak], 
                                            (u32 *)game_archetype.index_count.data, MeshRawDataArray[Streak].indices_count, 
                                            (u32 *)game_archetype.shader_program.data, shader_program_projectile,
                                            (u32 *)game_archetype.texture.data, texture,
                                            ((Range *)range_arena_game->ranges.data)[id.projectile]);
    archetypeInitializeTransforms((vec3 *)game_archetype.position.data,
                                  (vec3 *)game_archetype.rotation.data,
                                  (vec3 *)game_archetype.scale.data,
                                  (vec3){0.f, 0.f, 0.f}, 
                                  (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
                                  (vec3){.15f, .15f, .15f},
                                  ((Range *)range_arena_game->ranges.data)[id.projectile]);
    archetypeSetPositions((vec3 *)game_archetype.position.data, (vec3){-100.f, -100.f, 0.f}, ((Range *)range_arena_game->ranges.data)[id.projectile]);
    archetypeInitializeVelocities(&game_archetype, (vec3){0.f, 1.f, 0.f}, ((Range *)range_arena_game->ranges.data)[id.projectile]);
    archetypeInitializeSpeeds((f32 *)game_archetype.speed.data, 10.0f, ((Range *)range_arena_game->ranges.data)[id.projectile]);

    //  SETUP_BOX_ARCHETYPES
    //-------------------------------------------
    boxArchetypeAllocate(&box_archetype, MaxEntityCount);
    range_arena_box = rangeArenaAllocate(MaxEntityCount);

    id.colliders = rangeArenaInitalize(range_arena_box, range_arena_game->border);
                                                          // ((Range *)range_arena_game->ranges.data)[id.hero].length +
                                                          // ((Range *)range_arena_game->ranges.data)[id.enemy].length +
                                                          // ((Range *)range_arena_game->ranges.data)[id.projectile].length);
    // rangeArenaIndexPrint(range_arena_box, id.colliders);
    archetypeInitalizeMeshesShadersTextures((u32 *)box_archetype.vao.data, MeshVAOArray[Plane],
                                            (u32 *)box_archetype.index_count.data, MeshRawDataArray[Plane].indices_count,
                                            (u32 *)box_archetype.shader_program.data, shader_program_projectile,
                                            (u32 *)box_archetype.texture.data, texture,
                                            ((Range *)range_arena_box->ranges.data)[id.colliders]);
    archetypeInitializeTransforms((vec3 *)box_archetype.position.data,
                                  (vec3 *)box_archetype.rotation.data,
                                  (vec3 *)box_archetype.scale.data,
                                  (vec3){0.f, 0.f, 0.f}, 
                                  (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
                                  (vec3){.35f, 1.f, .35f},
                                  ((Range *)range_arena_box->ranges.data)[id.colliders]);

    /*
    // gameArchetypeAllocate(&archetype_plane, 1);
    gameArchetypeInitalizeMeshes((u32 *)archetype_plane.vao.data, MeshVAOArray[Plane], 
                                 (u32 *)archetype_plane.index_count.data, MeshRawDataArray[Plane].indices_count, 
                                 archetype_plane.index_count.length);
    gameArchetypeInitializeTransforms((vec3 *)archetype_plane.position.data,
                                      (vec3 *)archetype_plane.rotation.data,
                                      (vec3 *)archetype_plane.scale.data,
                                      (vec3){0.f, 0.f, -1.f}, 
                                      (vec3){pi * 0.5f, 0.f, 0.f}, 
                                      (vec3){20.f, 20.f, 20.f},
                                      archetype_plane.index_count.length);
    */

    // gameArchetypeInitializeCollisionBoxes(&game_archetype, 3.f, 3.f, 0.f, 0.f);
    // gameArchetypeInitializeCollisionBoxes(&game_archetype, 3.f, 3.f, 0.f, 0.f);
    // gameArchetypeInitializeCollisionBoxes(&game_archetype, 1.f, 1.f, 0.f, 0.f);
}

void input() {
    vec3 *velocity = ((vec3 *)game_archetype.velocity.data);
    const i32 s = 6;
    const i32 n = 7; // game_archetype.index_count.length; 
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
                    for(i32 i = s; i < n; ++i) {
                        velocity[s][0] = -base;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = s; i < n; ++i) {
                        velocity[s][0] = base;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = s; i < n; ++i) {
                        velocity[s][1] = base;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = s; i < n; ++i) {
                        velocity[s][1] = -base;
                    }
                } else if(event.key.keysym.sym == SDLK_SPACE) {
                    for(i32 i = s; i < n; ++i) {
                        archetypeSpawnProjectileAtEntity((vec3 *)game_archetype.position.data, s, 
                                                         (vec3 *)game_archetype.position.data, 
                                                         ((Range *)range_arena_game->ranges.data)[id.projectile].start, 
                                                         ((Range *)range_arena_game->ranges.data)[id.projectile].length);
                    }
                }
                break;
            }
            case SDL_KEYUP: {
                if(event.key.keysym.sym == SDLK_ESCAPE) {
                    should_quit = true;
                } else if(event.key.keysym.sym == SDLK_a) {
                    for(i32 i = s; i < n; ++i) {
                        if(velocity[s][0] < 0.f) velocity[s][0] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = s; i < n; ++i) {
                        if(velocity[s][0] > 0.f) velocity[s][0] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = s; i < n; ++i) {
                        if(velocity[s][1] > 0.f) velocity[s][1] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = s; i < n; ++i) {
                        if(velocity[s][1] < 0.f) velocity[s][1] = 0.f;
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
    float deltaTime = (SDL_GetTicks() - framePrevTime) / 1000.f;
    if (deltaTime > frameTime) {
        deltaTime = frameTimef32;
    }
    // printf("ticks: %d, ", SDL_GetTicks());
    framePrevTime = SDL_GetTicks();
    // printf("FPS: %f.\n", 999.f / (1000.f * deltaTime));

    // camera
    vec3 camera_new_location;
    glm_vec3_add(camera_position, camera_forward, camera_new_location);
    glm_lookat(camera_position, camera_new_location, camera_up, view);

    // update attributes
    // gameArchetypeUpdateVelocities(&game_archetype, SDL_GetTicks() / 1000.f, ((Range *)range_arena_game->ranges.data)[RangeIdEnemy]);
    // integrate movement
    archetypeIntegrateVelocity((vec3 *)game_archetype.position.data,
                               (vec3 *)game_archetype.velocity.data,
                               (f32 *)game_archetype.speed.data,
                               deltaTime,
                               (Range){0, range_arena_game->border});
    archetypeCopyVector((vec3 *)game_archetype.position.data,
                        (vec3 *)box_archetype.position.data,
                        ((Range *)range_arena_box->ranges.data)[id.colliders]);
    // finalize transformation matrices
    archetypeUpdateTransforms(
        (vec3 *)game_archetype.position.data,
        (vec3 *)game_archetype.rotation.data,
        (vec3 *)game_archetype.scale.data,
        (mat4 *)game_archetype.model.data,
        (Range){0, range_arena_game->border});
    archetypeUpdateTransforms(
        (vec3 *)box_archetype.position.data,
        (vec3 *)box_archetype.rotation.data,
        (vec3 *)box_archetype.scale.data,
        (mat4 *)box_archetype.model.data,
        ((Range *)range_arena_box->ranges.data)[id.colliders]);

    {
        // check collisions
        i32 coll_id = gameArchetypeCheckCollisions2(
                (vec3 *)box_archetype.position.data,
                (vec3 *)box_archetype.scale.data,
                ((Range *)range_arena_game->ranges.data)[id.hero],
                (vec3 *)box_archetype.position.data,
                (vec3 *)box_archetype.scale.data,
                ((Range *)range_arena_game->ranges.data)[id.enemy]);
        // printf("collision id = %d\n", coll_id);
        if (coll_id != -1) {
            ((vec3 *)game_archetype.position.data)[coll_id][0] = -1000.f;
            ((vec3 *)game_archetype.position.data)[coll_id][1] = -1000.f;
            // printf("collision id = %d\n", coll_id);
            coll_id = -1;
        }
    }
//     {
//         // check collisions
//         i32 coll_id = gameArchetypeCheckCollisions(&game_archetype, &game_archetype);
//         // printf("collision id = %d\n", coll_id);
//         if (coll_id != -1) {
//             // printf("hit collision id = %d\n", coll_id);
//             ((vec3 *)game_archetype.position.data)[coll_id][0] = -1000.f;
//             ((vec3 *)game_archetype.position.data)[coll_id][1] = -1000.f;
//             // printf("collision id = %d\n", coll_id);
//             coll_id = -1;
//         }
//     }
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
            getComponent(game_archetype, u32, vao),
            getComponent(game_archetype, u32, shader_program),
            getComponent(game_archetype, u32, texture),
            getComponent(game_archetype, u32, index_count),
            getComponent(game_archetype, mat4, model),
            view, proj, (Range){0, range_arena_game->border});
    archetypeRenderWires(
            getComponent(box_archetype, u32, vao),
            getComponent(box_archetype, u32, shader_program),
            getComponent(box_archetype, u32, texture),
            getComponent(box_archetype, u32, index_count),
            getComponent(box_archetype, mat4, model),
            view, proj, (Range){0, range_arena_box->border});
                    // view, proj, range_arena_box->ranges[id.colliders]);
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
