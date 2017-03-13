/*
1000scene.c
Written by Josh Davis, adpated by Liv Phillips & Caitlin Donahue for CS311, Winter 2017
Program implementing scene graph
*/

#define sceneTRANSFORMATION 0
#define sceneGEOMETRY 		1
#define sceneCAMERA 		2
#define sceneLIGHT 			3
#define sceneLOD 			4
#define sceneSWITCH         5

#define sceneCASTSHADOWS 0
#define sceneNOSHADOWS 1

GLenum units[9]={GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, 
				 GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7, GL_TEXTURE8};

/*** Creation and destruction ***/

/* Feel free to read from this struct's members, but don't write to them except 
through the accessor functions. */
typedef struct sceneNode sceneNode;
struct sceneNode {
	GLuint nodeType;
	GLuint unifDim;
	GLdouble *unif;

	// Transformation node
	GLdouble rotation[3][3];
	GLdouble translation[3];

	// Geometry node
	meshGLMesh *meshGL;
	sceneNode *firstChild, *nextSibling;
	GLuint texNum;
	texTexture **tex;

	// Camera node
	camCamera *cam;

	// Light node
	GLuint hasShadows;
	lightLight *light;
	GLint lightPosition, lightColor, lightAtten, lightDir, lightCos;
	shadowMap *sdwMap;
	GLint viewingSdw, textureUnit, textureSdw;

	// LOD node
	GLint *ranges, rangeDim;
	sceneNode **firstChildMeshes;
	
	//Switch node
	sceneNode **firstChildNodes;
	GLuint numSwitches;
	GLuint curSwitch;
};

int sceneInitializeDefaults(sceneNode *node, GLuint unifDim,
		sceneNode *firstChild, sceneNode *nextSibling) {
    node->unif = (GLdouble *)malloc(unifDim * sizeof(GLdouble));
    if (node->unif == NULL)
        return 1;
	node->unifDim = unifDim;
	node->firstChild = firstChild;
	node->nextSibling = nextSibling;
    mat33Identity(node->rotation);
	vecSet(3, node->translation, 0.0, 0.0, 0.0);
	return 0;
}

/* Initializes a transformation Node that handles rotation and translation for all child nodes.
 The user must remember to call sceneDestroy or 
sceneDestroyRecursively when finished. Returns 0 if no error occurred. */
int sceneInitializeTransformation(sceneNode *node, GLuint unifDim, GLdouble rotation[3][3],
		GLdouble translation[3], sceneNode *firstChild, sceneNode *nextSibling) {
	node->nodeType = sceneTRANSFORMATION;
	if (sceneInitializeDefaults(node, unifDim, firstChild, nextSibling) != 0)
		return 1;
	
    if(translation!=NULL){
    	vecCopy(3, node->translation, translation);
    }
    if(rotation!=NULL){
    	vecCopy(9, (GLdouble *)rotation, (GLdouble *)(node->rotation));
    }
	return 0;
}

/* Initializes a Geometry Node. This handles the mesh and textures of a unit.
 The user must remember to call sceneDestroy or 
sceneDestroyRecursively when finished. Returns 0 if no error occurred. */
int sceneInitializeGeometry(sceneNode *node, GLuint unifDim, GLuint texNum, 
		meshGLMesh *mesh, sceneNode *firstChild, sceneNode *nextSibling){
	node->nodeType = sceneGEOMETRY;
	if (sceneInitializeDefaults(node, unifDim, firstChild, nextSibling) != 0)
		return 1;
	node->unif = (GLdouble *)malloc(unifDim * sizeof(GLdouble) + 
        texNum * sizeof(texTexture *));
    node->tex = (texTexture **)&(node->unif[unifDim]);
	node->texNum = texNum;
	node->meshGL = mesh;
	return 0;

}

/* Initializes a Camera node. this should be the root node of a scene.
The user must remember to call sceneDestroy or 
sceneDestroyRecursively when finished. Returns 0 if no error occurred. */
int sceneInitializeCamera(sceneNode *node, GLuint unifDim, GLdouble rotation[3][3],
		GLdouble translation[3], sceneNode *firstChild, sceneNode *nextSibling){
	node->nodeType = sceneCAMERA;
	if (sceneInitializeDefaults(node, unifDim, firstChild, nextSibling) != 0)
		return 1;
    if(translation!=NULL){
    	vecCopy(3, node->translation, translation);
    }
    if(rotation!=NULL){
    	vecCopy(9, (GLdouble *)rotation, (GLdouble *)(node->rotation));
    }
	return 0;
}

/* Initializes a light node. The user must remember to call sceneDestroy or 
sceneDestroyRecursively when finished. Returns 0 if no error occurred. */
int sceneInitializeLight(sceneNode *node, GLuint unifDim, lightLight *light, sceneNode *firstChild, sceneNode *nextSibling){
	node->nodeType = sceneLIGHT;
	if (sceneInitializeDefaults(node, unifDim, firstChild, nextSibling) != 0)
		return 1;
	node->light = light;
	node->hasShadows  = sceneCASTSHADOWS;
	return 0;
}

int sceneInitializeLOD(sceneNode *node, GLuint unifDim, GLint rangeDim, 
		GLint *ranges, sceneNode *firstChildMeshes[], sceneNode *nextSibling){
	node->nodeType = sceneLOD;
	if (sceneInitializeDefaults(node, unifDim, NULL, nextSibling) != 0)
		return 1;
	node->rangeDim = rangeDim;
	node->ranges = (GLint *)malloc(rangeDim * sizeof(GLint));
	node->firstChildMeshes = (sceneNode **)malloc(rangeDim * sizeof(sceneNode *));
	return 0;
}

int sceneInitializeSwitch(sceneNode *node, GLuint unifDim, GLuint numSwitches, 
		sceneNode *firstChildNodes[], sceneNode *nextSibling){
	node->nodeType = sceneSWITCH;
	if (sceneInitializeDefaults(node, unifDim, NULL, nextSibling) != 0)
		return 1;
	node->numSwitches = numSwitches;
	node->curSwitch = 0;
	node->firstChildNodes = (sceneNode **)malloc(numSwitches * sizeof(sceneNode *));
	return 0;
}

/* Deallocates the resources backing this scene node. Does not destroy the 
resources backing the mesh, etc. */
void sceneDestroy(sceneNode *node) {
	if (node->unif != NULL)
		free(node->unif);
	node->unif = NULL;
	
	if (node->firstChildMeshes != NULL)
		free(node->firstChildMeshes);
	node->firstChildMeshes = NULL;
	
	if (node->firstChildNodes != NULL)
		free(node->firstChildNodes);
	node->firstChildNodes = NULL;
}

/* Calls sceneDestroy recursively on the node's descendants and younger 
siblings, and then on the node itself. */
void sceneDestroyRecursively(sceneNode *node) {
	if (node->firstChild != NULL)
		sceneDestroyRecursively(node->firstChild);
	if (node->nextSibling != NULL)
		sceneDestroyRecursively(node->nextSibling);
	if (node->firstChildMeshes != NULL) {
		for (int i = 0; i < node->rangeDim; i += 1) {
			if (node->firstChildMeshes[i] != NULL)
				sceneDestroyRecursively(node->firstChildMeshes[i]);
		}
	}
	
	if (node->firstChildNodes != NULL) {
		for (int i = 0; i < node->numSwitches; i += 1) {
			if (node->firstChildNodes[i] != NULL)
				sceneDestroyRecursively(node->firstChildNodes[i]);
		}
	}
	sceneDestroy(node);
}

/*** Accessors ***/
/* Sets what type of Node this is */
void sceneSetType(sceneNode *node, GLuint type) {
	node->nodeType = type;
}
/* Copies the unifDim-dimensional vector from unif into the node. */
void sceneSetUniform(sceneNode *node, GLdouble unif[]) {
	vecCopy(node->unifDim, unif, node->unif);
}

/* Sets one uniform in the node, based on its index in the unif array. */
void sceneSetOneUniform(sceneNode *node, int index, GLdouble unif) {
	node->unif[index] = unif;
}

/* Sets how many textures a Geometry node has */
void sceneSetTexNum(sceneNode *node, GLuint num) {
	node->texNum = num;
}

/* Sets All textures of a Geometry node */
void sceneSetTexture(sceneNode *node, texTexture *textures[]) {
	for(int i=0;i<node->texNum;i++){
		node->tex[i]=textures[i];
	}
}

/* Sets one uniform in the node, based on its index in the unif array. */
void sceneSetOneTexture(sceneNode *node, int index, texTexture *texture) {
	node->tex[index] = texture;
}

/* Sets the node's rotation. */
void sceneSetRotation(sceneNode *node, GLdouble rot[3][3]) {
	vecCopy(9, (GLdouble *)rot, (GLdouble *)(node->rotation));
}

/* Sets the node's translation. */
void sceneSetTranslation(sceneNode *node, GLdouble transl[3]) {
	vecCopy(3, transl, node->translation);
}

/* Sets the node's camera. */
void sceneSetCamera(sceneNode *node, camCamera *cam) {
	node->cam = cam;
}

/* Sets up the OpenGL locations for light Nodes */
void sceneSetLightLocations(sceneNode *node, GLint lightPos, GLint lightColor, 
	GLint lightAtten, GLint lightDir, GLint lightCos){
	if (lightPos != -1)
		node->lightPosition = lightPos;

	if (lightColor != -1)
		node->lightColor = lightColor;

	if (lightAtten != -1)
		node->lightAtten = lightAtten;

	if (lightDir != -1)
		node->lightDir = lightDir;

	if (lightCos != -1)
		node->lightCos = lightCos;

}

/* Sets the shadowMap of a light Node */
void sceneSetHasShadow(sceneNode *node, GLuint hasShadows) {
	node->hasShadows = hasShadows;
}

/* Sets up the OpenGL locations for light Node shadows */
void sceneSetShadowLocations(sceneNode *node, GLint viewingSdw, GLint textureUnit, GLint textureSdw) {
	if (viewingSdw != -1) 
		node->viewingSdw = viewingSdw;
	
	if (textureUnit != -1)
		node->textureUnit = textureUnit;
	
	if (textureSdw != -1) 
		node->textureSdw = textureSdw;
}

/* Sets the shadowMap of a light Node */
void sceneSetShadowMap(sceneNode *node, shadowMap *sdwMap) {
	node->sdwMap = sdwMap;
}

/* Sets the scene's mesh. */
void sceneSetMesh(sceneNode *node, meshGLMesh *mesh) {
	node->meshGL = mesh;
}

/* Sets the node's first child. */
void sceneSetFirstChild(sceneNode *node, sceneNode *child) {
	node->firstChild = child;
}

/* Sets the node's next sibling. */
void sceneSetNextSibling(sceneNode *node, sceneNode *sibling) {
	node->nextSibling = sibling;
}

/* Adds a sibling to the given node. The sibling shows up as the youngest of 
its siblings. */
void sceneAddSibling(sceneNode *node, sceneNode *sibling) {
	if (node->nextSibling == NULL)
		node->nextSibling = sibling;
	else
		sceneAddSibling(node->nextSibling, sibling);
}

/* Adds a child to the given node. The child shows up as the youngest of its 
siblings. */
void sceneAddChild(sceneNode *node, sceneNode *child) {
	if (node->firstChild == NULL)
		node->firstChild = child;
	else
		sceneAddSibling(node->firstChild, child);
}

/* Removes a sibling from the given node. Equality of nodes is assessed as 
equality of pointers. If the sibling is not present, then has no effect (fails 
silently). */
void sceneRemoveSibling(sceneNode *node, sceneNode *sibling) {
	if (node->nextSibling == NULL)
		return;
	else if (node->nextSibling == sibling)
		node->nextSibling = sibling->nextSibling;
	else
		sceneRemoveSibling(node->nextSibling, sibling);
}

/* Removes a child from the given node. Equality of nodes is assessed as 
equality of pointers. If the sibling is not present, then has no effect (fails 
silently). */
void sceneRemoveChild(sceneNode *node, sceneNode *child) {
	if (node->firstChild == NULL)
		return;
	else if (node->firstChild == child)
		node->firstChild = child->nextSibling;
	else
		sceneRemoveSibling(node->firstChild, child);
}

void sceneSetChildArray(sceneNode *node, sceneNode *firstChildMeshes[]){
	int range;
	
	if (node->nodeType == sceneLOD) {
		range = node->rangeDim;
	} else if (node->nodeType == sceneSWITCH) {
		range = node->numSwitches;
	}
	for(int i = 0; i < range; i += 1){
		node->firstChildMeshes[i] = firstChildMeshes[i];
	}	
}

void sceneSetOneChild(sceneNode *node, int index, sceneNode *child) {
	node->firstChildMeshes[index] = child;
}

void sceneSetChildArraySwitch(sceneNode *node, sceneNode *firstChildNodes[]){
	for(int i = 0; i < node->numSwitches; i += 1){
		node->firstChildNodes[i] = firstChildNodes[i];
	}	
}

void sceneSetRanges(sceneNode *node, GLint *ranges){
	for (int i=0;i<node->rangeDim;i++){
		node->ranges[i] = ranges[i];
	}
}

void sceneSetSwitch(sceneNode *node, GLuint switchIndex) {
	node->curSwitch = switchIndex;
} 

void sceneCycleSwitch(sceneNode *node) {
	node->curSwitch += 1;
	if (node->curSwitch == node->numSwitches) node->curSwitch = 0;
	printf("%d\n", node->curSwitch);
	fflush(stdout);
}

void sceneRenderTextures(sceneNode *node, GLint textureLocs[]){
	for(int k=0;k<node->texNum; k++){
			texRender(node->tex[k], units[k], k, textureLocs[k]);
	}
}

void sceneUnrenderTextures(sceneNode *node, GLint textureLocs[]){
	for(int k=0;k<node->texNum; k++){
			texUnrender(node->tex[k], units[k]);
	}
}

void sceneSetUniforms(sceneNode *node, GLuint unifNum, GLuint unifDims[], GLint unifLocs[]){
	int unifCount=0;
	for(int i=0; i<unifNum; i++){
		if(unifDims[i]==1){
			GLfloat unif[1];
			vecOpenGL(1, &node->unif[unifCount], unif);
			glUniform1fv(unifLocs[i], 1, unif);
		} else if(unifDims[i]==2){
			GLfloat unif2[2];
			vecOpenGL(2, &node->unif[unifCount], unif2);
			glUniform2fv(unifLocs[i], 1, unif2);
		} else if(unifDims[i]==3){
			GLfloat unif3[3];
			vecOpenGL(3, &node->unif[unifCount], unif3);
			glUniform3fv(unifLocs[i], 1, unif3);
		} else if(unifDims[i]==4){
			GLfloat unif4[4];
			vecOpenGL(4, &node->unif[unifCount], unif4);
			glUniform4fv(unifLocs[i], 1, unif4);
		}
		unifCount+=unifDims[i];
	}
}

void sceneRenderCamera(sceneNode *node, GLint modelingLoc, GLint projLoc,
		GLuint unifNum, GLuint unifDims[], GLint unifLocs[], 
		GLdouble m[4][4], GLdouble projection[4][4], GLdouble invCam[4][4], 
		GLint camPosLoc){
	GLfloat viewing[4][4], vec[3];
	GLdouble camInv[4][4], proj[4][4], projCamInv[4][4];
	mat44InverseIsometry(node->cam->rotation, node->cam->translation, camInv);
	if(node->cam->projectionType==camORTHOGRAPHIC){
		mat44Orthographic(node->cam->projection[camPROJL], node->cam->projection[camPROJR],
			node->cam->projection[camPROJB], node->cam->projection[camPROJT], 
			node->cam->projection[camPROJF], node->cam->projection[camPROJN], proj);
	} else {
		mat44Perspective(node->cam->projection[camPROJL], node->cam->projection[camPROJR],
	 		node->cam->projection[camPROJB], node->cam->projection[camPROJT], 
	 		node->cam->projection[camPROJF], node->cam->projection[camPROJN], proj);
	}
	mat444Multiply(proj, camInv, projCamInv);
	mat44Copy(projCamInv, projection);
	mat44Disect(projCamInv, node->rotation, node->translation);
	mat44Identity(m);
	mat44Copy(camInv, invCam);
	vecOpenGL(3, node->cam->translation, vec);
	glUniform3fv(camPosLoc, 1, vec);
}

void sceneRenderLight(sceneNode *node, GLdouble parent[4][4], 
		GLdouble parentProj[4][4], GLdouble parentCam[4][4], GLint modelingLoc,
		GLint projLoc, GLuint unifNum, GLuint unifDims[], 
		GLint unifLocs[], GLdouble m[4][4], GLdouble projection[4][4], 
		GLdouble eyeView[4][4]){
	mat44Copy(parent, m);
	mat44Copy(parentProj, projection);
	mat44Copy(parentCam, eyeView);
	lightRender(node->light, node->lightPosition, node->lightColor, 
		node->lightAtten, node->lightDir, node->lightCos);
	shadowRender(node->sdwMap, node->viewingSdw, units[node->textureUnit],
			 	node->textureUnit, node->textureSdw);
}

void sceneRenderTransformation(sceneNode *node, GLdouble parent[4][4], 
	GLdouble parentProj[4][4], GLdouble parentCam[4][4], GLint modelingLoc, GLint projLoc, 
	GLuint unifNum, GLuint unifDims[], GLint unifLocs[], GLdouble m[4][4],
	GLdouble projection[4][4], GLdouble eyeView[4][4]) {
	GLfloat model[4][4], proj[4][4];
	GLdouble mHold[4][4], projectionHold[4][4];

	mat44Isometry(node->rotation, node->translation, mHold);
	mat444Multiply(parent, mHold, m);
	mat444Multiply(parentProj, mHold, projection);
	mat44Copy(parentCam, eyeView);
}

void sceneRenderGeometry(sceneNode *node, GLdouble parent[4][4], 
	GLdouble parentProj[4][4], GLdouble parentCam[4][4], GLint modelingLoc,
	GLint projLoc, GLuint unifNum, GLuint unifDims[], GLint unifLocs[], 
	GLuint index, GLint textureLocs[], GLdouble m[4][4], 
	GLdouble projection[4][4], GLdouble eyeView[4][4], GLint distFromCam){
	GLfloat model[4][4], proj[4][4], cam[4][4];
	GLdouble mHold[4][4], projectionHold[4][4];
	

	mat44Disect(parent, node->rotation, node->translation);
	mat44OpenGL(parent, model);
	mat44Copy(parent, m);

	mat44OpenGL(parentProj, proj);
	mat44Copy(parentProj, projection);

	mat44OpenGL(parentCam, cam);
	mat44Copy(parentCam, eyeView);
	
	glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, (GLfloat *)model);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, (GLfloat *)proj);
	glUniformMatrix4fv(distFromCam, 1, GL_FALSE, (GLfloat *)cam);

	sceneRenderTextures(node, textureLocs);
	meshGLRender(node->meshGL, index);
	sceneUnrenderTextures(node, textureLocs);
}

void sceneRenderSwitch(sceneNode *node, GLdouble parent[4][4], 
		GLdouble parentProj[4][4], GLdouble parentCam[4][4], GLint modelingLoc,
		GLint projLoc, GLuint unifNum, GLuint unifDims[], GLint unifLocs[], 
		GLdouble m[4][4], GLdouble projection[4][4], GLdouble eyeView[4][4]) {
	mat44Disect(parent, node->rotation, node->translation);
	mat44Copy(parent, m);
	mat44Copy(parentProj, projection);
	mat44Copy(parentCam, eyeView);
	
	sceneSetFirstChild(node, node->firstChildNodes[node->curSwitch]);
}

void sceneRenderLOD(sceneNode *node, GLdouble parent[4][4], 
		GLdouble parentProj[4][4], GLdouble parentCam[4][4], GLint modelingLoc,
		GLint projLoc, GLuint unifNum, GLuint unifDims[], GLint unifLocs[], 
		GLdouble m[4][4], GLdouble projection[4][4], GLdouble eyeView[4][4]){
	GLfloat model[4][4], proj[4][4];
	GLdouble mHold[4][4], projectionHold[4][4];
	mat44Disect(parent, node->rotation, node->translation);
	mat44Copy(parent, m);
	mat44Copy(parentProj, projection);
	mat44Copy(parentCam, eyeView);

	for(int i=0;i<node->rangeDim;i++){
		if(-parentCam[2][3]<=node->ranges[i]){
			sceneSetFirstChild(node, node->firstChildMeshes[i]);
			return;
		}
	}
	if(-parentCam[2][3]>node->ranges[node->rangeDim-1]){
		sceneSetFirstChild(node, node->firstChildMeshes[node->rangeDim-1]);
	}
}

/* Renders the node, its younger siblings, and their descendants. parent is the 
modeling matrix at the parent of the node. If the node has no parent, then this 
matrix is the 4x4 identity matrix. Loads the modeling transformation into 
modelingLoc. The attribute information exists to be passed to meshGLRender. The 
uniform information is analogous, but sceneRender loads it, not meshGLRender. */
void sceneRender(sceneNode *node, GLdouble parent[4][4], GLdouble parentProj[4][4], 
		GLdouble parentCam[4][4], GLint modelingLoc, GLint projLoc, GLuint unifNum,
		GLuint unifDims[], GLint unifLocs[], GLuint index, GLint textureLocs[],
		GLint camPosLoc, GLint distFromCam) {
	
	GLdouble m[4][4], projection[4][4], invCam[4][4];
	/* Set the other uniforms.*/
	sceneSetUniforms(node, unifNum, unifDims, unifLocs);
	/* Updated */
	
	
	if (node->nodeType==sceneCAMERA){
		sceneRenderCamera(node, modelingLoc, projLoc, unifNum, 
			unifDims, unifLocs, m, projection, invCam, camPosLoc);
	} else if (node->nodeType==sceneLIGHT){
		sceneRenderLight(node, parent, parentProj, parentCam, modelingLoc, 
			projLoc, unifNum, unifDims, unifLocs, m, projection, invCam);
	} else if (node->nodeType==sceneTRANSFORMATION){
		sceneRenderTransformation(node, parent, parentProj, parentCam, 
			modelingLoc, projLoc, unifNum, unifDims, unifLocs, m, projection, 
			invCam);
	} else if (node->nodeType==sceneLOD){
		sceneRenderLOD(node, parent, parentProj, parentCam, modelingLoc, 
			projLoc, unifNum, unifDims, unifLocs, m, projection, invCam);
	}else if (node->nodeType==sceneSWITCH){
		sceneRenderSwitch(node, parent, parentProj, parentCam, modelingLoc, 
			projLoc, unifNum, unifDims, unifLocs, m, projection, invCam);
	} else {
		sceneRenderGeometry(node, parent, parentProj, parentCam, modelingLoc,
		projLoc, unifNum, unifDims, unifLocs, index, textureLocs, m, projection, 
		invCam, distFromCam);
	}
	
	/* Render the mesh, the children, and the younger siblings. */
	if(node->firstChild != NULL){
		sceneRender(node->firstChild, m, projection, invCam, modelingLoc, 
			projLoc, unifNum, unifDims, unifLocs, index, textureLocs, 
			camPosLoc, distFromCam);
	}
	if(node->nextSibling != NULL){
		sceneRender(node->nextSibling, parent, parentProj, parentCam, 
			modelingLoc, projLoc, unifNum, unifDims, unifLocs, index, 
			textureLocs, camPosLoc, distFromCam);
	}
}


