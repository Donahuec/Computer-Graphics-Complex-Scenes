/*
1000mainSwitchLights.c
Written by Josh Davis, adpated by Liv Phillips and Caitlin Donahue for CS311, Winter 2017
Shows Switch Node affecting lights
*/

/* On macOS, compile with...
    clang 1000mainSwitchLights.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <sys/time.h>

double getTime(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

#include "500shader.c"
#include "530vector.c"
#include "580mesh.c"
#include "1000matrix.c"
#include "520camera.c"
#include "540texture.c"
#include "560light.c"
#include "590shadow.c"
#include "1003scene.c"
#include "1000skybox.c"

camCamera cam;
texTexture texH, texV, texW, texT, texL; 
meshGLMesh meshH, meshV, meshW, meshT, meshTMed, meshTFar, meshL, meshLMed, meshLFar; 
/* Updated */
sceneNode nodeH, nodeV, nodeW, nodeT, nodeTMed, nodeTFar, nodeL, nodeLMed, nodeLFar,
	rootNode, transformationNodeT, transformationNodeL,
	transformationNodeT2, transformationNodeL2,
	transformationNodeT3, transformationNodeL3,
	lightNA, lightNB, lightNC, lightND, lightNE, lightNF,
	lodNodeL, lodNodeT, switchNodeT;
/* We need just one shadow program, because all of our meshes have the same 
attribute structure. */
shadowProgram sdwProg;

lightLight lightA, lightB, lightC, lightD, lightE, lightF;
shadowMap sdwMapA;
/* The main shader program has extra hooks for shadowing. */
GLuint program;
GLint viewingLoc, modelingLoc, projLoc, distFromCam;

GLint unifLocs[1], textureLocs[1];
GLint attrLocs[3];
GLint lightPosLoc[2], lightColLoc[2], lightAttLoc[2], lightDirLoc[2], lightCosLoc[2];
GLint camPosLoc;
GLint viewingSdwLoc[2], textureSdwLoc[2];

skyboxSkybox skyboxPersp, skyboxOrtho;

void handleError(int error, const char *description) {
	fprintf(stderr, "handleError: %d\n%s\n", error, description);
}

void handleResize(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
	camSetWidthHeight(&cam, width, height);
}

void handleKey(GLFWwindow *window, int key, int scancode, int action,
		int mods) {
	int shiftIsDown = mods & GLFW_MOD_SHIFT;
	int controlIsDown = mods & GLFW_MOD_CONTROL;
	int altOptionIsDown = mods & GLFW_MOD_ALT;
	int superCommandIsDown = mods & GLFW_MOD_SUPER;
	if (action == GLFW_PRESS && key == GLFW_KEY_L) {
		camSwitchProjectionType(&cam);
	} else if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (key == GLFW_KEY_O)
			camAddTheta(&cam, -0.1);
		else if (key == GLFW_KEY_P)
			camAddTheta(&cam, 0.1);
		else if (key == GLFW_KEY_I)
			camAddPhi(&cam, -0.1);
		else if (key == GLFW_KEY_K)
			camAddPhi(&cam, 0.1);
		else if (key == GLFW_KEY_U)
			camAddDistance(&cam, -1.5);
		else if (key == GLFW_KEY_J)
			camAddDistance(&cam, 1.5);
		else if (key == GLFW_KEY_S)
			/* use the s key to cycle through the switch node options*/
			sceneCycleSwitch(&switchNodeT);
		/* use shift to alter which light moves */
		else if (key == GLFW_KEY_C) {
			if(shiftIsDown==0){
				GLdouble vec[3];
				vecCopy(3, cam.target, vec);
				vec[0] -= 1.0;
				camSetTarget(&cam, vec);
			} else {
				GLdouble vec[3];
				vecCopy(3, cam.target, vec);
				vec[0] += 1.0;
				camSetTarget(&cam, vec);
			}
		} else if (key == GLFW_KEY_V) {
			if(shiftIsDown==0){
				GLdouble vec[3];
				vecCopy(3, cam.target, vec);
				vec[1] -= 1.0;
				camSetTarget(&cam, vec);
			} else {
				GLdouble vec[3];
				vecCopy(3, cam.target, vec);
				vec[1] += 1.0;
				camSetTarget(&cam, vec);
			}
		} else if (key == GLFW_KEY_B) {
			if(shiftIsDown==0){
				GLdouble vec[3];
				vecCopy(3, cam.target, vec);
				vec[2] -= 1.0;
				camSetTarget(&cam, vec);
			} else {
				GLdouble vec[3];
				vecCopy(3, cam.target, vec);
				vec[2] += 1.0;
				camSetTarget(&cam, vec);
			}
		}
		
	}
}

/* Returns 0 on success, non-zero on failure. Warning: If initialization fails 
midway through, then does not properly deallocate all resources. But that's 
okay, because the program terminates almost immediately after this function 
returns. */
int initializeCameraLight(void) {
    // GLdouble vec[3] = {50.0, 30.0, 5.0};
    GLdouble vec[3] = {50.0, 160.0, 7.0};
	camSetControls(&cam, camPERSPECTIVE, M_PI / 4, 10.0, 100.0, 100.0, 180.0, 
		1.3, -1.5, vec);
	sceneSetCamera(&rootNode, &cam);
	
	lightSetType(&lightA, lightSPOT);
	lightSetType(&lightB, lightSPOT);
	lightSetType(&lightC, lightSPOT);
	lightSetType(&lightD, lightSPOT);
	lightSetType(&lightE, lightSPOT);
	lightSetType(&lightF, lightSPOT);
	
	vecSet(3, vec, 50.0, 90.0, 100.0);
	lightShineFrom(&lightA, vec, M_PI, M_PI / 2.0 );
	lightSetSpotAngle(&lightA, M_PI / 2.0);
	lightShineFrom(&lightB, vec, M_PI, M_PI / 2.0 );
	lightSetSpotAngle(&lightB, M_PI / 2.0);
	lightShineFrom(&lightC, vec, M_PI, M_PI / 2.0 );
	lightSetSpotAngle(&lightC, M_PI / 2.0);
	lightShineFrom(&lightD, vec, M_PI, M_PI / 2.0 );
	lightSetSpotAngle(&lightD, M_PI / 2.0);
	lightShineFrom(&lightE, vec, M_PI, M_PI / 2.0 );
	lightSetSpotAngle(&lightE, M_PI / 2.0);
	lightShineFrom(&lightF, vec, M_PI, M_PI / 2.0 );
	lightSetSpotAngle(&lightF, M_PI / 2.0);
	
	vecSet(3, vec, 1.0, 1.0, 1.0);
	lightSetColor(&lightA, vec);
	
	vecSet(3, vec, 0.9, 0.8, 0.8);
	lightSetColor(&lightB, vec);
	
	vecSet(3, vec, 0.8, 0.7, 0.7);
	lightSetColor(&lightC, vec);
	
	vecSet(3, vec, 0.6, 0.6, 0.6);
	lightSetColor(&lightD, vec);
	
	vecSet(3, vec, 0.5, 0.5, 0.5);
	lightSetColor(&lightE, vec);
	
	vecSet(3, vec, 0.35, 0.35, 0.35);
	lightSetColor(&lightF, vec);

	vecSet(3, vec, 1.0, 0.0, 0.0);
	lightSetAttenuation(&lightA, vec);
	lightSetAttenuation(&lightB, vec);
	lightSetAttenuation(&lightC, vec);
	lightSetAttenuation(&lightD, vec);
	lightSetAttenuation(&lightE, vec);
	lightSetAttenuation(&lightF, vec);

	/* Configure shadow mapping. */
	if (shadowProgramInitialize(&sdwProg, 3) != 0)
		return 1;
	if (shadowMapInitialize(&sdwMapA, 1024, 1024) != 0)
		return 2;
	return 0;
}


/* Returns 0 on success, non-zero on failure. Warning: If initialization fails 
midway through, then does not properly deallocate all resources. But that's 
okay, because the program terminates almost immediately after this function 
returns. */
int initializeScene(void) {
	/*init textures*/
	if (texInitializeFile(&texH, "grass.png", GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 1;
    if (texInitializeFile(&texV, "granite 2.jpg", GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 2;
    if (texInitializeFile(&texW, "water.png", GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 3;
    if (texInitializeFile(&texT, "trunk 2.jpg", GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 4;
    if (texInitializeFile(&texL, "tree.png", GL_LINEAR, GL_LINEAR, 
    		GL_REPEAT, GL_REPEAT) != 0)
    	return 5;
	/*init attrs*/
	GLuint attrDims[3] = {3, 2, 3};
    double zs[24][38] = {
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 20.0, 20.0, 20.0, 25.0, 25.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 10.0, 10.0, 10.0, 12.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 20.0, 20.0, 25.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 10.0, 10.0, 10.0, 10.0, 5.0, 5.0, 5.0, 20.0, 25.0, 25.0, 25.0, 27.0, 27.0, 27.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.0, 20.0, 20.0, 20.0, 25.0, 25.0, 25.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 7.0, 7.0, 20.0, 20.0, 25.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 5.0, 7.0, 7.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 10.0, 10.0, 15.0, 15.0, 20.0, 20.0, 25.0, 30.0, 35.0, 30.0, 25.0, 20.0, 20.0, 15.0, 15.0, 10.0, 10.0, 7.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 10.0, 10.0, 15.0, 17.0, 20.0, 20.0, 25.0, 30.0, 35.0, 30.0, 25.0, 20.0, 20.0, 15.0, 15.0, 10.0, 10.0, 7.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 10.0, 10.0, 15.0, 15.0, 20.0, 20.0, 25.0, 30.0, 35.0, 30.0, 25.0, 20.0, 20.0, 15.0, 15.0, 10.0, 10.0, 7.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 10.0, 10.0, 15.0, 17.0, 20.0, 20.0, 25.0, 30.0, 35.0, 30.0, 25.0, 20.0, 20.0, 15.0, 15.0, 10.0, 10.0, 7.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 10.0, 10.0, 15.0, 15.0, 20.0, 20.0, 25.0, 30.0, 35.0, 30.0, 25.0, 20.0, 20.0, 15.0, 15.0, 10.0, 10.0, 7.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 7.0, 5.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 5.0, 5.0, 7.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 7.0, 7.0, 5.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 7.0, 5.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}, 
		{10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 10.0, 15.0, 20.0, 20.0, 20.0, 20.0, 25.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0},
		{10.0, 10.0, 10.0, 10.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 5.0, 10.0, 15.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 25.0, 25.0, 25.0, 30.0, 30.0, 35.0, 30.0, 25.0, 20.0, 20.0, 15.0, 15.0, 10.0, 10.0, 7.0, 5.0},
		{10.0, 10.0, 10.0, 10.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 5.0, 10.0, 15.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 25.0, 25.0, 25.0, 30.0, 30.0, 35.0, 30.0, 25.0, 20.0, 20.0, 15.0, 15.0, 10.0, 10.0, 7.0, 5.0},
		{10.0, 10.0, 10.0, 10.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 5.0, 10.0, 15.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 25.0, 25.0, 25.0, 30.0, 30.0, 35.0, 30.0, 25.0, 20.0, 20.0, 15.0, 15.0, 10.0, 10.0, 7.0, 5.0},
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0},
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0},
		{5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 20.0, 7.0, 7.0, 5.0, 5.0}};
	double ws[24][38] = {
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}, 
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
		{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0}};
		
	/*init meshes*/
	meshMesh mesh, meshLand;
	if (meshInitializeLandscape(&meshLand, 24, 38, 5.0, (double *)zs) != 0)
		return 6;
	if (meshInitializeDissectedLandscape(&mesh, &meshLand, M_PI / 3.0, 1) != 0)
		return 7;
	meshGLInitialize(&meshH, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshH, 0, attrLocs);
	meshGLVAOInitialize(&meshH, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeDissectedLandscape(&mesh, &meshLand, M_PI / 3.0, 0) != 0)
		return 8;
	meshDestroy(&meshLand);
	double *vert, normal[2];
	for (int i = 0; i < mesh.vertNum; i += 1) {
		vert = meshGetVertexPointer(&mesh, i);
		normal[0] = -vert[6];
		normal[1] = vert[5];
		vert[3] = (vert[0] * normal[0] + vert[1] * normal[1]) / 20.0;
		vert[4] = vert[2] / 20.0;
	}
	/**/
	meshGLInitialize(&meshV, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshV, 0, attrLocs);
	meshGLVAOInitialize(&meshV, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeLandscape(&mesh, 24, 38, 5.0, (double *)ws) != 0)
		return 9;
	/*Water*/
	meshGLInitialize(&meshW, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshW, 0, attrLocs);
	meshGLVAOInitialize(&meshW, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeCapsule(&mesh, 1.0, 10.0, 1, 8) != 0)
		return 10;
	/*Tree*/
	meshGLInitialize(&meshT, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshT, 0, attrLocs);
	meshGLVAOInitialize(&meshT, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeBox(&mesh, -1.0, 0.5, -1.0, 0.5, -2.0, 3.0))
		return 11;
	meshGLInitialize(&meshTMed, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshTMed, 0, attrLocs);
	meshGLVAOInitialize(&meshTMed, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeBox(&mesh, -0.5, 0.0, -0.5, 0.0, -2.0, 5.0))
		return 12;
	meshGLInitialize(&meshTFar, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshTFar, 0, attrLocs);
	meshGLVAOInitialize(&meshTFar, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeSphere(&mesh, 5.0, 8, 16) != 0)
		return 13;
	meshGLInitialize(&meshL, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshL, 0, attrLocs);
	meshGLVAOInitialize(&meshL, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeSphere(&mesh, 5.0, 4, 8) != 0)
		return 14;
	meshGLInitialize(&meshLMed, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshLMed, 0, attrLocs);
	meshGLVAOInitialize(&meshLMed, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	if (meshInitializeBox(&mesh, -2.0, 2.0, -2.0, 2.0, -2.0, 1.0))
		return 15;
	meshGLInitialize(&meshLFar, &mesh, 3, attrDims, 2);
	meshGLVAOInitialize(&meshLFar, 0, attrLocs);
	meshGLVAOInitialize(&meshLFar, 1, sdwProg.attrLocs);
	meshDestroy(&mesh);
	/*switch cube*/
	
	/*Initialize scene nodes*/
	if (sceneInitializeGeometry(&nodeW, 3, 1, &meshW, NULL, NULL) != 0)
		return 22;
	if (sceneInitializeGeometry(&nodeL, 3, 1, &meshL, NULL, NULL) != 0)
		return 23;
	if (sceneInitializeGeometry(&nodeLMed, 3, 1, &meshLMed, NULL, NULL)!= 0)
		return 24;
	if (sceneInitializeGeometry(&nodeLFar, 3, 1, &meshLFar, NULL, NULL)!= 0)
		return 25;
	if (sceneInitializeLOD(&lodNodeL, 3, 3, NULL, NULL, NULL) != 0)
		return 26;
	if (sceneInitializeTransformation(&transformationNodeL3, 3, NULL, NULL, &lodNodeL, NULL) != 0)
		return 31;
	if (sceneInitializeTransformation(&transformationNodeL2, 3, NULL, NULL, &lodNodeL, &transformationNodeL3) != 0)
		return 31;
	if (sceneInitializeTransformation(&transformationNodeL, 3, NULL, NULL, &lodNodeL, &transformationNodeL2) != 0)
		return 31;
	if (sceneInitializeGeometry(&nodeT, 3, 1, &meshT, &transformationNodeL, NULL) != 0)
		return 27;
	if(sceneInitializeGeometry(&nodeTMed, 3, 1, &meshTMed, &transformationNodeL, NULL) != 0)
		return 28;
	if(sceneInitializeGeometry(&nodeTFar, 3, 1, &meshTFar, &transformationNodeL, NULL) != 0)
		return 29;
	if (sceneInitializeLOD(&lodNodeT, 3, 3, NULL, NULL, NULL) != 0)
		return 30;
	if (sceneInitializeTransformation(&transformationNodeT3, 3, NULL, NULL, &lodNodeT, &nodeW) != 0)
		return 32; 
	if (sceneInitializeTransformation(&transformationNodeT2, 3, NULL, NULL, &lodNodeT, &transformationNodeT3) != 0)
		return 32; 
	if (sceneInitializeTransformation(&transformationNodeT, 3, NULL, NULL, &lodNodeT, &transformationNodeT2) != 0)
		return 32; 
	if (sceneInitializeGeometry(&nodeV, 3, 1, &meshV, NULL, &transformationNodeT) != 0)
		return 33;
	if (sceneInitializeGeometry(&nodeH, 3, 1, &meshH, &nodeV, NULL) != 0)
		return 34;
	if (sceneInitializeLight(&lightNA, 3, &lightA, NULL, NULL) != 0)
		return 35;
	if (sceneInitializeLight(&lightNB, 3, &lightB, NULL, NULL) != 0)
		return 35;
	if (sceneInitializeLight(&lightNC, 3, &lightC, NULL, NULL) != 0)
		return 35;
	if (sceneInitializeLight(&lightND, 3, &lightD, NULL, NULL) != 0)
		return 35;
	if (sceneInitializeLight(&lightNE, 3, &lightE, NULL, NULL) != 0)
		return 35;
	if (sceneInitializeLight(&lightNF, 3, &lightF, NULL, NULL) != 0)
		return 35;
	if (sceneInitializeSwitch(&switchNodeT, 3, 12, NULL, NULL) != 0)
		return 37;
	if (sceneInitializeCamera(&rootNode, 3, NULL, NULL, &nodeH, &switchNodeT) != 0)
		return 38; 

	GLdouble unif[3] = {0.0, 0.0, 0.0};
	sceneSetUniform(&nodeH, unif);
	sceneSetUniform(&nodeV, unif);
	sceneSetUniform(&nodeT, unif);
	sceneSetUniform(&nodeTMed, unif);
	sceneSetUniform(&nodeTFar, unif);
	sceneSetUniform(&nodeL, unif);
	sceneSetUniform(&nodeLMed, unif);
	sceneSetUniform(&nodeLFar, unif);
	sceneSetUniform(&transformationNodeT, unif);
	sceneSetUniform(&transformationNodeL, unif);
	sceneSetUniform(&lightNA, unif);
	sceneSetUniform(&lightNB, unif);
	sceneSetUniform(&lightNC, unif);
	sceneSetUniform(&lightND, unif);
	sceneSetUniform(&lightNE, unif);
	sceneSetUniform(&lightNF, unif);
	sceneSetUniform(&lodNodeL, unif);
	sceneSetUniform(&lodNodeT, unif);
	sceneSetUniform(&rootNode, unif);
	vecSet(3, unif, 1.0, 1.0, 1.0);
	sceneSetUniform(&nodeW, unif);
	
	texTexture *tex;
	tex = &texH;
	sceneSetTexture(&nodeH, &tex);
	tex = &texV;
	sceneSetTexture(&nodeV, &tex);
	tex = &texW;
	sceneSetTexture(&nodeW, &tex);
	tex = &texT;
	sceneSetTexture(&nodeT, &tex);
	sceneSetTexture(&nodeTMed, &tex);
	sceneSetTexture(&nodeTFar, &tex);
	tex = &texL;
	sceneSetTexture(&nodeL, &tex);
	sceneSetTexture(&nodeLMed, &tex);
	sceneSetTexture(&nodeLFar, &tex);
	
	GLdouble transl[3] = {45.0, 70.0, 5.0};
	sceneSetTranslation(&transformationNodeT, transl);
	vecSet(3, transl, 0.0, 0.0, 7.0);
	sceneSetTranslation(&transformationNodeL, transl);
	vecSet(3, transl, 15.0, 112.0, 28.0);
	sceneSetTranslation(&transformationNodeT2, transl);
	vecSet(3, transl, 0.0, 0.0, 7.0);
	sceneSetTranslation(&transformationNodeL2, transl);
	vecSet(3, transl, 90.0, 150.0, 30.0);
	sceneSetTranslation(&transformationNodeT3, transl);
	vecSet(3, transl, 0.0, 0.0, 7.0);
	sceneSetTranslation(&transformationNodeL3, transl);
	
	GLint r[3] = {100, 130, 150};
	sceneSetRanges(&lodNodeL, r);
	sceneSetRanges(&lodNodeT, r);
	
	sceneNode *childrenL[3];
	childrenL[0] = &nodeL;
	childrenL[1] = &nodeLMed;
	childrenL[2] = &nodeLFar;
	sceneSetChildArray(&lodNodeL, childrenL);
	sceneNode *childrenT[3];
	childrenT[0] = &nodeT;
	childrenT[1] = &nodeTMed;
	childrenT[2] = &nodeTFar;
	sceneSetChildArray(&lodNodeT, childrenT);
	
	sceneNode *childrenS[12];
	childrenS[0] = &lightNA;
	childrenS[1] = &lightNB;
	childrenS[2] = &lightNC;
	childrenS[3] = &lightND;
	childrenS[4] = &lightNE;
	childrenS[5] = &lightNF;
	childrenS[6] = &lightNF;
	childrenS[7] = &lightNE;
	childrenS[8] = &lightND;
	childrenS[9] = &lightNC;
	childrenS[10] = &lightNB;
	childrenS[11] = &lightNA;
	sceneSetChildArraySwitch(&switchNodeT, childrenS);
	
	sceneSetLightLocations(&lightNA, lightPosLoc[0], lightColLoc[0], 
		lightAttLoc[0], lightDirLoc[0], lightCosLoc[0]);
	sceneSetShadowLocations(&lightNA, viewingSdwLoc[0], 7, textureSdwLoc[0]);
	sceneSetShadowMap(&lightNA, &sdwMapA);
	
	sceneSetLightLocations(&lightNB, lightPosLoc[0], lightColLoc[0], 
		lightAttLoc[0], lightDirLoc[0], lightCosLoc[0]);
	sceneSetShadowLocations(&lightNB, viewingSdwLoc[0], 7, textureSdwLoc[0]);
	sceneSetShadowMap(&lightNB, &sdwMapA);
	
	sceneSetLightLocations(&lightNC, lightPosLoc[0], lightColLoc[0], 
		lightAttLoc[0], lightDirLoc[0], lightCosLoc[0]);
	sceneSetShadowLocations(&lightNC, viewingSdwLoc[0], 7, textureSdwLoc[0]);
	sceneSetShadowMap(&lightNC, &sdwMapA);
	
	sceneSetLightLocations(&lightND, lightPosLoc[0], lightColLoc[0], 
		lightAttLoc[0], lightDirLoc[0], lightCosLoc[0]);
	sceneSetShadowLocations(&lightND, viewingSdwLoc[0], 7, textureSdwLoc[0]);
	sceneSetShadowMap(&lightND, &sdwMapA);
	
	sceneSetLightLocations(&lightNE, lightPosLoc[0], lightColLoc[0], 
		lightAttLoc[0], lightDirLoc[0], lightCosLoc[0]);
	sceneSetShadowLocations(&lightNE, viewingSdwLoc[0], 7, textureSdwLoc[0]);
	sceneSetShadowMap(&lightNE, &sdwMapA);
	
	sceneSetLightLocations(&lightNF, lightPosLoc[0], lightColLoc[0], 
		lightAttLoc[0], lightDirLoc[0], lightCosLoc[0]);
	sceneSetShadowLocations(&lightNF, viewingSdwLoc[0], 7, textureSdwLoc[0]);
	sceneSetShadowMap(&lightNF, &sdwMapA);

	return 0;
}

void destroyScene(void) {
	texDestroy(&texH);
	texDestroy(&texV);
	texDestroy(&texW);
	texDestroy(&texT);
	texDestroy(&texL);
	meshGLDestroy(&meshH);
	meshGLDestroy(&meshV);
	meshGLDestroy(&meshW);
	meshGLDestroy(&meshT);
	meshGLDestroy(&meshTMed);
	meshGLDestroy(&meshTFar);
	meshGLDestroy(&meshL);
	meshGLDestroy(&meshLMed);
	meshGLDestroy(&meshLFar);
	sceneDestroyRecursively(&rootNode);
}

int initializeShaderProgram(void) {
	GLchar vertexCode[] = "\
		#version 140\n\
		uniform mat4 modeling;\
		uniform mat4 proj;\
		uniform mat4 viewing;\
		uniform mat4 viewingSdw;\
		in vec3 position;\
		in vec2 texCoords;\
		in vec3 normal;\
		out vec3 fragPos;\
		out vec3 normalDir;\
		out vec2 st;\
		out vec4 fragSdw;\
		out vec4 eyeView;\
		void main(void) {\
			mat4 scaleBias = mat4(\
				0.5, 0.0, 0.0, 0.0, \
				0.0, 0.5, 0.0, 0.0, \
				0.0, 0.0, 0.5, 0.0, \
				0.5, 0.5, 0.5, 1.0);\
			vec4 worldPos = modeling * vec4(position, 1.0);\
			gl_Position = proj * vec4(position, 1.0);\
			eyeView = viewing * worldPos;\
			fragSdw = scaleBias * viewingSdw * worldPos;\
			fragPos = vec3(worldPos);\
			normalDir = vec3(modeling * vec4(normal, 0.0));\
			st = texCoords;\
		}";
	GLchar fragmentCode[] = "\
		#version 140\n\
		uniform sampler2D texture0;\
		uniform vec3 specular;\
		uniform vec3 camPos;\
		uniform vec3 lightPos;\
		uniform vec3 lightCol;\
		uniform vec3 lightAtt;\
		uniform vec3 lightAim;\
		uniform float lightCos;\
		uniform sampler2DShadow textureSdw;\
		in vec4 eyeView;\
		in vec3 fragPos;\
		in vec3 normalDir;\
		in vec2 st;\
		in vec4 fragSdw;\
		out vec4 fragColor;\
		void main(void) {\
			vec3 view = normalize(camPos-fragPos);\
			float rim = 1 - max(dot(view, normalDir), 0.0);\
			rim = smoothstep(0.6, 1.0, rim);\
			vec3 finalRim = vec3(0.0, 0.0, 0.2) * vec3(rim, rim, rim);\
			vec3 diffuse = vec3(texture(texture0, st));\
			vec3 norDir = normalize(normalDir);\
	        vec3 camDir = normalize(camPos - fragPos);\
	        float shininess = 64.0;\
			vec3 litDir = normalize(lightPos - fragPos);\
			vec3 refDir = 2.0 * dot(litDir, norDir) * norDir - litDir;\
			float d = distance(lightPos, fragPos);\
			float a = lightAtt[0] + lightAtt[1] * d + lightAtt[2] * d * d;\
			float diffInt = dot(norDir, litDir) / a;\
			float specInt = dot(refDir, camDir);\
			if (dot(lightAim, -litDir) < lightCos)\
				diffInt = 0.0;\
			if (diffInt <= 0.0 || specInt <= 0.0)\
	            specInt = 0.0;\
			float sdw = textureProj(textureSdw, fragSdw);\
			diffInt *= sdw;\
			specInt *= sdw;\
			vec3 diffRefl = max(0.3, diffInt) * lightCol * diffuse;\
    		vec3 specRefl = pow(specInt, shininess) * lightCol * specular;\
    		fragColor = vec4(diffRefl+specRefl, 1.0);\
		}";
	program = makeProgram(vertexCode, fragmentCode);
	if (program != 0) {
		glUseProgram(program);
		attrLocs[0] = glGetAttribLocation(program, "position");
		attrLocs[1] = glGetAttribLocation(program, "texCoords");
		attrLocs[2] = glGetAttribLocation(program, "normal");
		modelingLoc = glGetUniformLocation(program, "modeling");
		projLoc = glGetUniformLocation(program, "proj");
		unifLocs[0] = glGetUniformLocation(program, "specular");
		textureLocs[0] = glGetUniformLocation(program, "texture0");
		camPosLoc = glGetUniformLocation(program, "camPos");
		lightPosLoc[0] = glGetUniformLocation(program, "lightPos");
		lightColLoc[0] = glGetUniformLocation(program, "lightCol");
		lightAttLoc[0] = glGetUniformLocation(program, "lightAtt");
		lightDirLoc[0] = glGetUniformLocation(program, "lightAim");
		lightCosLoc[0] = glGetUniformLocation(program, "lightCos");
		viewingSdwLoc[0] = glGetUniformLocation(program, "viewingSdw");
		textureSdwLoc[0] = glGetUniformLocation(program, "textureSdw");
		distFromCam = glGetUniformLocation(program, "camDist");
	}
	return (program == 0);
}

void render(void) {
	GLdouble identity[4][4];
	mat44Identity(identity);
	/* Save the viewport transformation. */
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	/* For each shadow-casting light, render its shadow map using minimal 
	uniforms and textures. */
	GLint sdwTextureLocs[1] = {-1};
	
	shadowMapRender(&sdwMapA, &sdwProg, &lightA, -100.0, -1.0);
	sceneRender(&nodeH, identity, identity, identity, sdwProg.modelingLoc, sdwProg.modelingLoc, 0, NULL, NULL, 1, 
		sdwTextureLocs, -1, -1);
	shadowMapUnrender();
	
	/* Finish preparing the shadow maps, restore the viewport, and begin to 
	render the scene. */
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if(cam.projectionType==camPERSPECTIVE){
		skyboxSkyboxRender(&skyboxPersp, &rootNode);
	}
	glUseProgram(program);

	GLuint unifDims[1] = {3};
	sceneRender(&rootNode, identity, identity, identity, modelingLoc, projLoc, 1, unifDims, 
		unifLocs, 0, textureLocs, camPosLoc, distFromCam);
		
	if (cam.projectionType==camORTHOGRAPHIC){
		skyboxSkyboxRender(&skyboxOrtho, &rootNode);
	}
	
	shadowUnrender(GL_TEXTURE6);
	shadowUnrender(GL_TEXTURE7);
}

int main(void) {
	double oldTime;
	double newTime = getTime();
    glfwSetErrorCallback(handleError);
	
    if (glfwInit() == 0) {
    	fprintf(stderr, "main: glfwInit failed.\n");
        return 1;
    }
	
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window;
    window = glfwCreateWindow(768, 768, "Final Project", NULL, NULL);
	
    if (window == NULL) {
    	fprintf(stderr, "main: glfwCreateWindow failed.\n");
        glfwTerminate();
        return 2;
    }
	
    glfwSetWindowSizeCallback(window, handleResize);
    glfwSetKeyCallback(window, handleKey);
    glfwMakeContextCurrent(window);
	
    if (gl3wInit() != 0) {
    	fprintf(stderr, "main: gl3wInit failed.\n");
    	glfwDestroyWindow(window);
    	glfwTerminate();
    	return 3;
    }
	
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n", 
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
		
	/* We no longer do glDepthRange(1.0, 0.0). Instead we have changed our 
	projection matrices. */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0, 0.0, 0.4, 1.0);
	
    if (initializeShaderProgram() != 0)
    	return 4;
	
    skyboxInitializeSkyboxShader(&skyboxPersp);
    skyboxInitializeSkyboxShader(&skyboxOrtho);
	
    /* Initialize the shadow mapping before the meshes. Why? */
	if (initializeCameraLight() != 0)
		return 5;
	
	char *skyboxTex[] = {"purplenebula_ft.tga", "purplenebula_bk.tga", "purplenebula_up.tga",
		 "purplenebula_dn.tga", "purplenebula_lf.tga", "purplenebula_rt.tga"};
	skyboxInitializeSkybox(&skyboxOrtho, skyboxTex, NULL, 0);
	skyboxInitializeSkybox(&skyboxPersp, skyboxTex, NULL, 1);
	
    if (initializeScene() != 0)
    	return 6;
	
    while (glfwWindowShouldClose(window) == 0) {
    	oldTime = newTime;
    	newTime = getTime();
    	if (floor(newTime) - floor(oldTime) >= 1.0){
			fprintf(stderr, "main: %f frames/sec\n", 1.0 / (newTime - oldTime));
			sceneCycleSwitch(&switchNodeT);
    	}
			
		render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
	
    /* Deallocate more resources than ever. */
    shadowProgramDestroy(&sdwProg);
    shadowMapDestroy(&sdwMapA);
    glDeleteProgram(program);
    destroyScene();
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


