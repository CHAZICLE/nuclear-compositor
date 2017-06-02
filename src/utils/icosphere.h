#ifndef __NUCLEAR_COMPOSITOR_ICOSPHERE_H_INCLUDED__
#define __NUCLEAR_COMPOSITOR_ICOSPHERE_H_INCLUDED__

struct icosphere {
	unsigned int *triangleIndexBuffer;
	int triangles;
	unsigned int *lineIndexBuffer;
	int lines;
	float *vertexPositionBuffer;
	int vertices;
};

struct icosphere *create_icosphere(int recursion);

#endif
