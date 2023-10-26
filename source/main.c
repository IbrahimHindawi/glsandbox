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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "core.h"
#include "shader.h"
#include "fileops.h"
#include "renderops.h"
#include "meshops.h"
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
GameArchetype archetypeEnemy;
GameArchetype archetypeHero;
GameArchetype archetypeProjectile;
GameArchetype archetypeColliders;
GameArchetype archetypePlane;

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
    {
        fops_read("resource/simple.vert");
        u32 vertex_shader = shader_compile(fops_buffer, GL_VERTEX_SHADER);
        // printf("%s", fops_buffer);
        fops_read("resource/simple.frag");
        u32 fragment_shader = shader_compile(fops_buffer, GL_FRAGMENT_SHADER);
        // printf("%s", fops_buffer);
        // shader_program = shader_link(vertex_shader, fragment_shader);
        shader_program = shader_link(vertex_shader, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }
    {
        fops_read("resource/starfield.vert");
        u32 vertex_shader = shader_compile(fops_buffer, GL_VERTEX_SHADER);
        // printf("%s", fops_buffer);
        fops_read("resource/starfield.frag");
        u32 fragment_shader = shader_compile(fops_buffer, GL_FRAGMENT_SHADER);
        // printf("%s", fops_buffer);
        // shader_program = shader_link(vertex_shader, fragment_shader);
        shader_program_starfield = shader_link(vertex_shader, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }
    {
        fops_read("resource/projectile.vert");
        u32 vertex_shader = shader_compile(fops_buffer, GL_VERTEX_SHADER);
        // printf("%s", fops_buffer);
        fops_read("resource/projectile.frag");
        u32 fragment_shader = shader_compile(fops_buffer, GL_FRAGMENT_SHADER);
        // printf("%s", fops_buffer);
        // shader_program = shader_link(vertex_shader, fragment_shader);
        shader_program_projectile = shader_link(vertex_shader, fragment_shader);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    // TEXTURE
    /////////////////////////////////////////////
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        i32 width, height, n_channels;
        // u8 *data = stbi_load("resource/cgfx.png", &width, &height, &n_channels, 0);
        // u8 *data = stbi_load("resource/toylowres.jpg", &width, &height, &n_channels, 0);
        u8 *data = stbi_load("resource/Ship_BaseColor_purple_mid.jpg", &width, &height, &n_channels, 0);
        // u8 *data = stbi_load("resource/awesomeface.png", &width, &height, &n_channels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            printf("failed to load texture.");
        }
        STBI_FREE(data);
    }
    {
        glGenTextures(1, &texture2);
        glBindTexture(GL_TEXTURE_2D, texture2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        i32 width, height, n_channels;
        // u8 *data = stbi_load("resource/cgfx.png", &width, &height, &n_channels, 0);
        // u8 *data = stbi_load("resource/toylowres.jpg", &width, &height, &n_channels, 0);
        u8 *data = stbi_load("resource/Ship_BaseColor_orange_mid.jpg", &width, &height, &n_channels, 0);
        // u8 *data = stbi_load("resource/awesomeface.png", &width, &height, &n_channels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            printf("failed to load texture2.");
        }
        STBI_FREE(data);
    }

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

    //  SETUPARCHETYPES
    //-------------------------------------------
    gameArchetypeAllocate(&archetypeEnemy, 6);
    gameArchetypeInitalizeMeshes((u32 *)archetypeEnemy.vao.data, MeshVAOArray[Ship], 
                                 (u32 *)archetypeEnemy.index_count.data, MeshRawDataArray[Ship].indices_count, 
                                 archetypeEnemy.index_count.length);
    gameArchetypeInitializeTransforms((vec3 *)archetypeEnemy.position.data,
                                      (vec3 *)archetypeEnemy.rotation.data,
                                      (vec3 *)archetypeEnemy.scale.data,
                                      (vec3){0.f, 0.f, 0.f}, 
                                      (vec3){pi * 0.5, 0.f, 0.f}, 
                                      (vec3){.15f, .15f, .15f},
                                      archetypeEnemy.index_count.length);
    gameArchetypeInitializePositionsAsLine(&archetypeEnemy, 2.f, 10.f);
    gameArchetypeInitializeVelocities(&archetypeEnemy, (vec3){0.f, -1.f, 0.f});

    gameArchetypeAllocate(&archetypeHero, 1);
    gameArchetypeInitalizeMeshes((u32 *)archetypeHero.vao.data, MeshVAOArray[Ship], 
                                 (u32 *)archetypeHero.index_count.data, MeshRawDataArray[Ship].indices_count, 
                                 archetypeHero.index_count.length);
    gameArchetypeInitializeTransforms((vec3 *)archetypeHero.position.data,
                                      (vec3 *)archetypeHero.rotation.data,
                                      (vec3 *)archetypeHero.scale.data,
                                      (vec3){0.f, -3.f, 0.f}, 
                                      (vec3){pi * 0.5f, pi, 0.f}, 
                                      (vec3){.15f, .15f, .15f},
                                      archetypeHero.index_count.length);

    gameArchetypeAllocate(&archetypeProjectile, 100);
    gameArchetypeInitalizeMeshes((u32 *)archetypeProjectile.vao.data, MeshVAOArray[Streak], 
                                 (u32 *)archetypeProjectile.index_count.data, MeshRawDataArray[Streak].indices_count, 
                                 archetypeProjectile.index_count.length);
    gameArchetypeInitializeTransforms((vec3 *)archetypeProjectile.position.data,
                                      (vec3 *)archetypeProjectile.rotation.data,
                                      (vec3 *)archetypeProjectile.scale.data,
                                      (vec3){0.f, 0.f, 0.f}, 
                                      (vec3){-1.f * pi * 0.5f, pi, 0.f}, 
                                      (vec3){.15f, .15f, .15f},
                                      archetypeProjectile.index_count.length);
    gameArchetypeInitializePositions(&archetypeProjectile, (vec3){-100.f, -100.f, 0.f});
    gameArchetypeInitializeVelocities(&archetypeProjectile, (vec3){0.f, 1.f, 0.f});

    gameArchetypeAllocate(&archetypePlane, 1);
    gameArchetypeInitalizeMeshes((u32 *)archetypePlane.vao.data, MeshVAOArray[Plane], 
                                 (u32 *)archetypePlane.index_count.data, MeshRawDataArray[Plane].indices_count, 
                                 archetypePlane.index_count.length);
    gameArchetypeInitializeTransforms((vec3 *)archetypePlane.position.data,
                                      (vec3 *)archetypePlane.rotation.data,
                                      (vec3 *)archetypePlane.scale.data,
                                      (vec3){0.f, 0.f, -1.f}, 
                                      (vec3){pi * 0.5f, 0.f, 0.f}, 
                                      (vec3){20.f, 20.f, 20.f},
                                      archetypePlane.index_count.length);

    // gameArchetypeInitializeCollisionBoxes(&archetypeEnemy, 3.f, 3.f, 0.f, 0.f);
    // gameArchetypeInitializeCollisionBoxes(&archetypeHero, 3.f, 3.f, 0.f, 0.f);
    // gameArchetypeInitializeCollisionBoxes(&archetypeProjectile, 1.f, 1.f, 0.f, 0.f);
}

void input() {
    vec3 *velocity = ((vec3 *)archetypeHero.velocity.data);
    const i64 n = archetypeHero.index_count.length; 
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
                    for(i32 i = 0; i < n; ++i) {
                        velocity[0][0] = -base;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = 0; i < n; ++i) {
                        velocity[0][0] = base;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = 0; i < n; ++i) {
                        velocity[0][1] = base;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = 0; i < n; ++i) {
                        velocity[0][1] = -base;
                    }
                } else if(event.key.keysym.sym == SDLK_SPACE) {
                    for(i32 i = 0; i < n; ++i) {
                        gameSpawnProjectileAtEntity(&archetypeHero, &archetypeProjectile, 0);
                    }
                }
                break;
            }
            case SDL_KEYUP: {
                if(event.key.keysym.sym == SDLK_ESCAPE) {
                    should_quit = true;
                } else if(event.key.keysym.sym == SDLK_a) {
                    for(i32 i = 0; i < n; ++i) {
                        if(velocity[0][0] < 0.f)
                            velocity[0][0] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = 0; i < n; ++i) {
                        if(velocity[0][0] > 0.f)
                            velocity[0][0] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = 0; i < n; ++i) {
                        if(velocity[0][1] > 0.f)
                            velocity[0][1] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = 0; i < n; ++i) {
                        if(velocity[0][1] < 0.f)
                            velocity[0][1] = 0.f;
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

    // gameArchetypeSpawnProjectile(&archetypeEnemy, &archetypeProjectile);

    // update attributes
    // gameArchetypeUpdateVelocities(&archetypeEnemy, SDL_GetTicks() / 1000.f);
    // gameArchetypeUpdateColliders(&archetypeHero);
    // gameArchetypeUpdateColliders(&archetypeEnemy);
    // gameArchetypeUpdateColliders(&archetypeProjectile);

    // integrate movement
    gameArchetypeIntegrateVelocity(&archetypeHero, deltaTime, 4.f);
    gameArchetypeIntegrateVelocity(&archetypeEnemy, deltaTime, 3.f);
    gameArchetypeIntegrateVelocity(&archetypeProjectile, deltaTime, 5.f);

    // finalize transformation matrices
    gameArchetypeUpdateTransforms(&archetypeHero);
    gameArchetypeUpdateTransforms(&archetypeEnemy);
    gameArchetypeUpdateTransforms(&archetypeProjectile);
    gameArchetypeUpdateTransforms(&archetypePlane);

//     {
//         // check collisions
//         i32 coll_id = gameArchetypeCheckCollisions(&archetypeHero, &archetypeEnemy);
//         // printf("collision id = %d\n", coll_id);
//         if (coll_id != -1) {
//             ((vec3 *)archetypeEnemy.position.data)[coll_id][0] = -1000.f;
//             ((vec3 *)archetypeEnemy.position.data)[coll_id][1] = -1000.f;
//             // printf("collision id = %d\n", coll_id);
//             coll_id = -1;
//         }
//     }
//     {
//         // check collisions
//         i32 coll_id = gameArchetypeCheckCollisions(&archetypeProjectile, &archetypeEnemy);
//         // printf("collision id = %d\n", coll_id);
//         if (coll_id != -1) {
//             // printf("hit collision id = %d\n", coll_id);
//             ((vec3 *)archetypeEnemy.position.data)[coll_id][0] = -1000.f;
//             ((vec3 *)archetypeEnemy.position.data)[coll_id][1] = -1000.f;
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
    gameArchetypeRenderBG(&archetypePlane, shader_program_starfield, view, proj);

    gameArchetypeRender(&archetypeEnemy, shader_program, view, proj, texture2);
    // gameArchetypeRenderBoxes(&archetypeEnemy, shader_program_projectile, view, proj, texture2);

    gameArchetypeRender(&archetypeHero, shader_program, view, proj, texture);
    // gameArchetypeRenderBoxes(&archetypeHero, shader_program_projectile, view, proj, texture2);

    gameArchetypeRender(&archetypeProjectile, shader_program_projectile, view, proj, texture2);
    // gameArchetypeRenderBoxes(&archetypeProjectile, shader_program_projectile, view, proj, texture2);

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
    gameArchetypeDeallocate(&archetypeHero);
    gameArchetypeDeallocate(&archetypeEnemy);
    gameArchetypeDeallocate(&archetypeProjectile);

    /* Shuts down all SDL subsystems */
    SDL_Quit(); 
    return 0;
}
