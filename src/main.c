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
#include <getopt.h>

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
        save_to_png(NULL);
        // render_to_png(NULL);
    }
}

static void print_help(int exitcode) {
    static char* options[] = {
        "-h, --help",                   "display this help and exit",
        "-o, --output <name>",          "specify the file to output to\n                                    (default = <time>.png)",
        "-f, --tofile",                 "output to a file and exit (implied by -o)",
        "-s, --size   <width*height>",  "size of the image to generate\n                                    (default = 1920x1080)"
    };
    fprintf(stderr, "Spacescape options:\n");
    int i;
    for(i = 0; i < 4; i++) {
        fprintf(stderr, "  %-30s%s\n", options[i * 2], options[i * 2 + 1]);
    }
    end(exitcode);
}

int main(int argc, char **argv) {
    
    int i;
    bool tofile = false;
    char* output = NULL;
    
    int c, option_index = 0;
    static struct option long_options[] = {
        {"help", 0, 0, 0},
        {"output", 1, 0, 0},
        {"tofile", 0, 0, 0},
        {"size", 1, 0, 0},
        {NULL, 0, NULL, 0}
    };
    bool fail = false;
    bool help = false;
    while ((c = getopt_long(argc, argv, "ho:fs:",
            long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                switch(option_index) {
                case 0: // help
                    goto help;
                case 1: // output
                    goto output;
                case 2: // file
                    goto tofile;
                case 3: // size
                    goto size;
                default:
                    goto unknown;
                }
                break;
            help:
            case 'h':   // help
                help = true;
                break;
            output:
            case 'o':   // output
                output = optarg;
                // drop down to tofile, since we're outputting to a file
            tofile:
            case 'f':
                tofile = true;
                break;
            size:
            case 's':
                i = 0;
                while(optarg[i] != '*' && optarg[i] != '\0') i++;
                if(optarg[i] == '\0') {
                    goto size_fail;
                }
                optarg[i] = '\0';
                char* err = NULL;
                width = strtol(optarg, &err, 10);
                if (err == optarg || (err != NULL && *err != '\0')) {
                    goto size_fail;
                }
                height = strtol(optarg + i + 1, &err, 10);
                if (err == optarg || (err != NULL && *err != '\0')) {
                    goto size_fail;
                }
                printf("width: %d, height: %d\n", width, height);
                break;
            size_fail:
                fprintf(stderr, "Invalid size string '%s'\n", optarg);
                print_help(1);
                break;
            unknown:
            case '?':
                fail = true;
                break;
            default:
                fprintf(stderr, "?? getopt returned character code 0%o ??\n", c);
        }
    }
    if(fail) {
        print_help(1);
    }
    if(optind < argc) {
        fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[optind]);
        print_help(1);
    }
    if(help) {
        print_help(0);
    }
    
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
    if(tofile) {
        glfwWindowHint(GLFW_VISIBLE, false);
        window = glfwCreateWindow(1, 1, "SpaceScape", NULL, NULL);
    } else {
        window = glfwCreateWindow(width, height, "SpaceScape", NULL, NULL);
    }
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

    if(tofile) {
        render_to_png(output);
    } else {
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
    }

    render_cleanup();

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}