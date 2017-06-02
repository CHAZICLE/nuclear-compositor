#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "icosphere.h"

#define ARRAY_EXTEND_C 32

struct vertex_arraylist {
	float *data;
	int vertex_count,vertex_storage_count;
};
static struct vertex_arraylist *vertex_arraylist_create()
{
	return calloc(1, sizeof(struct vertex_arraylist));
}
static int vertex_arraylist_addVertex(struct vertex_arraylist *v, float x, float y, float z)
{
	if(v->data==0 || v->vertex_storage_count==0 || v->vertex_count+1>=v->vertex_storage_count)
	{
		v->vertex_storage_count += ARRAY_EXTEND_C;
		float *newdata = malloc(sizeof(float)*3*v->vertex_storage_count);
		if(v->data!=0 && v->vertex_count>0)
		{
			memcpy(newdata, v->data, sizeof(float)*3*v->vertex_count);
			free(v->data);
		}
		v->data = newdata;
	}
	float m = sqrt(x*x+y*y+z*z);
	v->data[v->vertex_count*3+0] = x/m;
	v->data[v->vertex_count*3+1] = y/m;
	v->data[v->vertex_count*3+2] = z/m;
	v->vertex_count++;
	return v->vertex_count-1;
}
static void vertex_arraylist_destroy(struct vertex_arraylist *vlist)
{
	free(vlist->data);
	free(vlist);
}

// triangle linked list
struct triangle_linkedlist {
	struct triangle *first,*last;
	int count;
};

struct triangle  {
	struct triangle *next;
	int a,b,c;
};
static struct triangle_linkedlist *triangle_linkedlist_create()
{
	return calloc(1, sizeof(struct triangle_linkedlist));
}
static void triangle_linkedlist_addTriangle(struct triangle_linkedlist *tlist, int a, int b, int c)
{
	struct triangle *tri = malloc(sizeof(struct triangle));
	tri->a = a;
	tri->b = b;
	tri->c = c;
	tri->next = 0;
	if(tlist->last==0)
	{
		tlist->first = tlist->last = tri;
		tlist->count = 1;
	}
	else
	{
		tlist->last->next = tri;
		tlist->last = tri;
		tlist->count++;
	}
}
static int triangle_linkedlist_findc(struct triangle_linkedlist *tlist, int a, int b)
{
	for(struct triangle *tri = tlist->first;tri!=0;tri = tri->next)
	{
		if(tri->a==a && tri->b==b)
			return tri->c;
	}
	return -1;
}
static void triangle_linkedlist_destroy(struct triangle_linkedlist *tlist)
{
	struct triangle *node = tlist->first;
	while(node!=0)
	{
		struct triangle *t = node;
		node = node->next;
		free(t);
	}
	free(tlist);
}
static int getmidpoint(struct vertex_arraylist *vlist, struct triangle_linkedlist *midpoints, int _a, int _b)
{
	int a = _a;
	int b = _b;
	if(a>b)
	{
		a = _b;
		b = _a;
	}
	int c = triangle_linkedlist_findc(midpoints, a, b);
	if(c==-1)
	{
		c = vertex_arraylist_addVertex(vlist,
			(vlist->data[a*3+0]+vlist->data[b*3+0])/2.f,
			(vlist->data[a*3+1]+vlist->data[b*3+1])/2.f,
			(vlist->data[a*3+2]+vlist->data[b*3+2])/2.f
		);
		triangle_linkedlist_addTriangle(midpoints, a, b, c);
	}
	return c;
}

struct icosphere *create_icosphere(int recursion)
{
	struct icosphere *icosphere = malloc(sizeof(struct icosphere));

	// initial vertex build
	struct vertex_arraylist *vlist = vertex_arraylist_create();
	float t = (1.0 + sqrt(5.0)) / 2.0;
	vertex_arraylist_addVertex(vlist, -1,  t,  0);
	vertex_arraylist_addVertex(vlist,  1,  t,  0);
	vertex_arraylist_addVertex(vlist, -1, -t,  0);
	vertex_arraylist_addVertex(vlist,  1, -t,  0);

	vertex_arraylist_addVertex(vlist,  0, -1,  t);
	vertex_arraylist_addVertex(vlist,  0,  1,  t);
	vertex_arraylist_addVertex(vlist,  0, -1, -t);
	vertex_arraylist_addVertex(vlist,  0,  1, -t);

	vertex_arraylist_addVertex(vlist,  t,  0, -1);
	vertex_arraylist_addVertex(vlist,  t,  0,  1);
	vertex_arraylist_addVertex(vlist, -t,  0, -1);
	vertex_arraylist_addVertex(vlist, -t,  0,  1);

	// connect initial build
	struct triangle_linkedlist *tlist = triangle_linkedlist_create();
	triangle_linkedlist_addTriangle(tlist, 0, 11, 5);
	triangle_linkedlist_addTriangle(tlist, 0, 5, 1);
	triangle_linkedlist_addTriangle(tlist, 0, 1, 7);
	triangle_linkedlist_addTriangle(tlist, 0, 7, 10);
	triangle_linkedlist_addTriangle(tlist, 0, 10, 11);

	triangle_linkedlist_addTriangle(tlist, 1, 5, 9);
	triangle_linkedlist_addTriangle(tlist, 5, 11, 4);
	triangle_linkedlist_addTriangle(tlist, 11, 10, 2);
	triangle_linkedlist_addTriangle(tlist, 10, 7, 6);
	triangle_linkedlist_addTriangle(tlist, 7, 1, 8);

	triangle_linkedlist_addTriangle(tlist, 3, 9, 4);
	triangle_linkedlist_addTriangle(tlist, 3, 4, 2);
	triangle_linkedlist_addTriangle(tlist, 3, 2, 6);
	triangle_linkedlist_addTriangle(tlist, 3, 6, 8);
	triangle_linkedlist_addTriangle(tlist, 3, 8, 9);

	triangle_linkedlist_addTriangle(tlist, 4, 9, 5);
	triangle_linkedlist_addTriangle(tlist, 2, 4, 11);
	triangle_linkedlist_addTriangle(tlist, 6, 2, 10);
	triangle_linkedlist_addTriangle(tlist, 8, 6, 7);
	triangle_linkedlist_addTriangle(tlist, 9, 8, 1);

	// subdivide
	struct triangle_linkedlist *midpoints = triangle_linkedlist_create();
	for(int i=0;i<recursion;i++)
	{
		struct triangle_linkedlist *tlist2 = triangle_linkedlist_create();
		for(struct triangle *tri = tlist->first;tri!=0;tri = tri->next)
		{
			int a = getmidpoint(vlist, midpoints, tri->a, tri->b);
			int b = getmidpoint(vlist, midpoints, tri->b, tri->c);
			int c = getmidpoint(vlist, midpoints, tri->c, tri->a);

			triangle_linkedlist_addTriangle(tlist2, tri->a, a, c);
			triangle_linkedlist_addTriangle(tlist2, tri->b, b, a);
			triangle_linkedlist_addTriangle(tlist2, tri->c, c, b);
			triangle_linkedlist_addTriangle(tlist2, a, b, c);
		}
		triangle_linkedlist_destroy(tlist);
		tlist = tlist2;
	}
	
	//vlist->data has vertex position buffer
	icosphere->vertices = vlist->vertex_count;
	icosphere->vertexPositionBuffer = malloc(sizeof(float)*3*icosphere->vertices);
	memcpy(icosphere->vertexPositionBuffer, vlist->data, sizeof(float)*3*icosphere->vertices);

	fprintf(stderr, "Verts: %d\n", icosphere->vertices);

	//tlist to linear index buffer
	icosphere->triangles = tlist->count;
	icosphere->triangleIndexBuffer = malloc(sizeof(unsigned int)*3*icosphere->triangles);
	icosphere->lines = icosphere->triangles*3;
	icosphere->lineIndexBuffer = malloc(sizeof(unsigned int)*icosphere->lines*2);
	int vid = 0;
	for(struct triangle *tri = tlist->first;tri!=0;tri = tri->next)
	{
		icosphere->triangleIndexBuffer[vid*3+0] = tri->a;
		icosphere->triangleIndexBuffer[vid*3+1] = tri->b;
		icosphere->triangleIndexBuffer[vid*3+2] = tri->c;

		icosphere->lineIndexBuffer[vid*6+0] = tri->a;
		icosphere->lineIndexBuffer[vid*6+1] = tri->b;

		icosphere->lineIndexBuffer[vid*6+2] = tri->b;
		icosphere->lineIndexBuffer[vid*6+3] = tri->c;

		icosphere->lineIndexBuffer[vid*6+4] = tri->c;
		icosphere->lineIndexBuffer[vid*6+5] = tri->a;
		vid++;
	}
	
	// cleanup
	triangle_linkedlist_destroy(tlist);
	triangle_linkedlist_destroy(midpoints);
	vertex_arraylist_destroy(vlist);

	return icosphere;
}
