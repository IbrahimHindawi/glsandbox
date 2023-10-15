#include "SDL2/SDL_events.h"
#include "SDL2/SDL_video.h"
#ifdef _MSC_VER
    #define SDL_MAIN_HANDLED
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
// #include "mesh.h"
#include "gameArchetype.h"

// system
bool should_quit = false;
SDL_Window *window = NULL;
char fops_buffer[fops_buffer_size];

// time
#define FPS 60
const int frameTime = 1000 / FPS;
const float frameTimef32 = 1000.f / FPS;
int framePrevTime;
int frameDelay;

// ship
i32 ship_indices[] = { 
    // #include "models/cubeIndices.txt"
    #include "models/shipIndices.txt"
    // #include "models/rubberIndices.txt"
    /*
    0, 1, 3,
    1, 2, 3
    */
};
f32 ship_vertices[] = {
    // #include "models/cubeVertices.txt"
    #include "models/shipVertices.txt"
    // #include "models/rubberVertices.txt"
    /*
     1.0f, 0.0f,  1.0f, 1.0f, 1.0f,
     1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f,  1.0f, 0.0f, 1.0f,
    */
};

// streak
i32 streak_indices[] = { 
    #include "models/streakIndices.txt"
};
f32 streak_vertices[] = {
    #include "models/streakVertices.txt"
};

// box
i32 box_indices[] = { 
    0, 1, 3,
    1, 2, 3
};
f32 box_vertices[] = {
     1.0f, 0.0f,  1.0f, 1.0f, 1.0f,
     1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 0.0f,  1.0f, 0.0f, 1.0f,
};

gameArchetype archetypeEnemy;
gameArchetype archetypeHero;
gameArchetype archetypeProjectile;
gameArchetype archetypeColliders;
gameArchetype archetypePlane;

u32 shader_program;
u32 shader_program_starfield;
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
    //  SHADER
    //-------------------------------------------
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
    // TEXTURE
    //-------------------------------------------
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
        u8 *data = stbi_load("resource/Ship_BaseColor_low.png", &width, &height, &n_channels, 0);
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
        u8 *data = stbi_load("resource/green.jpg", &width, &height, &n_channels, 0);
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
    //--------------------------------------------
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
    // setup MVP
    // glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(proj);
    glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, proj);

    //  ARCHETYPES
    //-------------------------------------------
    gameArchetypeInitializeMemory(&archetypeEnemy, 6);
    gameArchetypeInitializeMemoryRenderer(&archetypeEnemy, 
            ship_vertices, sizeofarray(ship_vertices, f32), 
            ship_indices, sizeofarray(ship_indices, i32));
    gameArchetypeInitializeMemoryRendererDebug(&archetypeEnemy, 
            box_vertices, sizeofarray(box_vertices, f32), 
            box_indices, sizeofarray(box_vertices, f32));

    gameArchetypeInitializeMemory(&archetypeHero, 1);
    gameArchetypeInitializeMemoryRenderer(&archetypeHero, 
            ship_vertices, sizeofarray(ship_vertices, f32), 
            ship_indices, sizeofarray(ship_indices, i32));
    gameArchetypeInitializeMemoryRendererDebug(&archetypeHero, 
            box_vertices, sizeofarray(box_vertices, f32), 
            box_indices, sizeofarray(box_vertices, f32));

    gameArchetypeInitializeMemory(&archetypeProjectile, 100);
    gameArchetypeInitializeMemoryRenderer(&archetypeProjectile, 
            streak_vertices, sizeofarray(streak_vertices, f32), 
            streak_indices, sizeofarray(streak_indices, i32));
    gameArchetypeInitializeMemoryRendererDebug(&archetypeProjectile,
            box_vertices, sizeofarray(box_vertices, f32), 
            box_indices, sizeofarray(box_vertices, f32));

    gameArchetypeInitializeMemory(&archetypePlane, 1);
    gameArchetypeInitializeMemoryRenderer(&archetypePlane, 
            box_vertices, sizeofarray(box_vertices, f32), 
            box_indices, sizeofarray(box_indices, i32));

    // gameArchetypeSetupPositionsAsGrid(&archetypeEnemy);
    gameArchetypeSetupPositionsAsLine(&archetypeEnemy, 15.f);
    ((vec3 *)archetypeHero.pos.data)[0][1] = -20.f;
    gameArchetypeSetupPositions(&archetypeProjectile, (vec3){-100.f, -100.f, 0.f});
    gameArchetypeSetupVelocities(&archetypeProjectile, (vec3){0.f, 10.f, 0.f});

    gameArchetypeSetupCollisionBoxes(&archetypeEnemy, 3.f, 3.f);
    gameArchetypeSetupCollisionBoxes(&archetypeHero, 3.f, 3.f);
    gameArchetypeSetupCollisionBoxes(&archetypeProjectile, 3.f, 3.f);
}

void input() {
    vec3 *vel = ((vec3 *)archetypeHero.vel.data);
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
                        vel[0][0] = -base;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = 0; i < n; ++i) {
                        vel[0][0] = base;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = 0; i < n; ++i) {
                        vel[0][1] = base;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = 0; i < n; ++i) {
                        vel[0][1] = -base;
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
                        if(vel[0][0] < 0.f)
                            vel[0][0] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_d) {
                    for(i32 i = 0; i < n; ++i) {
                        if(vel[0][0] > 0.f)
                            vel[0][0] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_w) {
                    for(i32 i = 0; i < n; ++i) {
                        if(vel[0][1] > 0.f)
                            vel[0][1] = 0.f;
                    }
                } else if(event.key.keysym.sym == SDLK_s) {
                    for(i32 i = 0; i < n; ++i) {
                        if(vel[0][1] < 0.f)
                            vel[0][1] = 0.f;
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
    gameArchetypeUpdateVelocities(&archetypeEnemy, SDL_GetTicks() / 1000.f);
    gameArchetypeUpdateColliders(&archetypeHero);
    gameArchetypeUpdateColliders(&archetypeEnemy);
    gameArchetypeUpdateColliders(&archetypeProjectile);

    // integrate movement
    gameArchetypeUpdatePlayer(&archetypeHero, deltaTime, 16.f);
    gameArchetypeUpdate(&archetypeEnemy, deltaTime, 4.f);
    gameArchetypeUpdate(&archetypeProjectile, deltaTime, 5.f);
    // gameArchetypeUpdate(&archetypePlane, deltaTime, 0.f);
    gameArchetypeUpdateBG(&archetypePlane, 20.f);

    {
        // check collisions
        i32 coll_id = gameArchetypeCheckCollisions(&archetypeHero, &archetypeEnemy);
        // printf("collision id = %d\n", coll_id);
        if (coll_id != -1) {
            ((vec3 *)archetypeEnemy.pos.data)[coll_id][0] = -1000.f;
            ((vec3 *)archetypeEnemy.pos.data)[coll_id][1] = -1000.f;
            // printf("collision id = %d\n", coll_id);
            coll_id = -1;
        }
    }
    {
        // check collisions
        i32 coll_id = gameArchetypeCheckCollisions(&archetypeProjectile, &archetypeEnemy);
        // printf("collision id = %d\n", coll_id);
        if (coll_id != -1) {
            // printf("hit collision id = %d\n", coll_id);
            ((vec3 *)archetypeEnemy.pos.data)[coll_id][0] = -1000.f;
            ((vec3 *)archetypeEnemy.pos.data)[coll_id][1] = -1000.f;
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
    gameArchetypeRenderBG(&archetypePlane, shader_program_starfield, view, proj);

    gameArchetypeRender(&archetypeEnemy, shader_program, view, proj, texture);
    // gameArchetypeRenderBoxes(&archetypeEnemy, shader_program, view, proj, texture2);

    gameArchetypeRender(&archetypeHero, shader_program, view, proj, texture);
    // gameArchetypeRenderBoxes(&archetypeHero, shader_program, view, proj, texture2);

    gameArchetypeRender(&archetypeProjectile, shader_program, view, proj, texture2);
    // gameArchetypeRenderBoxes(&archetypeProjectile, shader_program, view, proj, texture2);

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

    // Setup OpenGL
    //-------------------------------------------
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);


    // Creates a SDL window
    //-------------------------------------------
    window = SDL_CreateWindow("glsandbox", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 640, SDL_WINDOW_OPENGL);

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
    gameArchetypeDeinitializeMemory(&archetypeHero);
    gameArchetypeDeinitializeMemory(&archetypeEnemy);
    gameArchetypeDeinitializeMemory(&archetypeProjectile);

    /* Shuts down all SDL subsystems */
    SDL_Quit(); 
    return 0;
}
