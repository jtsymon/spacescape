/* 
 * File:   shader.h
 * Author: jts
 *
 * Created on 3 March 2014, 9:01 AM
 */

#ifndef SHADER_H
#define	SHADER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <GL/glew.h>

extern void shader_error(GLuint object);
extern char* read_file(const char* filename);
extern GLuint create_shader(const char* filename, GLenum type);
extern GLuint create_program(GLuint vertex_shader, GLuint fragment_shader);
extern GLuint create_program_src(const char * vertex_file_path, const char * fragment_file_path);

#endif	/* SHADER_H */

