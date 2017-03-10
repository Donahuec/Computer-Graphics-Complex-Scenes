/* On macOS, compile with...
    clang 580mainSpot.c /usr/local/gl3w/src/gl3w.o -lglfw -framework OpenGL -framework CoreFoundation
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include "500shader.c"
#include "530vector.c"
#include "580mesh.c"
#include "520matrix.c"
#include "520camera.c"
#include "560light.c"
#include "540texture.c"
#include "580scene.c"

GLdouble alpha = 0.0;
GLuint program;
GLint attrLocs[3];
GLint positionLoc, colorLoc;
GLint viewingLoc, modelingLoc;
GLint camLoc, lightLoc, colLoc, attLoc, dirLoc, cosLoc;
GLint texLocs[1];
GLint unifLocs[1];
texTexture *tex[3];
texTexture texture;
camCamera cam;
lightLight light;
GLdouble transl[] = {5.0, 30.0, -3.0};
GLdouble rgb[] = {1.0, 1.0, 1.0};
GLdouble atten[] = {1.0, 0.0, 0.0};
GLuint attrDims[3] = {3, 2, 3};


/* Allocate three meshes and three scene graph nodes. */
meshGLMesh rootMesh, childMesh, siblingMesh;
sceneNode rootNode, childNode, siblingNode;

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
			camAddDistance(&cam, -0.1);
		else if (key == GLFW_KEY_J)
			camAddDistance(&cam, 0.1);
	}
}

/* Returns 0 on success, non-zero on failure. Warning: If initialization fails 
midway through, then does not properly deallocate all resources. But that's 
okay, because the program terminates almost immediately after this function 
returns. */
int initializeScene(void) {
	
	lightSetTranslation(&light, transl);
	lightSetColor(&light, rgb);
	lightSetAttenuation(&light, atten);
	lightSetType(&light, lightOMNI);
	
	
	
	/*Initialize textures*/
	if (texInitializeFile(&texture, "dog.jpeg", GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT) != 0) {
		return 1;
	}
	tex[0] = &texture;
	
	/* Initialize meshes. */
	meshMesh mesh;
	if (meshInitializeCapsule(&mesh, 0.5, 2.0, 16, 32) != 0)
		return 1;
	meshGLInitialize(&rootMesh, &mesh, 3, attrDims, 1);
	meshDestroy(&mesh);
	if (meshInitializeBox(&mesh, -0.5, 0.5, -0.5, 0.5, -0.5, 0.5) != 0)
		return 2;
	meshGLInitialize(&childMesh, &mesh, 3, attrDims, 1);
	meshDestroy(&mesh);
	if (meshInitializeSphere(&mesh, 0.5, 16, 32) != 0)
		return 3;
	meshGLInitialize(&siblingMesh, &mesh, 3, attrDims, 1);
	meshDestroy(&mesh);
	
	meshGLVAOInitialize(&rootMesh, 0, attrLocs);
	meshGLVAOInitialize(&childMesh, 0, attrLocs);
	meshGLVAOInitialize(&siblingMesh, 0, attrLocs);

	
	/* Initialize scene graph nodes. */
	if (sceneInitialize(&siblingNode, 1, 1, &siblingMesh, NULL, NULL) != 0)
		return 4;
	if (sceneInitialize(&childNode, 1, 1, &childMesh, NULL, NULL) != 0)
		return 5;
	if (sceneInitialize(&rootNode, 1, 1, &rootMesh, &childNode, &siblingNode) != 0)
		return 6;
	
	sceneSetTexture(&rootNode, tex);
	sceneSetTexture(&siblingNode, tex);
	sceneSetTexture(&childNode, tex);
	
	/* Customize the uniforms. */
	GLdouble trans[3] = {1.0, 0.0, 0.0};
	sceneSetTranslation(&childNode, trans);
	vecSet(3, trans, 0.0, 1.0, 0.0);
	sceneSetTranslation(&siblingNode, trans);
	GLdouble unif[3] = {1.0, 1.0, 1.0};
	sceneSetUniform(&siblingNode, unif);
	sceneSetUniform(&childNode, unif);
	sceneSetUniform(&rootNode, unif);
	return 0;
}

void destroyScene(void) {
	meshGLDestroy(&siblingMesh);
	meshGLDestroy(&childMesh);
	meshGLDestroy(&rootMesh);
	sceneDestroyRecursively(&rootNode);
	texDestroy(tex[0]);
}

/* Returns 0 on success, non-zero on failure. */
int initializeShaderProgram(void) {
GLchar vertexCode[] = "\
	#version 140\n\
    uniform mat4 viewing;\
    uniform mat4 modeling;\
    in vec3 position;\
    in vec2 texCoords;\
    in vec3 normal;\
    out vec3 fragPos;\
    out vec3 normalDir;\
    out vec2 st;\
    void main() {\
        vec4 worldPos = modeling * vec4(position, 1.0);\
        gl_Position = viewing * worldPos;\
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
    in vec3 fragPos;\
    in vec3 normalDir;\
    in vec2 st;\
	out vec4 fragColor;\
    void main() {\
        vec3 surfCol = vec3(texture(texture0, st));\
        vec3 norDir = normalize(normalDir);\
        vec3 litDir = normalize(lightPos - fragPos);\
        vec3 camDir = normalize(camPos - fragPos);\
        vec3 refDir = 2.0 * dot(litDir, norDir) * norDir - litDir;\
        float d = distance(lightPos, fragPos);\
        float a = lightAtt[0] + lightAtt[1] * d + lightAtt[2] * d * d;\
        float diffInt = dot(norDir, litDir) / a;\
        float specInt = dot(refDir, camDir);\
        if (diffInt <= 0.0 || specInt <= 0.0)\
            specInt = 0.0;\
        float ambInt = 0.1;\
        if (diffInt <= ambInt)\
            diffInt = ambInt;\
        vec3 diffLight = diffInt * lightCol * surfCol;\
        float shininess = 64.0;\
        vec3 specLight = pow(specInt / a, shininess) * lightCol * specular;\
        fragColor = vec4(diffLight + specLight, 1.0);\
	}";
	
	program = makeProgram(vertexCode, fragmentCode);
	if (program != 0) {
		glUseProgram(program);
		viewingLoc = glGetUniformLocation(program, "viewing");
		modelingLoc = glGetUniformLocation(program, "modeling");
		attrLocs[0] = glGetAttribLocation(program, "position");
		attrLocs[1] = glGetAttribLocation(program, "texCoords");
		attrLocs[2] = glGetAttribLocation(program, "normal");
		camLoc = glGetUniformLocation(program, "camPos");
		lightLoc = glGetUniformLocation(program, "lightPos");
		colLoc = glGetUniformLocation(program, "lightCol");
		attLoc = glGetUniformLocation(program, "lightAtt");
		texLocs[0] = glGetUniformLocation(program, "texture0");
		unifLocs[0] = glGetUniformLocation(program, "specular");
	}
	return (program == 0);
}

void render(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	camRender(&cam, viewingLoc);
	GLfloat camPos[3];
	vecOpenGL(3, cam.translation, camPos);
	glUniform3fv(camLoc, 1, camPos);
	
	GLdouble rot[3][3], identity[4][4], axis[3] = {1.0, 1.0, 1.0};
	vecUnit(3, axis, axis);
	alpha += 0.01;
	mat33AngleAxisRotation(alpha, axis, rot);
	sceneSetRotation(&rootNode, rot);
	sceneSetOneUniform(&rootNode, 0, 0.5 + 0.5 * sin(alpha * 7.0));
	mat44Identity(identity);
	
	GLuint unifDims[1] = {3};
	
	lightRender(&light, lightLoc, colLoc, attLoc, dirLoc, cosLoc);
	
	sceneRender(&rootNode, identity, modelingLoc, 1, unifDims, unifLocs, 0, texLocs);
}


int main(void) {
    glfwSetErrorCallback(handleError);
    if (glfwInit() == 0)
        return 1;
    /* Ask GLFW to supply an OpenGL 3.2 context. */
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window;
    window = glfwCreateWindow(512, 512, "GL3W", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        return 2;
    }
    glfwSetWindowSizeCallback(window, handleResize);
    glfwSetKeyCallback(window, handleKey);
    glfwMakeContextCurrent(window);
	
    /* You might think that getting an OpenGL 3.2 context would make OpenGL 3.2 
    available to us. But you'd be wrong. The following call 'loads' a bunch of 
    OpenGL 3.2 functions, so that we can use them. This is why we use GL3W. */
    if (gl3wInit() != 0) {
    	fprintf(stderr, "main: gl3wInit failed.\n");
    	glfwDestroyWindow(window);
    	glfwTerminate();
    	return 3;
    }
	
    fprintf(stderr, "main: OpenGL %s, GLSL %s.\n", 
		glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    glEnable(GL_DEPTH_TEST);
    glDepthRange(1.0, 0.0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    if (initializeShaderProgram() != 0)
    	return 4;
    /* Initialize a whole scene, rather than just one mesh. */
    if (initializeScene() != 0)
    	return 3;
    
    GLdouble target[3] = {0.0, 0.0, 0.0};
	camSetControls(&cam, camPERSPECTIVE, M_PI / 6.0, 10.0, 512.0, 512.0, 10.0, 
		M_PI / 4.0, M_PI / 4.0, target);
    while (glfwWindowShouldClose(window) == 0) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteProgram(program);
    /* Don't forget to destroy the whole scene. */
    destroyScene();
	glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}