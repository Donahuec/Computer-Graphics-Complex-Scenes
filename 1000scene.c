/*
1000scene.c
Written by Josh Davis, adpated by Liv Phillips & Caitlin Donahue for CS311, Winter 2017
Program implementing scene graph
*/

#define sceneTRANSFORMATION 0
#define sceneGEOMETRY 1
#define sceneCAMERA 2

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
};

/* Initializes a sceneNode struct. The translation and rotation are initialized to trivial values. The user must remember to call sceneDestroy or 
sceneDestroyRecursively when finished. Returns 0 if no error occurred. */

/* Updated */
int sceneInitializeTransformation(sceneNode *node, GLuint unifDim, GLdouble rotation[3][3],
		GLdouble translation[3], sceneNode *firstChild, sceneNode *nextSibling) {
	node->nodeType = sceneTRANSFORMATION;
    node->unif = (GLdouble *)malloc(unifDim * sizeof(GLdouble));
    if (node->unif == NULL)
        return 1;
    if(translation==NULL){
    	vecSet(3, node->translation, 0.0, 0.0, 0.0);
    } else{
    	vecCopy(3, node->translation, translation);
    }
    if(rotation==NULL){
    	mat33Identity(node->rotation);
    } else {
    	vecCopy(9, (GLdouble *)rotation, (GLdouble *)(node->rotation));
    }
	node->unifDim = unifDim;
	node->firstChild = firstChild;
	node->nextSibling = nextSibling;
	return 0;
}

int sceneInitializeGeometry(sceneNode *node, GLuint unifDim, GLuint texNum, 
		meshGLMesh *mesh, sceneNode *firstChild, sceneNode *nextSibling){
	node->nodeType = sceneGEOMETRY;
	node->unif = (GLdouble *)malloc(unifDim * sizeof(GLdouble) + 
        texNum * sizeof(texTexture *));
    if (node->unif == NULL)
        return 1;
    node->tex = (texTexture **)&(node->unif[unifDim]);
    mat33Identity(node->rotation);
	vecSet(3, node->translation, 0.0, 0.0, 0.0);
	node->texNum = texNum;
	node->unifDim = unifDim;
	node->meshGL = mesh;
	node->firstChild = firstChild;
	node->nextSibling = nextSibling;
	return 0;

}

int sceneInitializeCamera(sceneNode *node, GLuint unifDim, GLdouble rotation[3][3],
		GLdouble translation[3], sceneNode *firstChild, sceneNode *nextSibling){
	node->nodeType = sceneCAMERA;
    node->unif = (GLdouble *)malloc(unifDim * sizeof(GLdouble));
    if (node->unif == NULL)
        return 1;
    if(translation==NULL){
    	vecSet(3, node->translation, 0.0, 0.0, 0.0);
    } else{
    	vecCopy(3, node->translation, translation);
    }
    if(rotation==NULL){
    	mat33Identity(node->rotation);
    } else {
    	vecCopy(9, (GLdouble *)rotation, (GLdouble *)(node->rotation));
    }
	node->unifDim = unifDim;
	node->firstChild = firstChild;
	node->nextSibling = nextSibling;
	return 0;
}
/* Updated */

/* Deallocates the resources backing this scene node. Does not destroy the 
resources backing the mesh, etc. */
void sceneDestroy(sceneNode *node) {
	if (node->unif != NULL)
		free(node->unif);
	node->unif = NULL;
}

/*** Accessors ***/
/* . */
void sceneSetType(sceneNode *node, GLuint type) {
	node->nodeType = type;
}
/* Copies the unifDim-dimensional vector from unif into the node. */
void sceneSetUniform(sceneNode *node, double unif[]) {
	vecCopy(node->unifDim, unif, node->unif);
}

/* Sets one uniform in the node, based on its index in the unif array. */
void sceneSetOneUniform(sceneNode *node, int index, double unif) {
	node->unif[index] = unif;
}

void sceneSetTexNum(sceneNode *node, GLuint num) {
	node->texNum = num;
}

void sceneSetTexture(sceneNode *node, texTexture *textures[]) {
	for(int i=0;i<node->texNum;i++){
		node->tex[i]=textures[i];
	}
}

/* Sets one uniform in the node, based on its index in the unif array. */
void sceneSetOneTexture(sceneNode *node, int index, texTexture *texture) {
	node->tex[index] = texture;
}

/* Calls sceneDestroy recursively on the node's descendants and younger 
siblings, and then on the node itself. */
void sceneDestroyRecursively(sceneNode *node) {
	if (node->firstChild != NULL)
		sceneDestroyRecursively(node->firstChild);
	if (node->nextSibling != NULL)
		sceneDestroyRecursively(node->nextSibling);
	sceneDestroy(node);
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

void sceneRenderTextures(sceneNode *node, GLint textureLocs[]){
	GLenum units[9]={GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, 
					 GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7, GL_TEXTURE8};
	for(int k=0;k<node->texNum; k++){
			texRender(node->tex[k], units[k], k, textureLocs[k]);
	}
}

void sceneUnrenderTextures(sceneNode *node, GLint textureLocs[]){
	GLenum units[9]={GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3, GL_TEXTURE4, 
					 GL_TEXTURE5, GL_TEXTURE6, GL_TEXTURE7, GL_TEXTURE8};
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
void sceneRenderCamera(sceneNode *node, GLdouble parent[4][4], 
		GLdouble parentCam[4][4], GLint modelingLoc, GLint modelingCamLoc,
		GLuint unifNum, GLuint unifDims[], GLint unifLocs[], double m[4][4], double mC[4][4]){
	GLfloat viewing[4][4];
	double camInv[4][4], proj[4][4], projCamInv[4][4];
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
	mat44Copy(projCamInv, mC);
	mat44Identity(m);
	mat44OpenGL(projCamInv, viewing);
	glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, (GLfloat *)viewing);
}

void sceneRenderTransformation(sceneNode *node, GLdouble parent[4][4], GLdouble parentCam[4][4], 
		GLint modelingLoc, GLint modelingCamLoc, GLuint unifNum, GLuint unifDims[], 
		GLint unifLocs[], double m[4][4], double mC[4][4]) {
	GLfloat model[4][4], modelCam[4][4];
	double mHold[4][4], mCHold[4][4];

	mat44Isometry(node->rotation, node->translation, mHold);
	mat44Copy(mHold, mCHold);
	mat444Multiply(parent, mHold, m);
	mat44OpenGL(m, model);
	mat444Multiply(parentCam, mCHold, mC);
	mat44OpenGL(mC, modelCam);

	glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, (GLfloat *)model);
	glUniformMatrix4fv(modelingCamLoc, 1, GL_FALSE, (GLfloat *)modelCam);
}

void sceneRenderGeometry(sceneNode *node, GLdouble parent[4][4], GLdouble parentCam[4][4], 
		GLint modelingLoc, GLint modelingCamLoc, GLuint unifNum, GLuint unifDims[], 
		GLint unifLocs[], GLuint index, GLint textureLocs[], double m[4][4], double mC[4][4]){
	GLfloat model[4][4], modelCam[4][4];
	double mHold[4][4], mCHold[4][4];
	

	mat44Disect(parent, node->rotation, node->translation);
	mat44OpenGL(parent, model);
	mat44Copy(parent, m);
	mat44OpenGL(parentCam, modelCam);
	mat44Copy(parentCam, mC);
	glUniformMatrix4fv(modelingLoc, 1, GL_FALSE, (GLfloat *)model);
	glUniformMatrix4fv(modelingCamLoc, 1, GL_FALSE, (GLfloat *)modelCam);

	sceneRenderTextures(node, textureLocs);
	meshGLRender(node->meshGL, index);
	sceneUnrenderTextures(node, textureLocs);
}


/* Renders the node, its younger siblings, and their descendants. parent is the 
modeling matrix at the parent of the node. If the node has no parent, then this 
matrix is the 4x4 identity matrix. Loads the modeling transformation into 
modelingLoc. The attribute information exists to be passed to meshGLRender. The 
uniform information is analogous, but sceneRender loads it, not meshGLRender. */
void sceneRender(sceneNode *node, GLdouble parent[4][4], GLdouble parentCam[4][4], 
		GLint modelingLoc, GLint modelingCamLoc, GLuint unifNum, GLuint unifDims[], 
		GLint unifLocs[], GLuint index, GLint textureLocs[]) {
	
	double m[4][4], mC[4][4];
	/* Updated */
	if (node->nodeType==sceneCAMERA){
		sceneRenderCamera(node, parent, parentCam, modelingLoc, modelingCamLoc, unifNum, 
			unifDims, unifLocs, m, mC);
	} else if (node->nodeType==sceneTRANSFORMATION){
		sceneRenderTransformation(node, parent, parentCam, modelingLoc, modelingCamLoc, unifNum, 
			unifDims, unifLocs, m, mC);
	} else {
		sceneRenderGeometry(node, parent, parentCam, modelingLoc, modelingCamLoc, unifNum, 
			unifDims, unifLocs, index, textureLocs, m, mC);
	}
	
	/* Set the other uniforms.*/
	sceneSetUniforms(node, unifNum, unifDims, unifLocs);
	
	/* Render the mesh, the children, and the younger siblings. */
	if(node->firstChild != NULL){
		sceneRender(node->firstChild, m, mC, modelingLoc, modelingCamLoc,
			unifNum, unifDims, unifLocs, index, textureLocs);
	}
	if(node->nextSibling != NULL){
		sceneRender(node->nextSibling, parent, parentCam, modelingLoc, modelingCamLoc,
			unifNum, unifDims, unifLocs, index, textureLocs);
	}
	// if(node->nodeType==sceneGEOMETRY){
	// 	sceneUnrenderTextures(node, textureLocs);
	// }
	/* Updated */
}


