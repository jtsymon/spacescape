
#include <GLES3/gl3.h>

#include "render.h"

GLuint textures[2], framebuffers[2];
GLuint quad_vertexbuffer;
GLuint VertexArrayID;
bool gen_starfield = true, gen_stars = true, gen_nebulae = true, gen_sun = true;
int width = 1920, height = 1080;
int scale;
int fb = 0;

GLuint vect_passthrough;
GLuint frag_starfield, frag_star, frag_sun, frag_nebula1, frag_nebula2;
GLuint frag_screen;
GLuint program_starfield, program_star, program_sun, program_nebula1, program_nebula2;
GLuint program_screen;

static void render_gen_framebuffers() {
    // The framebuffer, which regroups 0 or more textures, and 0 or 1 depth buffer.
    glGenFramebuffers(2, framebuffers);
    // The textures to render to
    glGenTextures(2, textures);
    
    int i;
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
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            fputs("Failed to generate framebuffers", stderr);
            exit(1);
        }
    }
}

void render_init() {
    render_gen_framebuffers();
    
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
    
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    
    vect_passthrough = create_shader("shaders/pass_through.vert", GL_VERTEX_SHADER);
    frag_starfield = create_shader("shaders/starfield.frag", GL_FRAGMENT_SHADER);
    frag_star = create_shader("shaders/star.frag", GL_FRAGMENT_SHADER);
    frag_nebula1 = create_shader("shaders/nebula1.frag", GL_FRAGMENT_SHADER);
    frag_nebula2 = create_shader("shaders/nebula2.frag", GL_FRAGMENT_SHADER);
    frag_sun = create_shader("shaders/sun.frag", GL_FRAGMENT_SHADER);
    frag_screen = create_shader("shaders/screen.frag", GL_FRAGMENT_SHADER);
    
    program_starfield = create_program(vect_passthrough, frag_starfield);
    program_star = create_program(vect_passthrough, frag_star);
    program_nebula1 = create_program(vect_passthrough, frag_nebula1);
    program_nebula2 = create_program(vect_passthrough, frag_nebula2);
    program_sun = create_program(vect_passthrough, frag_sun);
    program_screen = create_program(vect_passthrough, frag_screen);
    
    glDeleteShader(vect_passthrough);
    glDeleteShader(frag_starfield);
    glDeleteShader(frag_star);
    glDeleteShader(frag_nebula1);
    glDeleteShader(frag_nebula2);
    glDeleteShader(frag_sun);
}

void render_cleanup() {
    printf("Cleaning up...\n");
    
    glDeleteProgram(program_starfield);
    glDeleteProgram(program_star);
    glDeleteProgram(program_nebula1);
    glDeleteProgram(program_nebula2);
    glDeleteProgram(program_sun);
    glDeleteProgram(program_screen);
    
    glDeleteFramebuffers(2, framebuffers);
    glDeleteTextures(2, textures);
    
    glDeleteBuffers(1, &quad_vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
}

static void render_prepare_screen() {
    // Prepare to render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    
    GLuint texID = glGetUniformLocation(program_screen, "renderedTexture");
    glUseProgram(program_screen);

    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[fb]);
    // Set our "renderedTexture" sampler to user Texture Unit 0
    glUniform1i(texID, 0);
}

static void render_set_framebuffer(GLuint program) {
    // set framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[fb]);
    glViewport(0, 0, width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    // swap frame buffers
    if(++fb > 1) fb = 0;
    // map the texture of the inactive framebuffer
    GLuint texID = glGetUniformLocation(program, "renderedTexture");
    // Bind our texture in Texture Unit -
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[fb]);
    // Set our "renderedTexture" sampler to user Texture Unit 0
    glUniform1i(texID, 0);
}

static void render() {
    GLuint program;
    int i;
    printf("\nRender: started at %llu\n", get_time_us());
    
    if(gen_starfield) {
        program = program_starfield;
        glUseProgram(program);
        render_set_framebuffer(program);
        
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
    }
    if(gen_stars) {
        program = program_star;
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
            render_set_framebuffer(program);
            int x = rand() % width, y = rand() % height;
            float radius = 1.f + scale * 0.005f + (rand() % scale) * 0.01f;
            printf("Position: %d, %d; Radius: %f\n", x, y, radius);
            glUniform2f(glGetUniformLocation(program, "coord"), x, y);
            glUniform1f(glGetUniformLocation(program, "radius"), radius);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        glDisableVertexAttribArray(0);
    }
    if(gen_nebulae) {
        if(rand() < RAND_MAX / 2) {
            program = program_nebula1;
        } else {
            program = program_nebula2;
        }
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
            render_set_framebuffer(program);
            float r = (float)rand() / RAND_MAX;
            float g = (float)rand() / RAND_MAX;
            float b = (float)rand() / RAND_MAX;
            float seed1 = (float)(rand() & 0xAFF) / 10;
            float seed2 = 5.f + (float)(rand() % 5000) / 5000;
            float seed3 = 5.f + (float)(rand() % 5000) / 5000;
            float seed4 = 5.f + (float)(rand() % 5000) / 5000;
            float intensity = 0.5 + (((float)rand() / RAND_MAX) * 0.5);
            float falloff = 3 + ((float)rand() / RAND_MAX) * 3;
            glUniform4f(glGetUniformLocation(program, "seed"), seed1, seed2, seed3, seed4);
            glUniform3f(glGetUniformLocation(program, "colour"), r, g, b);
            glUniform1f(glGetUniformLocation(program, "intensity"), intensity);
            glUniform1f(glGetUniformLocation(program, "falloff"), falloff);
            printf("Seed: %f; Intensity: %f; Falloff: %f; #%02x%02x%02x\n", seed1, intensity, falloff,
                    (int)(r * 255) & 0xFF, (int)(g * 255) & 0xFF, (int)(b * 255) & 0xFF);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        glDisableVertexAttribArray(0);
    }
    if(gen_sun) {
        program = program_sun;
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
        
        int n = rand() > RAND_MAX / 2 ? 1 : 0;
        while (rand() < RAND_MAX / 20) n++;
        
        for(i = 0; i < n; i++) {
            render_set_framebuffer(program);
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
    }
    
    printf("Render: finished at %llu\n", get_time_us());
    // swap framebuffer again so we get the final texture
    if(++fb > 1) fb = 0;
}

void save_to_png() {
    int i;
    uint8_t pixels[width * height * 3];
    printf("PNG: started at %llu\n", get_time_us());
    // copy pixels from screen
    glBindTexture(GL_TEXTURE_2D, textures[fb]);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)pixels);
    // use libpng to write the pixels to a png image
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) goto png_fail;
    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_write_struct(&png, &info);
        goto png_fail;
    }
    // filename => current time in milliseconds
    char filename[256];
    snprintf(filename, 256, "%llu.png", get_time_us() * 1000);
    FILE *file = fopen(filename, "wb");
    if (!file) {
        png_destroy_write_struct(&png, &info);
        goto png_fail;
    }
    png_init_io(png, file);
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, 
            PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_colorp palette = png_malloc(png, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
    if (!palette) {
        fclose(file);
        png_destroy_write_struct(&png, &info);
        goto png_fail;
    }
    png_set_PLTE(png, info, palette, PNG_MAX_PALETTE_LENGTH);
    png_write_info(png, info);
    png_set_packing(png);
    png_bytepp rows = (png_bytepp)png_malloc(png, height * sizeof(png_bytep));
    for (i = 0; i < height; ++i) {
        rows[i] = (png_bytep)(pixels + (height - i - 1) * width * 3);
    }
    png_write_image(png, rows);
    png_write_end(png, info);
    png_free(png, palette);
    png_destroy_write_struct(&png, &info);
    fclose(file);
    printf("PNG: finished at %llu\n", get_time_us());
    
    return;
png_fail:
    fail("Failed to create PNG\n");
}

void render_to_png() {
    render();
    save_to_png();
}

void render_to_screen() {
    render();
    // get back into a state we can draw from
    render_prepare_screen();
}