/* 
 * File:   render.h
 * Author: jts
 *
 * Created on 3 March 2014, 9:06 AM
 */

#ifndef RENDER_H
#define	RENDER_H

#include <GL/glew.h>

#include "shader.h"
#include "misc.h"

extern GLuint textures[2], framebuffers[2];
extern GLuint quad_vertexbuffer;
extern bool gen_starfield, gen_stars, gen_nebulae, gen_sun;
extern int width, height;
extern int scale;
extern GLuint program_screen;

extern void render_init();
extern void render_cleanup();
extern void render();

#endif	/* RENDER_H */

