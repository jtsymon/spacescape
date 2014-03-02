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

#include "misc.h"
#include "shader.h"
#include "render.h"

static void error_callback(int error, const char* description) {
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        render_to_screen();
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        save_to_png();
    }
}

int main() {
    scale = max(width, height);
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

    render_init();

    // Render to our framebuffer
    render_to_screen();

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

    render_cleanup();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}