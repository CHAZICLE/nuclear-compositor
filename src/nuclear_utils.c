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

void print_eglversion(EGLDisplay display)
{
	fprintf(stderr, "print_eglversion (%d):\n\tEGL_CLIENT_APIS: %s\n", display, eglQueryString(display, EGL_CLIENT_APIS));
	fprintf(stderr, "\tEGL_VENDOR: %s\n", eglQueryString(display, EGL_VENDOR));
	fprintf(stderr, "\tEGL_VERSION: %s\n", eglQueryString(display, EGL_VERSION));
	fprintf(stderr, "\tEGL_EXTENSIONS: %s\n\n", eglQueryString(display, EGL_EXTENSIONS));
}
void print_glversion()
{
	fprintf(stderr, "print_glversion:\n");
	fprintf(stderr, "\tGL_VENDOR: %s\n", glGetString(GL_VENDOR));
	fprintf(stderr, "\tGL_RENDERER: %s\n", glGetString(GL_RENDERER));
	fprintf(stderr, "\tGL_VERSION: %s\n", glGetString(GL_VERSION));
	fprintf(stderr, "\tGL_SHADING_LANGUAGE_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	fprintf(stderr, "\tGL_EXTENSIONS: %s\n\n", glGetString(GL_EXTENSIONS));
}
void print_eglerror(const char *err)
{
	switch(eglGetError())
	{
		case EGL_SUCCESS: fprintf(stderr, "Error: (EGL_SUCCESS): %s\n", err); break;
		case EGL_NOT_INITIALIZED: fprintf(stderr, "Error: (EGL_NOT_INITIALIZED): %s\n", err); break;
		case EGL_BAD_ACCESS: fprintf(stderr, "Error: (EGL_BAD_ACCESS): %s\n", err); break;
		case EGL_BAD_ALLOC: fprintf(stderr, "Error: (EGL_BAD_ALLOC): %s\n", err); break;
		case EGL_BAD_ATTRIBUTE: fprintf(stderr, "Error: (EGL_BAD_ATTRIBUTE): %s\n", err); break;
		case EGL_BAD_CONTEXT: fprintf(stderr, "Error: (EGL_BAD_CONTEXT): %s\n", err); break;
		case EGL_BAD_CONFIG: fprintf(stderr, "Error: (EGL_BAD_CONFIG): %s\n", err); break;
		case EGL_BAD_CURRENT_SURFACE: fprintf(stderr, "Error: (EGL_BAD_CURRENT_SURFACE): %s\n", err); break;
		case EGL_BAD_DISPLAY: fprintf(stderr, "Error: (EGL_BAD_DISPLAY): %s\n", err); break;
		case EGL_BAD_SURFACE: fprintf(stderr, "Error: (EGL_BAD_SURFACE): %s\n", err); break;
		case EGL_BAD_MATCH: fprintf(stderr, "Error: (EGL_BAD_MATCH): %s\n", err); break;
		case EGL_BAD_PARAMETER: fprintf(stderr, "Error: (EGL_BAD_PARAMETER): %s\n", err); break;
		case EGL_BAD_NATIVE_PIXMAP: fprintf(stderr, "Error: (EGL_BAD_NATIVE_PIXMAP): %s\n", err); break;
		case EGL_BAD_NATIVE_WINDOW: fprintf(stderr, "Error: (EGL_BAD_NATIVE_WINDOW): %s\n", err); break;
		case EGL_CONTEXT_LOST: fprintf(stderr, "Error: (EGL_CONTEXT_LOST): %s\n", err); break;
	}
}
void print_glerror(const char *err)
{
	switch(glGetError())
	{
		case GL_NO_ERROR: fprintf(stderr, "Error: (GL_NO_ERROR): %s\n", err); break;
		case GL_INVALID_ENUM: fprintf(stderr, "Error: (GL_INVALID_ENUM): %s\n", err); break;
		case GL_INVALID_VALUE: fprintf(stderr, "Error: (GL_INVALID_VALUE): %s\n", err); break;
		case GL_INVALID_OPERATION: fprintf(stderr, "Error: (GL_INVALID_OPERATION): %s\n", err); break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: fprintf(stderr, "Error: (GL_INVALID_FRAMEBUFFER_OPERATION): %s\n", err); break;
		case GL_OUT_OF_MEMORY: fprintf(stderr, "Error: GL_OUT_OF_MEMORY): %s\n", err); break;
	}
}


GLuint load_shader(GLint type, const char *shader_path)
{
	GLuint shaderId = glCreateShader(type);
	if(shaderId==0)
	{
		print_glerror("Failed to create GLSL shader");
		exit(1);
	}

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


#define getGLiv(pname) \
	GLint result_##pname; \
	glGetShaderiv(shaderId, pname, &result_##pname);
	getGLiv(GL_SHADER_TYPE		)
	getGLiv(GL_DELETE_STATUS	)
	getGLiv(GL_COMPILE_STATUS	)
	getGLiv(GL_INFO_LOG_LENGTH	)
	getGLiv(GL_SHADER_SOURCE_LENGTH	)
#undef getGLiv

	if(result_GL_COMPILE_STATUS==GL_FALSE)
	{
		if(result_GL_INFO_LOG_LENGTH>0){
			char *err = malloc(result_GL_INFO_LOG_LENGTH+1);
			err[result_GL_INFO_LOG_LENGTH] = 0;
			glGetShaderInfoLog(shaderId, result_GL_INFO_LOG_LENGTH, NULL, err);
			fprintf(stderr, "Failed to compile shader %s: %d\"%s\"\n%s\n", shader_path, result_GL_INFO_LOG_LENGTH, err, shader_code);
		}
		else
			fprintf(stderr, "Failed to compile shader %s: \n%s\n", shader_path, shader_code);
		exit(2);
	}
	fprintf(stderr, "Shader %s status:\n", shader_path);
#define getGLiv(pname) \
	fprintf(stderr, "\t" #pname ": %d\n", result_##pname);
	getGLiv(GL_SHADER_TYPE		)
	getGLiv(GL_DELETE_STATUS	)
	getGLiv(GL_COMPILE_STATUS	)
	getGLiv(GL_INFO_LOG_LENGTH	)
	getGLiv(GL_SHADER_SOURCE_LENGTH	)
#undef getGLiv

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
