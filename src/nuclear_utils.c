#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nuclear_utils.h"

PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = 0;
PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = 0;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC c_glEGLImageTargetTexture2DOES = 0;
PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = 0;

void init_egl()
{
	eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL) eglGetProcAddress ("eglBindWaylandDisplayWL");
	eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL) eglGetProcAddress ("eglQueryWaylandBufferWL");
	c_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress ("glEGLImageTargetTexture2DOES");
	eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
}

GLuint load_shader(GLint type, const char *shader_path)
{
	GLuint shaderId = glCreateShader(type);

	FILE *fs = fopen(shader_path, "rb");
	fseek(fs, 0, SEEK_END);
	long fsize = ftell(fs);
	fseek(fs, 0, SEEK_SET);
	char *shader_code = malloc(fsize+1);
	shader_code[fsize] = 0;
	fread(shader_code, fsize, 1, fs);
	glShaderSource(shaderId, 1, (const char *const *)&shader_code , NULL);
	glCompileShader(shaderId);
	fclose(fs);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		char *err = malloc(InfoLogLength+1);
		glGetShaderInfoLog(shaderId, InfoLogLength, NULL, &err[0]);
		fprintf(stderr, "Failed to compile shader %s: \"%s\"\n%s\n", shader_path, err, shader_code);
		exit(1);
	}
	if(!Result)
	{
		fprintf(stderr, "Failed to compile shader %s: \n%s\n", shader_path, shader_code);
		exit(2);
	}
	free(shader_code);

	return shaderId;
}
GLuint loadShaders(const char *vertexShaderName, const char *fragmentShaderName)
{
	GLuint VertexShaderID = load_shader(GL_VERTEX_SHADER, vertexShaderName);
	GLuint FragmentShaderID = load_shader(GL_FRAGMENT_SHADER, fragmentShaderName);

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);
	//glDetachShader(ProgramID, VertexShaderID);
	//glDetachShader(ProgramID, FragmentShaderID);
	//glDeleteShader(VertexShaderID);
	//glDeleteShader(FragmentShaderID);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		char *err = malloc(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &err[0]);
		fprintf(stderr, "Failed to link shader program (%s, %s): %s\n", vertexShaderName, fragmentShaderName, err);
		exit(1);
	}
	if(!Result)
		exit(2);

	return ProgramID;
}
