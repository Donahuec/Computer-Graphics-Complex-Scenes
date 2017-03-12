typedef struct skyboxSkybox skyboxSkybox;
struct skyboxSkybox {
	GLuint programBack, skyboxTex;
	GLint backPosition, backView, backLoc, skybox;
	GLuint vao, vbo;
	GLdouble rotation[3][3];
	texTexture texBackground;
};

void skyboxInitializeSkybox(skyboxSkybox *skybox, char *textures[], GLdouble *vertices, 
	GLint projType){
	GLuint attrDims[3] = {3, 2, 3};
	GLdouble *skyboxVertices;
	if(vertices != NULL){
		skyboxVertices = vertices;
	} else if (projType==camPERSPECTIVE){
		GLdouble perspectiveVertices[] = {
	    // Positions          
		  -0.5f,  0.5f, -0.5f,
		  -0.5f, -0.5f, -0.5f,
		   0.5f, -0.5f, -0.5f,
		   0.5f, -0.5f, -0.5f,
		   0.5f,  0.5f, -0.5f,
		  -0.5f,  0.5f, -0.5f,
		  
		  -0.5f, -0.5f,  0.5f,
		  -0.5f, -0.5f, -0.5f,
		  -0.5f,  0.5f, -0.5f,
		  -0.5f,  0.5f, -0.5f,
		  -0.5f,  0.5f,  0.5f,
		  -0.5f, -0.5f,  0.5f,
		  
		   0.5f, -0.5f, -0.5f,
		   0.5f, -0.5f,  0.5f,
		   0.5f,  0.5f,  0.5f,
		   0.5f,  0.5f,  0.5f,
		   0.5f,  0.5f, -0.5f,
		   0.5f, -0.5f, -0.5f,
		   
		  -0.5f, -0.5f,  0.5f,
		  -0.5f,  0.5f,  0.5f,
		   0.5f,  0.5f,  0.5f,
		   0.5f,  0.5f,  0.5f,
		   0.5f, -0.5f,  0.5f,
		  -0.5f, -0.5f,  0.5f,
		  
		  -0.5f,  0.5f, -0.5f,
		   0.5f,  0.5f, -0.5f,
		   0.5f,  0.5f,  0.5f,
		   0.5f,  0.5f,  0.5f,
		  -0.5f,  0.5f,  0.5f,
		  -0.5f,  0.5f, -0.5f,
		  
		  -0.5f, -0.5f, -0.5f,
		  -0.5f, -0.5f,  0.5f,
		   0.5f, -0.5f, -0.5f,
		   0.5f, -0.5f, -0.5f,
		  -0.5f, -0.5f,  0.5f,
		   0.5f, -0.5f,  0.5f
		};
		skyboxVertices = perspectiveVertices;
	} else {
		GLdouble orthographicVertices[] = {
   		// Positions          
	      -100.0f,  100.0f, -100.0f,
		  -100.0f, -100.0f, -100.0f,
		   100.0f, -100.0f, -100.0f,
		   100.0f, -100.0f, -100.0f,
		   100.0f,  100.0f, -100.0f,
		  -100.0f,  100.0f, -100.0f,
		  
		  -100.0f, -100.0f,  100.0f,
		  -100.0f, -100.0f, -100.0f,
		  -100.0f,  100.0f, -100.0f,
		  -100.0f,  100.0f, -100.0f,
		  -100.0f,  100.0f,  100.0f,
		  -100.0f, -100.0f,  100.0f,
		  
		   100.0f, -100.0f, -100.0f,
		   100.0f, -100.0f,  100.0f,
		   100.0f,  100.0f,  100.0f,
		   100.0f,  100.0f,  100.0f,
		   100.0f,  100.0f, -100.0f,
		   100.0f, -100.0f, -100.0f,
		   
		  -100.0f, -100.0f,  100.0f,
		  -100.0f,  100.0f,  100.0f,
		   100.0f,  100.0f,  100.0f,
		   100.0f,  100.0f,  100.0f,
		   100.0f, -100.0f,  100.0f,
		  -100.0f, -100.0f,  100.0f,
		  
		  -100.0f,  100.0f, -100.0f,
		   100.0f,  100.0f, -100.0f,
		   100.0f,  100.0f,  100.0f,
		   100.0f,  100.0f,  100.0f,
		  -100.0f,  100.0f,  100.0f,
		  -100.0f,  100.0f, -100.0f,
		  
		  -100.0f, -100.0f, -100.0f,
		  -100.0f, -100.0f,  100.0f,
		   100.0f, -100.0f, -100.0f,
		   100.0f, -100.0f, -100.0f,
		  -100.0f, -100.0f,  100.0f,
		   100.0f, -100.0f,  100.0f
		};
		skyboxVertices = orthographicVertices;

	}
	
	glUseProgram(skybox->programBack);
	glGenBuffers(1, &(skybox->vbo));
	glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(GLdouble), skyboxVertices, GL_STATIC_DRAW);
	glGenVertexArrays(1, &(skybox->vao));
	glBindVertexArray(skybox->vao);
	glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo);
	glEnableVertexAttribArray(skybox->backLoc);
	glVertexAttribPointer(skybox->backLoc, 3, GL_DOUBLE, GL_FALSE, 0, NULL);
	glBindVertexArray(0);

	create_cube_map(textures[0], textures[1], textures[2],
		 textures[3], textures[4], textures[5], &(skybox->texBackground));
}

void skyboxInitializeSkyboxShader(skyboxSkybox *skybox){
	GLchar vertexCode[] = "\
		#version 140\n\
		in vec3 position;\
		out vec3 TexCoords;\
		uniform mat4 projectionView;\
		void main(){\
		    gl_Position =  projectionView * vec4(position, 1.0);\
		    TexCoords = position;\
		}"; 
	GLchar fragmentCode[] = "\
		#version 140\n\
		in vec3 TexCoords;\
		out vec4 color;\
		uniform samplerCube skybox;\
		void main(){\
			vec3 fog = vec3(0.0, 0.0, 0.1);\
			vec3 tex = vec3(texture(skybox, TexCoords));\
			if(tex[0]<0.1 && tex[1]<0.1 && tex[2]<0.1){\
		    	color = vec4(mix(tex, fog, 0.9), 1.0);\
		    } else {\
		    	color = vec4(tex, 1.0);\
		    }\
		}"; 
	skybox->programBack = makeProgram(vertexCode, fragmentCode);
	glUseProgram(skybox->programBack);
	skybox->backLoc = glGetAttribLocation(skybox->programBack, "position");
	skybox->backView = glGetUniformLocation(skybox->programBack, "projectionView");
	skybox->skybox = glGetUniformLocation(skybox->programBack, "skybox");
}

void skyboxSkyboxRender(skyboxSkybox *skybox, sceneNode *node){
	GLdouble parent[4][4], rotation[3][3];
	GLfloat parentFloat[4][4];
	GLdouble transl[3] = {0.0, 0.0, 0.0};
	glUseProgram(skybox->programBack);
	
	mat44Combine(node->rotation, transl, parent);
	mat44OpenGL(parent, parentFloat);
	glUniformMatrix4fv(skybox->backView, 1, GL_FALSE, (GLfloat *)parentFloat);
	glDepthMask(GL_FALSE);
	
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->texBackground.openGL);
	glUniform1i(skybox->skybox, 0);
	glEnableVertexAttribArray(skybox->backLoc);
	glBindVertexArray(skybox->vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
	glBindVertexArray(0);
}