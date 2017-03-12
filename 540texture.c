


/* These pre-compiler directives need to be in this order. */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_FAILURE_USERMSG

/* Feel free to read from this struct's members, but don't write to them except 
through the accessor functions. */
typedef struct texTexture texTexture;
struct texTexture {
	GLuint width, height, texelDim;
	GLuint openGL;
};

/* minification and magnification should be GL_NEAREST or GL_LINEAR. leftRight 
and bottomTop should be one of GL_CLAMP, GL_REPEAT, etc. */
void texSetFilteringBorder(texTexture *tex, GLint minification, 
		GLint magnification, GLint leftRight, GLint bottomTop) {
	glBindTexture(GL_TEXTURE_2D, tex->openGL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minification);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magnification);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, leftRight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bottomTop);
}

/* Loads the given image file into an OpenGL texture. The width and height of 
the image must be powers of 2. For other parameter meanings, see 
texSetFilteringBorder. Returns 0 on success, non-zero on failure. On success, 
the user must call texDestroy when finished with the texture. */
int texInitializeFile(texTexture *tex, char *path, GLint minification, 
		GLint magnification, GLint leftRight, GLint bottomTop) {
	/* Use STB Image to load the texture data from the file. */
	int width, height, texelDim;
	unsigned char *rawData;
	rawData = stbi_load(path, &width, &height, &texelDim, 0);
	if (rawData == NULL) {
		fprintf(stderr, "texInitializeFile: failed to load %s\n", path);
		fprintf(stderr, "with STB Image reason: %s.\n", stbi_failure_reason());
		return 1;
	}
	
	/* Load the data into OpenGL. */
	glGenTextures(1, &(tex->openGL));
	texSetFilteringBorder(tex, minification, magnification, leftRight, 
		bottomTop);

	/* Right now we support only 3-channel images. Supporting other texelDims 
	is not hard --- maybe just changing certain parameters to glTexImage2D 
	below, and then using the data correctly in the shader. */
	if(texelDim==3){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, 
		GL_UNSIGNED_BYTE, rawData);
	} else if (texelDim==4){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, 
		GL_UNSIGNED_BYTE, rawData);
	} else {
		fprintf(stderr, "texInitializeFile: %d != 3 channels.\n", texelDim);
		return 2;
	}
	
	stbi_image_free(rawData);
	if (glGetError() != GL_NO_ERROR) {
		fprintf(stderr, "texInitializeFile: OpenGL error.\n");
		glDeleteTextures(1, &(tex->openGL));
		return 3;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	tex->width = width;
	tex->height = height;
	tex->texelDim = texelDim;
	return 0;
} 

int load_cube_map_side(
  texTexture *tex, GLenum side_target, const char* file_name) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, tex->openGL);

  int x, y, n;
  int force_channels = 4;
  unsigned char*  image_data = stbi_load(
    file_name, &x, &y, &n, force_channels);
  if (!image_data) {
    fprintf(stderr, "ERROR: could not load %s\n", file_name);
    return 1;
  }
  // non-power-of-2 dimensions check
  if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
    fprintf(stderr,
    	"WARNING: image %s is not power-of-2 dimensions\n",
    	file_name);
  }
  
  // copy image data into 'target' side of cube map
  glTexImage2D(
    side_target,
    0,
    GL_RGBA,
    x,
    y,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    image_data);
  free(image_data);
  return 0;
}
void create_cube_map(
  const char* front,
  const char* back,
  const char* top,
  const char* bottom,
  const char* left,
  const char* right,
  texTexture* tex) {
  // generate a cube-map texture to hold all the sides
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &(tex->openGL));
  // load each image and copy into a side of the cube-map texture
  load_cube_map_side(tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front);
  load_cube_map_side(tex, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back);
  load_cube_map_side(tex, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top);
  load_cube_map_side(tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom);
  load_cube_map_side(tex, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left);
  load_cube_map_side(tex, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right);
  // format cube map texture
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

/* Deallocates the resources backing the texture. */
void texDestroy(texTexture *tex) {
	glDeleteTextures(1, &(tex->openGL));
}

/* At the start of rendering a frame, the renderer calls this function, to hook 
the texture into a certain texture unit. textureUnit is something like 
GL_TEXTURE0. textureUnitIndex would then be 0. */
void texRender(texTexture *tex, GLenum textureUnit, GLint textureUnitIndex, 
		GLint textureLoc) {
	glActiveTexture(textureUnit);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex->openGL);
    glUniform1i(textureLoc, textureUnitIndex);
}

/* At the end of rendering a frame, the renderer calls this function, to unhook 
the texture from a certain texture unit. textureUnit is something like 
GL_TEXTURE0. */
void texUnrender(texTexture *tex, GLenum textureUnit) {
	glActiveTexture(textureUnit);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}


