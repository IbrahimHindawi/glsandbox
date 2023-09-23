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
#include "mesh.h"

// system
bool should_quit = false;
SDL_Window *window = NULL;
char fops_buffer[1024];

// time
#define FPS 60
const int frameTime = 1000 / FPS;
const float frameTimef32 = 1000.f / FPS;
int framePrevTime;
int frameDelay;

// mesh
i32 indices[] = { 
    // #include "models/cubeIndices.txt"
    #include "models/shipIndices.txt"
    // #include "models/rubberIndices.txt"
};
f32 vertices[] = {
    // #include "models/cubeVertices.txt"
    #include "models/shipVertices.txt"
    // #include "models/rubberVertices.txt"
};

// streak
i32 streak_indices[] = { 
    // #include "models/cubeIndices.txt"
    #include "models/streakIndices.txt"
    // #include "models/rubberIndices.txt"
};
f32 streak_vertices[] = {
    // #include "models/cubeVertices.txt"
    #include "models/streakVertices.txt"
    // #include "models/rubberVertices.txt"
};

gameArchetype archetypeEnemy;
gameArchetype archetypeHero;
gameArchetype archetypeProjectile;

u32 shader_program;
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
        u8 *data = stbi_load("resource/Ship_BaseColor.png", &width, &height, &n_channels, 0);
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
        u8 *data = stbi_load("resource/green.png", &width, &height, &n_channels, 0);
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
    gameArchetypeInitializeMemoryRenderer(&archetypeEnemy, vertices, sizeofarray(vertices, f32), indices, sizeofarray(indices, i32));

    gameArchetypeInitializeMemory(&archetypeHero, 1);
    gameArchetypeInitializeMemoryRenderer(&archetypeHero, vertices, sizeofarray(vertices, f32), indices, sizeofarray(indices, i32));

    gameArchetypeInitializeMemory(&archetypeProjectile, 100);
    gameArchetypeInitializeMemoryRenderer(&archetypeProjectile, streak_vertices, sizeofarray(streak_vertices, f32), streak_indices, sizeofarray(streak_indices, i32));

    // gameArchetypeSetupPositionsAsGrid(&archetypeEnemy);
    gameArchetypeSetupPositionsAsLine(&archetypeEnemy, 15.f);
    // gameArchetypeSetupPositionsAsLine(&archetypeProjectile, 5.f);
    ((vec3 *)archetypeHero.pos.data)[0][1] = -20.f;
    // ((vec3 *)archetypeProjectile.pos.data)[0][0] = 0.f;
    ((vec3 *)archetypeProjectile.pos.data)[0][1] = 20.f;

    gameArchetypeSetupCollisionBoxes(&archetypeEnemy, 3.f, 3.f);
    gameArchetypeSetupCollisionBoxes(&archetypeHero, 3.f, 3.f);
    gameArchetypeSetupCollisionBoxes(&archetypeProjectile, 1.f, 1.f);
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

    // update enemy
    gameArchetypeUpdateVelocities(&archetypeEnemy, SDL_GetTicks() / 1000.f);
    gameArchetypeUpdateColliders(&archetypeEnemy);
    gameArchetypeUpdateColliders(&archetypeHero);

    // integrate movement
    gameArchetypeUpdate(&archetypeEnemy, deltaTime, 4.f);
    gameArchetypeUpdate(&archetypeProjectile, deltaTime, 4.f);
    gameArchetypeUpdatePlayer(&archetypeHero, deltaTime, 16.f);

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

void render() {
    // begin
    glClearColor(.05f, .05f, .05f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // bind
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    gameArchetypeRender(&archetypeEnemy, shader_program, view, proj, texture);
    gameArchetypeRender(&archetypeHero, shader_program, view, proj, texture);
    gameArchetypeRender(&archetypeProjectile, shader_program, view, proj, texture2);
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


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

    puts("OpenGL loaded");
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version: %s\n", glGetString(GL_VERSION));


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
