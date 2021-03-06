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
#define sceneSTATE			6

#define sceneSTATETEX		0
#define sceneSTATESHINE		1
#define sceneSTATEON		2

#define statusOFF			0
#define statusON			1

#define sceneCASTSHADOWS 	0
#define sceneNOSHADOWS 		1

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
	sceneNode *rootNode;
	lightLight *light;
	GLint lightPosition, lightColor, lightAtten, lightDir, lightCos;
	shadowMap *sdwMap;
	shadowProgram *sdwProg;
	GLint viewingSdw, textureUnit, textureSdw;
	GLdouble near;
	GLdouble far;

	// LOD node
	GLint *ranges, rangeDim;
	sceneNode **firstChildMeshes;
	
	//Switch node
	sceneNode **firstChildNodes;
	GLuint numSwitches;
	GLuint curSwitch;

	//State node
	GLint stateType;
	GLfloat *colors;
	GLfloat *shininess;
	texTexture **states;
	GLuint stateNum;
	GLint curState;
	GLint status;
};

/* Initialized a few defaults standard across all nodes */
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
int sceneInitializeLight(sceneNode *node, GLuint unifDim, lightLight *light, 
		sceneNode *firstChild, sceneNode *nextSibling, sceneNode *rootNode,
		shadowProgram *sdwProg, GLdouble far, GLdouble near){
	node->nodeType = sceneLIGHT;
	if (sceneInitializeDefaults(node, unifDim, firstChild, nextSibling) != 0)
		return 1;
	node->light = light;
	node->hasShadows  = sceneCASTSHADOWS;
	node->far = far;
	node->near = near;
	node->rootNode = rootNode;
	node->sdwProg = sdwProg;
	return 0;
}

/* Initializes LOD nodes. ranges are the distances from the camera at with the node should switch */
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

/* Initializes a switch node. This type of Node chooses which child to render based off of its current
switch. The default switch is 0 */
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

/* Initializes a state node. This type of Node chooses what state information to send to its 
child based off of its current state.*/
int sceneInitializeState(sceneNode *node, GLuint unifDim, GLint stateType, 
		GLuint stateNum, GLuint startState, sceneNode *firstChild, 
		sceneNode *nextSibling){
	node->nodeType = sceneSTATE;
	node->stateType = stateType;
	if (sceneInitializeDefaults(node, unifDim, firstChild, nextSibling) != 0)
		return 1;
	node->stateNum = stateNum;
	node->curState = startState;
	if (node->stateType == sceneSTATETEX){
		node->states = (texTexture **)malloc(stateNum * sizeof(texTexture *));
	} else if (node->stateType == sceneSTATESHINE){
		node->shininess = (GLfloat *)malloc(stateNum * sizeof(GLfloat));
	} 
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

/* Sets shadow map's near plane */
void sceneSetNear(sceneNode *node, GLdouble near){
	node->near = near;
}

/* Sets shadow map's far plane */
void sceneSetFar(sceneNode *node, GLdouble far){
	node->far = far;
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

/* Sets the shadowProgram of a light Node */
void sceneSetShadowProgram(sceneNode *node, shadowProgram *sdwProg) {
	node->sdwProg = sdwProg;
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

/* Sets the node that shadowing starts at. */
void sceneSetRoot(sceneNode *node, sceneNode *rootNode){
	node->rootNode = rootNode;
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

/* Sets all of the children of a LOD node. The array must be the same size as the Node's ranngeDim */
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

/* Sets one LOD child at index  */
void sceneSetOneChild(sceneNode *node, int index, sceneNode *child) {
	node->firstChildMeshes[index] = child;
}

/* Sets all children of a switch node. the array must be of length numSwitches  */
void sceneSetChildArraySwitch(sceneNode *node, sceneNode *firstChildNodes[]){
	for(int i = 0; i < node->numSwitches; i += 1){
		node->firstChildNodes[i] = firstChildNodes[i];
	}	
}

/* sets all of the ranges of a LOD node. ranges must equal rangeDim */
void sceneSetRanges(sceneNode *node, GLint *ranges){
	for (int i=0;i<node->rangeDim;i++){
		node->ranges[i] = ranges[i];
	}
}

/* Sets the current switch of a switch node based off of the given index */
void sceneSetSwitch(sceneNode *node, GLuint switchIndex) {
	node->curSwitch = switchIndex;
} 

/* Cycles to the next child. If it is the last child, returns to the beginning */
void sceneCycleSwitch(sceneNode *node) {
	node->curSwitch += 1;
	if (node->curSwitch == node->numSwitches) node->curSwitch = 0;
}
/* Sets the texture states for a state node */
void sceneSetTextureStates(sceneNode *node, texTexture *states[]){
	for (int i=0;i<node->stateNum;i++){
		node->states[i] = states[i];
	}
}
/* updates one texture state for a state node */
void sceneSetOneTextureState(sceneNode *node, GLint index, texTexture *state){
	node->states[index] = state;
}

/* Sets the shininess states for a state node */
void sceneSetShininessStates(sceneNode *node, GLfloat *shinies){
	for (int i=0;i<node->stateNum*3.0;i++){
		node->shininess[i] = shinies[i];
	}
}
/* Updates one shininess state for a state node */
void sceneSetOneShininessState(sceneNode *node, GLint index, GLfloat shinies[3]){
	node->shininess[index] = shinies[0];
	node->shininess[index+1] = shinies[1];
	node->shininess[index+2] = shinies[2];
}
/* Turns state node on/off, which the state node will send to its children, telling them if they should render */
void sceneSetStatus(sceneNode *node, GLint status){
	node->status = status;
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

/* Chooses which opengl function to use to set usniforms based off of the number of uniforms */
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

/* Render a camera Node */
void sceneRenderCamera(sceneNode *node, GLdouble m[4][4], 
		GLdouble projCamInv[4][4], GLdouble camInv[4][4], GLint camPosLoc){
	GLfloat vec[3];
	camRenderScene(node->cam, camInv, projCamInv);
	mat44Disect(projCamInv, node->rotation, node->translation);
	vecOpenGL(3, node->cam->translation, vec);
	glUniform3fv(camPosLoc, 1, vec);
}

/* Render a light node. Renders shadows if the light node has shadows */
void sceneRenderLight(sceneNode *node){
	lightRender(node->light, node->lightPosition, node->lightColor, 
		node->lightAtten, node->lightDir, node->lightCos);
	if (node->hasShadows == sceneCASTSHADOWS) {
		shadowRender(node->sdwMap, node->viewingSdw, units[node->textureUnit],
				 	node->textureUnit, node->textureSdw);
	}
}

/* Render a lod node */
void sceneRenderLOD(sceneNode *node, GLdouble parentCam[4][4]){
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

/* Choose which child a switch node should render */
void sceneRenderSwitch(sceneNode *node) {
	sceneSetFirstChild(node, node->firstChildNodes[node->curSwitch]);
}

/* Choose which state to pass onto child */
void sceneRenderState(sceneNode *node) {
	if(node->stateType == sceneSTATETEX){
		node->firstChild->tex[0] = node->states[node->curState];
	} else if(node->stateType == sceneSTATESHINE){
		node->unif[0] = node->shininess[node->curState];
	} 
}

/* Render a transformation node, passing on its transformation to its child */
void sceneRenderTransformation(sceneNode *node, GLdouble parent[4][4], 
		GLdouble parentProj[4][4], GLint projLoc, GLdouble m[4][4], 
		GLdouble projection[4][4]) {
	GLfloat model[4][4], proj[4][4];
	GLdouble mHold[4][4], projectionHold[4][4];

	mat44Isometry(node->rotation, node->translation, mHold);
	mat444Multiply(parent, mHold, m);
	mat444Multiply(parentProj, mHold, projection);
}

/* Render a Geometry Node */
void sceneRenderGeometry(sceneNode *node, GLdouble parent[4][4], 
		GLdouble parentProj[4][4], GLdouble parentCam[4][4], GLint modelingLoc,
		GLint projLoc, GLuint index, GLint textureLocs[], GLint camInvLoc){
	GLfloat model[4][4], proj[4][4], cam[4][4];
	
	mat44OpenGL(parent, model);
	mat44OpenGL(parentProj, proj);
	mat44OpenGL(parentCam, cam);
	
	glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, (GLfloat *)model);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, (GLfloat *)proj);
	glUniformMatrix4fv(camInvLoc, 1, GL_FALSE, (GLfloat *)cam);

	sceneRenderTextures(node, textureLocs);
	meshGLRender(node->meshGL, index);
	sceneUnrenderTextures(node, textureLocs);
}


/* Renders the node, its younger siblings, and their descendants. parent is the 
modeling matrix at the parent of the node. If the node has no parent, then this 
matrix is the 4x4 identity matrix. Loads the modeling transformation into 
modelingLoc. The attribute information exists to be passed to meshGLRender. The 
uniform information is analogous, but sceneRender loads it, not meshGLRender. */
void sceneRender(sceneNode *node, GLdouble parent[4][4], GLdouble parentProj[4][4], 
		GLdouble parentCam[4][4], GLint modelingLoc, GLint projLoc, GLuint unifNum,
		GLuint unifDims[], GLint unifLocs[], GLuint index, GLint textureLocs[],
		GLint camPosLoc, GLint camInvLoc) {
	GLdouble m[4][4], projection[4][4], invCam[4][4];
	mat44Copy(parent, m);
	mat44Copy(parentProj, projection);
	mat44Copy(parentCam, invCam);

	if (node->nodeType==sceneCAMERA){
		sceneRenderCamera(node, m, projection, invCam, camPosLoc);

	} else if (node->nodeType==sceneLIGHT){
		sceneRenderLight(node);

	} else if (node->nodeType==sceneTRANSFORMATION){
		sceneRenderTransformation(node, parent, parentProj, projLoc, m, 
			projection);

	} else if (node->nodeType==sceneLOD){
		sceneRenderLOD(node, parentCam);

	}else if (node->nodeType==sceneSWITCH){
		sceneRenderSwitch(node);

	} else if (node->nodeType==sceneSTATE){
		sceneRenderState(node);
	} else {
		/* Set the other uniforms.*/
		sceneSetUniforms(node, unifNum, unifDims, unifLocs);
		sceneRenderGeometry(node, parent, parentProj, parentCam, modelingLoc,
		projLoc, index, textureLocs, camInvLoc);
	}

	/* Render the mesh, the children, and the younger siblings. */
	if(node->nextSibling != NULL){
		sceneRender(node->nextSibling, parent, parentProj, parentCam, 
			modelingLoc, projLoc, unifNum, unifDims, unifLocs, index, 
			textureLocs, camPosLoc, camInvLoc);
	}
	if(node->nodeType!=sceneSTATE || node->stateType != sceneSTATEON || node->status != statusOFF){
		if(node->firstChild != NULL){
			sceneRender(node->firstChild, m, projection, invCam, modelingLoc, 
				projLoc, unifNum, unifDims, unifLocs, index, textureLocs, 
				camPosLoc, camInvLoc);
		}
	}
	
}

void scenePreRenderLight(sceneNode *node){
	GLint sdwTextureLocs[1] = {-1};
	GLdouble identity[4][4];
	mat44Identity(identity);

	shadowMapRender(node->sdwMap, node->sdwProg, node->light, node->far, node->near);
	sceneRender(node->rootNode, identity, identity, identity, node->sdwProg->modelingLoc, 
		node->sdwProg->modelingLoc, 0, NULL, NULL, 1, sdwTextureLocs, -1, -1);
	shadowMapUnrender();
}

