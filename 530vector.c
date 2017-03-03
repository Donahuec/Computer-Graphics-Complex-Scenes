/*
530vector.c
Written by Liv Phillips for CS311, Winter 2017
Implements functions to work with vectors
*/
/*** In general dimensions ***/
#include <stdio.h>

/* Copies part of a vector to another vector */
void vecOffset(int start, int dim, GLdouble v[], GLdouble vPrime[]){
	for(int i=0;i<dim;i++){
		vPrime[start]=v[i];
		start++;
		
	}
}

/* Assumes that there are dim + 2 arguments, the last dim of which are GLdoubles. 
Sets the dim-dimensional vector v to those GLdoubles. */
void vecSet(int dim, GLdouble v[], ...){
	va_list argumentPointer;
	va_start(argumentPointer, v);
	for(int i=0;i<dim;i++){
		v[i]=va_arg(argumentPointer, GLdouble);
	}
	va_end(argumentPointer);
}

void vecPrint(int dim, GLdouble v[]){
	printf("--------------------\n");
	for(int i=0;i<dim;i++){
		printf("[%f]\n", v[i]);
	}
	printf("--------------------\n");
}

void vecGLuintPrint(int dim, GLuint v[]){
	printf("--------------------\n");
	for(int i=0;i<dim;i++){
		printf("[%u]\n", v[i]);
	}
	printf("--------------------\n");
}

void vecGLfloatPrint(int dim, GLfloat v[]){
	printf("--------------------\n");
	for(int i=0;i<dim;i++){
		printf("[%f]\n", v[i]);
	}
	printf("--------------------\n");
}

/* Copies the dim-dimensional vector v to the dim-dimensional vector copy. */
void vecCopy(int dim, GLdouble v[], GLdouble copy[]) {
	for(int i=0;i<dim;i++){
		copy[i]=v[i];
	}
}

/* Adds the dim-dimensional vectors v and w. */
void vecAdd(int dim, GLdouble v[], GLdouble w[], GLdouble vPlusW[]) {
	for(int i=0;i<dim;i++){
		vPlusW[i]=v[i]+w[i];
	}
}

/* Subtracts the dim-dimensional vectors v and w. */
void vecSubtract(int dim, GLdouble v[], GLdouble w[], GLdouble vMinusW[]) {
	for(int i=0;i<dim;i++){
		vMinusW[i]=v[i]-w[i];
	}
}

/* Scales the dim-dimensional vector w by the number c. */
void vecScale(int dim, GLdouble c, GLdouble w[], GLdouble cTimesW[]) {
	for(int i=0;i<dim;i++){
		cTimesW[i]=w[i]*c;
	}
}
/* Returns the dot product of the dim-dimensional vectors v and w. */
GLdouble vecDot(int dim, GLdouble v[], GLdouble w[]){
	GLdouble vwDot=0;
	for (int i=0; i<dim; i++){
		vwDot+= (v[i]*w[i]);
	}
	return vwDot;
}

/* Returns the length of the dim-dimensional vector v. */
GLdouble vecLength(int dim, GLdouble v[]){
	GLdouble vLength=vecDot(dim, v, v);
	vLength=sqrt(vLength);
	return vLength;
}

/* Returns the length of the dim-dimensional vector v. If the length is 
non-zero, then also places a scaled version of v into the dim-dimensional 
vector unit, so that unit has length 1. */
GLdouble vecUnit(int dim, GLdouble v[], GLdouble unit[]){
	GLdouble vLength=vecDot(dim, v, v);
	vLength=sqrt(vLength);
	if (vLength !=0){
		vecScale(dim, (1.0/vLength), v, unit);
	} else {
		vecCopy(dim, v, unit);
	}
	return vLength;
}

/* Computes the cross product of the 3-dimensional vectors v and w, and places 
it into vCrossW. */
void vec3Cross(GLdouble v[3], GLdouble w[3], GLdouble vCrossW[3]){
	vCrossW[0]=(v[1]*w[2])-(v[2]*w[1]);
	vCrossW[1]=(v[2]*w[0])-(v[0]*w[2]);
	vCrossW[2]=(v[0]*w[1])-(v[1]*w[0]);
}

/* Computes the 3-dimensional vector v from its spherical coordinates. 
rho >= 0.0 is the radius. 0 <= phi <= pi is the co-latitude. -pi <= theta <= pi 
is the longitude or azimuth. */
void vec3Spherical(GLdouble rho, GLdouble phi, GLdouble theta, GLdouble v[3]){
	v[0]=(rho * sin(phi) * cos(theta));
	v[1]=(rho * sin(phi) * sin(theta));
	v[2]=(rho * cos(phi));
}


void vecOpenGL(int dim, GLdouble v[], GLfloat openGL[]) {
	for (int i = 0; i < dim; i += 1)
		openGL[i] = v[i];
}

