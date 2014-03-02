/* 
 * File:   main.c
 * Author: jts
 *
 * Created on 28 February 2014, 6:50 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef int bool;
#define true 1
#define false 0

unsigned long get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

int max(int a, int b) {
    return a > b ? a : b;
}
int min(int a, int b) {
    return a < b ? a : b;
}

/**
 * Display compilation errors from the OpenGL shader compiler
 */
void shader_error(GLuint object) {
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        fprintf(stderr, "printlog: Not a shader or a program\n");
        return;
    }

    char* log = (char*) malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    fprintf(stderr, "%s", log);
    free(log);
}

/**
 * Reads a file and returns the data from the file
 * The receiver is responsible for free()ing the returned char* array
 */
char* read_file(const char* filename) {
    struct stat st;
    if (stat(filename, &st) != 0) {
        fprintf(stderr, "Cannot determine size of %s: %s\n", filename, strerror(errno));
        return NULL;
    }
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Cannot open %s for binary reading: %s\n", filename, strerror(errno));
        return NULL;
    }

    char* data = malloc(st.st_size + 1);
    fread(data, 1, st.st_size, file);
    fclose(file);
    data[st.st_size] = '\0';
    
    return data;
}

/**
 * Compile the shader from file 'filename', with error handling
 */
GLuint create_shader(const char* filename, GLenum type) {
    GLuint shader = glCreateShader(type);

    char const * source = read_file(filename);
    if (source == NULL) {
        fprintf(stderr, "Error opening %s: ", filename);
        perror("");
        return 0;
    }

    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);


    free((char*) source);

    GLint compile_ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        fprintf(stderr, "%s: ", filename);
        shader_error(shader);
        glDeleteShader(shader);
        return 0;
    }
    printf("Shader '%s' compiled successfully\n", filename);
    return shader;
}

GLuint create_program(const char * vertex_file_path, const char * fragment_file_path) {

    // Create the shaders
    GLuint vertex_shader = create_shader(vertex_file_path, GL_VERTEX_SHADER);
    GLuint fragment_shader = create_shader(fragment_file_path, GL_FRAGMENT_SHADER);

    if (vertex_shader == 0 || fragment_shader == 0) exit(1);

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, vertex_shader);
    glAttachShader(ProgramID, fragment_shader);
    glLinkProgram(ProgramID);

    // Check the program
    GLint compile_ok = GL_FALSE;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        fprintf(stderr, "Failed to link: ");
        shader_error(ProgramID);
        exit(1);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return ProgramID;
}

GLuint textures[2], framebuffers[2];
GLuint quad_vertexbuffer;
bool gen_starfield = true, gen_stars = true, gen_nebulae = true, gen_sun = false;
int width = 1920, height = 1080;
int scale;
int fb = 0;
GLuint program;

void set_framebuffer(int* fb, GLuint program) {
    // set framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[*fb]);
    glViewport(0, 0, width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    // swap frame buffers
    if(++(*fb) > 1) *fb = 0;
    // map the texture of the inactive framebuffer
    GLuint texID = glGetUniformLocation(program, "renderedTexture");
    // Bind our texture in Texture Unit -
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[*fb]);
    // Set our "renderedTexture" sampler to user Texture Unit 0
    glUniform1i(texID, 0);
}

GLuint rendertoscreen_program;
void draw() {
    // Render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    
    GLuint texID = glGetUniformLocation(rendertoscreen_program, "renderedTexture");
    glUseProgram(rendertoscreen_program);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[fb]);
    // Set our "renderedTexture" sampler to user Texture Unit 0
    glUniform1i(texID, 0);
}

void generate() {
    int i;
    printf("Started at %llu\n", get_time_us());
    
    if(gen_starfield) {
        program = create_program("pass_through.vert", "starfield.frag");
        glUseProgram(program);
        
        set_framebuffer(&fb, program);
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glVertexAttribPointer(
                0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3, // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0, // stride
                (void*) 0 // array buffer offset
                );

        // Draw the triangles !
        glUniform1f(glGetUniformLocation(program, "seed"), (float)(rand() & 0xAFF) / 10);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

        glDisableVertexAttribArray(0);
        glDeleteProgram(program);
    }
    if(gen_stars) {
        program = create_program("pass_through.vert", "star.frag");
        glUseProgram(program);
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glVertexAttribPointer(
                0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3, // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0, // stride
                (void*) 0 // array buffer offset
                );
        
        int n = 1;
        while (rand() < RAND_MAX / 2) n++;
        // TODO: Could optimise this by only drawing regions which contain stars
        for (i = 0; i < n; i++) {
            set_framebuffer(&fb, program);
            int x = rand() % width, y = rand() % height;
            float radius = 1.f + scale * 0.005f + (rand() % scale) * 0.01f;
            printf("Position: %d, %d; Radius: %f\n", x, y, radius);
            glUniform2f(glGetUniformLocation(program, "coord"), x, y);
            glUniform1f(glGetUniformLocation(program, "radius"), radius);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        glDisableVertexAttribArray(0);
        glDeleteProgram(program);
    }
    if(gen_nebulae) {
        program = create_program("pass_through.vert", "nebula.frag");
        glUseProgram(program);
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glVertexAttribPointer(
                0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3, // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0, // stride
                (void*) 0 // array buffer offset
                );
        
        int n = 1;
        while (rand() < RAND_MAX / 2) n++;
        
        for(i = 0; i < n; i++) {
            set_framebuffer(&fb, program);
            float r = (float)rand() / RAND_MAX;
            float g = (float)rand() / RAND_MAX;
            float b = (float)rand() / RAND_MAX;
            float seed = (float)(rand() & 0xAFF) / 10;
            float intensity = 0.5 + (((float)rand() / RAND_MAX) * 0.5);
            float falloff = 3 + ((float)rand() / RAND_MAX) * 3;
            glUniform1f(glGetUniformLocation(program, "seed"), seed);
            glUniform3f(glGetUniformLocation(program, "colour"), r, g, b);
            glUniform1f(glGetUniformLocation(program, "intensity"), intensity);
            glUniform1f(glGetUniformLocation(program, "falloff"), falloff);
            printf("Seed: %f; Intensity: %f; Falloff: %f; #%02x%02x%02x\n", seed, intensity, falloff,
                    (int)(r * 255) & 0xFF, (int)(g * 255) & 0xFF, (int)(b * 255) & 0xFF);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        glDisableVertexAttribArray(0);
        glDeleteProgram(program);
    }
    if(gen_sun) {
        program = create_program("pass_through.vert", "sun.frag");
        glUseProgram(program);
        
        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glVertexAttribPointer(
                0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3, // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0, // stride
                (void*) 0 // array buffer offset
                );
        
        int n = 1;
        while (rand() < RAND_MAX / 10) n++;
        
        for(i = 0; i < n; i++) {
            set_framebuffer(&fb, program);
            int x = rand() % width, y = rand() % height;
            float r,g,b;
            if(rand() > RAND_MAX / 2) {
                r = 1;
                g = (float)rand() / RAND_MAX;
                b = ((float)rand() / RAND_MAX) * 0.25f;
            } else {
                r = ((float)rand() / RAND_MAX) * 0.25f;
                g = (float)rand() / RAND_MAX;
                b = 1;
            }
            float radius = 1.f + scale * (rand() / (RAND_MAX * 20.f) + 0.02f);
            printf("Position: %d, %d; Radius: %f; #%02x%02x%02x\n", x, y, radius,
                    (int)(r * 255) & 0xFF, (int)(g * 255) & 0xFF, (int)(b * 255) & 0xFF);
            glUniform2f(glGetUniformLocation(program, "coord"), x, y);
            glUniform1f(glGetUniformLocation(program, "radius"), radius);
            glUniform1f(glGetUniformLocation(program, "radius_squared"), radius * radius);
            glUniform3f(glGetUniformLocation(program, "colour"), r, g, b);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        glDisableVertexAttribArray(0);
        glDeleteProgram(program);
    }
    
    printf("Finished at %llu\n", get_time_us());
    // swap framebuffer again so we get the final texture
    if(++fb > 1) fb = 0;
    
    // get back into a state we can draw from
    draw();
}

static void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        generate();
    }
}

int main() {
    scale = max(width, height);
    int i;
    srand(get_time_us());

    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    // Initialize the library
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(width, height, "SpaceScape", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    // Init GLEW
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    fprintf(stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION));
    if (!GLEW_ARB_vertex_buffer_object) {
        fputs("VBO not supported\n", stderr);
        exit(1);
    }

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // ================================== //
    // Generate Framebuffers and Textures //
    // ================================== //

    // The framebuffer, which regroups 0 or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(2, framebuffers);
    // The textures to render to
    glGenTextures(2, textures);
    
    for(i = 0; i <= 1; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);

        // "Bind" the newly created texture : all future texture functions will modify this texture
        glBindTexture(GL_TEXTURE_2D, textures[i]);

        // Give an empty image to OpenGL ( the last "0" means "empty" )
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

        // Poor filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[i], 0);

        //    // Set the list of draw buffers.
        //    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        //    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

        // Always check that our framebuffer is ok
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            return false;
    }

    static const GLfloat fullscreen_quad[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    };

    glGenBuffers(1, &quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof (fullscreen_quad), fullscreen_quad, GL_STATIC_DRAW);

    rendertoscreen_program = create_program("pass_through.vert", "screen.frag");
    
    // Render to our framebuffer
    generate();

    while (!glfwWindowShouldClose(window)) {

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glVertexAttribPointer(
                0, // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3, // size
                GL_FLOAT, // type
                GL_FALSE, // normalized?
                0, // stride
                (void*) 0 // array buffer offset
                );

        // Draw the triangles !
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

        glDisableVertexAttribArray(0);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(rendertoscreen_program);

    glDeleteFramebuffers(2, framebuffers);
    glDeleteTextures(2, textures);
    glDeleteBuffers(1, &quad_vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);


    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}