#include "shader.h"

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

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader) {
    if (vertex_shader == 0 || fragment_shader == 0) {
        if(vertex_shader == 0) {
            fputs("Received NULL vertex shader", stderr);
        }
        if(fragment_shader == 0) {
            fputs("Received NULL fragment shader", stderr);
        }
        exit(1);
    }

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

GLuint create_program_src(const char * vertex_file_path, const char * fragment_file_path) {
    
    create_program(create_shader(vertex_file_path, GL_VERTEX_SHADER), create_shader(fragment_file_path, GL_FRAGMENT_SHADER));
    
}