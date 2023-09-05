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
    // #include "models/shipIndices.txt"
    #include "models/rubberIndices.txt"
};

f32 vertices[] = {
    // #include "models/cubeVertices.txt"
    // #include "models/shipVertices.txt"
    #include "models/rubberVertices.txt"
};

// u32 vao;
// u32 vbo;
// u32 ebo;
// i32 vertex_count;

u32 shader_program;
u32 texture;
Mesh mesh;
mat4 model;
mat4 view;
mat4 proj;
// vec3 pos;
// vec3 vel;
f32 angle;
f32 speed;

Mesh mesh2;

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
    // uint32_t texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    i32 width, height, n_channels;
    // u8 *data = stbi_load("resource/cgfx.png", &width, &height, &n_channels, 0);
    u8 *data = stbi_load("resource/toylowres.jpg", &width, &height, &n_channels, 0);
    // u8 *data = stbi_load("resource/awesomeface.png", &width, &height, &n_channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        printf("failed to load texture.");
    }
    STBI_FREE(data);

    //  GL BUFFERS
    //-------------------------------------------
    mesh = meshCreate(vertices, sizeofarray(vertices, f32), indices, sizeofarray(indices, i32));
    mesh2 = meshCreate(vertices, sizeofarray(vertices, f32), indices, sizeofarray(indices, i32));
    // vertex_count = sizeofarray(indices, i32);
    // printf("Vertex Count = %d", vertex_count);
    // printf("GLSizei= %lld", sizeof(GLsizei));

    /*
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)0);
    glEnableVertexAttribArray(0);  

    // tex
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void*)(sizeof(f32) * 3));
    glEnableVertexAttribArray(1);  

    glBindVertexArray(0);
    */

    // XFORMS
    //--------------------------------------------
    // camera look at dir
    camera_position[2] = 10.0f;
    glm_vec3_sub(camera_direction, camera_position, camera_direction);
    glm_vec3_normalize(camera_direction);

    // camera right
    glm_vec3_cross((vec3){0.0f, 1.0f, 0.0f}, camera_direction, camera_right);
    glm_vec3_normalize(camera_right);

    // camera up
    glm_vec3_cross(camera_direction, camera_right, camera_up);
    camera_forward[2] = -1.0f;

    // setup MVP
    glm_mat4_identity(model);
    glm_mat4_identity(view);
    glm_mat4_identity(proj);
    glm_perspective(glm_rad(45.0f), 800.0f / 600.0f, 0.1f, 100.0f, proj);

    speed = 4.f;
    mesh.pos[0] = -5.f;
}

void input() {
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
                }
                if(event.key.keysym.sym == SDLK_a) {
                    mesh.vel[0] = -1.f;
                }
                if(event.key.keysym.sym == SDLK_d) {
                    mesh.vel[0] = 1.f;
                }
                if(event.key.keysym.sym == SDLK_w) {
                    mesh.vel[1] = 1.f;
                }
                if(event.key.keysym.sym == SDLK_s) {
                    mesh.vel[1] = -1.f;
                }
                break;
            }
            case SDL_KEYUP: {
                if(event.key.keysym.sym == SDLK_ESCAPE) {
                    should_quit = true;
                }
                if(event.key.keysym.sym == SDLK_a) {
                    mesh.vel[0] = 0.f;
                }
                if(event.key.keysym.sym == SDLK_d) {
                    mesh.vel[0] = 0.f;
                }
                if(event.key.keysym.sym == SDLK_w) {
                    mesh.vel[1] = 0.f;
                }
                if(event.key.keysym.sym == SDLK_s) {
                    mesh.vel[1] = 0.f;
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
    // printf("prev: %d, delay: %d\n", framePrevTime, frameDelay);   

    mesh.pos[0] += speed * mesh.vel[0] * deltaTime;
    mesh.pos[1] += speed * mesh.vel[1] * deltaTime;
    angle += .01f;

    vec3 camera_new_location;
    glm_vec3_add(camera_position, camera_forward, camera_new_location);
    glm_lookat(camera_position, camera_new_location, camera_up, view);

    // transforms S T R
    glm_mat4_identity(model);
    glm_scale_uni(model, 1.f);
    glm_translate(model, mesh.pos);
    glm_rotate(model, angle, (vec3){1.f, 1.f, 0.f});
    // glm_rotate(model, 3.141592f * .5f, (vec3){0.f, 1.f, 0.f});
    mesh2.pos[0] += .01f;
}

void render() {
    // begin
    glClearColor(.05f, .05f, .05f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // bind
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // uniforms
    uint32_t view_location = glGetUniformLocation(shader_program, "view");
    glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
    uint32_t proj_location = glGetUniformLocation(shader_program, "proj");
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj[0]);
    uint32_t model_location = glGetUniformLocation(shader_program, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, model[0]);
    // draw
    meshRender(&mesh, texture, shader_program);

    // uniforms
    glm_mat4_identity(model);
    view_location = glGetUniformLocation(shader_program, "view");
    glUniformMatrix4fv(view_location, 1, GL_FALSE, view[0]);
    proj_location = glGetUniformLocation(shader_program, "proj");
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj[0]);
    model_location = glGetUniformLocation(shader_program, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, model[0]);
    // draw
    meshRender(&mesh2, texture, shader_program);

    /*
    glBindTexture(GL_TEXTURE_2D, texture);
    glUseProgram(shader_program);
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    */

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
    window = SDL_CreateWindow("cgfx", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 640, SDL_WINDOW_OPENGL);

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

    /* Shuts down all SDL subsystems */
    SDL_Quit(); 
    return 0;
}
