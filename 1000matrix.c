/*
520matrix.c
Written by Liv Phillips for CS311, Winter 2017
Implements functions to work with matrices
*/
/*** 2 x 2 Matrices ***/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat22Print(GLdouble m[2][2]) {
	for(int i=0;i<2;i++){
		printf("%f %f\n", m[i][0], m[i][1]);
	}
}

/* Returns the determinant of the matrix m. If the determinant is 0.0, then the 
matrix is not invertible, and mInv is untouched. If the determinant is not 0.0, 
then the matrix is invertible, and its inverse is placed into mInv. */
GLdouble mat22Invert(GLdouble m[2][2], GLdouble mInv[2][2]) {
	GLdouble det = (m[0][0]*m[1][1])-(m[0][1]*m[1][0]);
	if(det != 0.0){
		mInv[0][0]=m[1][1];
		mInv[1][1]=m[0][0];
		mInv[0][1]=-m[0][1];
		mInv[1][0]=-m[1][0];
		for(int i=0;i<2;i++){
			mInv[i][0]*=(1.0/det);
			mInv[i][1]*=(1.0/det);
		}
	}
	return det;
}

/* Multiplies a 2x2 matrix m by a 2-column v, storing the result in mTimesV. 
The output should not */
void mat221Multiply(GLdouble m[2][2], GLdouble v[2], GLdouble mTimesV[2]) {
	for(int i=0;i<2;i++){
		mTimesV[i]=(m[i][0]*v[0])+(m[i][1]*v[1]);
	}
	
}

/* Fills the matrix m from its two columns. */
void mat22Columns(GLdouble col0[2], GLdouble col1[2], GLdouble m[2][2]) {
	for(int i=0;i<2;i++){
		m[i][0]=col0[i];
		m[i][1]=col1[i];
	}
}

/*** 3 X 3 matrices **/

/* Pretty-prints the given matrix, with one line of text per row of matrix. */
void mat33Print(GLdouble m[3][3]) {
	for(int i=0;i<3;i++){
		printf("%f %f %f\n", m[i][0], m[i][1], m[i][2]);
	}
}

/* Multiplies the 3x3 matrix m by the 3x3 matrix n. */
void mat333Multiply(GLdouble m[3][3], GLdouble n[3][3], GLdouble mTimesN[3][3]){
	for(int i=0;i<3;i++){
		for (int j=0; j<3; j++){
			mTimesN[i][j]=(m[i][0]*n[0][j])+(m[i][1]*n[1][j])+(m[i][2]*n[2][j]);
		}
	}
}

/* Multiplies the 3x3 matrix m by the 3x1 matrix v. */
void mat331Multiply(GLdouble m[3][3], GLdouble v[3], GLdouble mTimesV[3]){
	for(int i=0;i<3;i++){
		mTimesV[i]=(m[i][0]*v[0])+(m[i][1]*v[1])+(m[i][2]*v[2]);
	}

}

/* Builds a 3x3 matrix representing 2D rotation and translation in homogeneous 
coordinates. More precisely, the transformation first rotates through the angle 
theta (in radians, counterclockwise), and then translates by the vector (x, y). 
*/
void mat33Isometry(GLdouble theta, GLdouble x, GLdouble y, GLdouble isom[3][3]){
	for(int i=0; i<3; i++){
		if (i==0){
			isom[i][0]=cos(theta);
			isom[i][1]=-sin(theta);
			isom[i][2]=x;
		} else if(i==1){
			isom[i][0]=sin(theta);
			isom[i][1]=cos(theta);
			isom[i][2]=y;
		}
	}
}
void mat33FormMatrix(GLdouble U[3][3], GLdouble axis[3]){
	for(int i=0; i<3; i++){
		U[i][i]=0.0;
		if(i==0){
			U[i][2]=axis[1];
			U[2][i]=-axis[1];
		} else if(i==1){
			U[i][0]=axis[2];
			U[0][i]=-axis[2];
		} else if(i==2){
			U[i][1]=axis[0];
			U[1][i]=-axis[0];
		}
	}	
}

void mat33Identity(GLdouble id[3][3]){
	for (int i=0; i<3; i++){
		id[i][0]=0.0;
		id[i][1]=0.0;
		id[i][2]=0.0;
	}
	for (int j=0; j<3;j++){
		id[j][j]=1.0;
	}
}

void mat33Scale(GLdouble scalar, GLdouble m[3][3], GLdouble mScale[3][3]){
	for(int i=0;i<3;i++){
		mScale[i][0]=m[i][0]*scalar;
		mScale[i][1]=m[i][1]*scalar;
		mScale[i][2]=m[i][2]*scalar;
	}
}

void mat333Add(GLdouble m[3][3], GLdouble m2[3][3], GLdouble mPlusM[3][3]){
	for (int i=0;i<3;i++){
		mPlusM[i][0]=m[i][0]+m2[i][0];
		mPlusM[i][1]=m[i][1]+m2[i][1];
		mPlusM[i][2]=m[i][2]+m2[i][2];
	}
}

/* Given a length-1 3D vector axis and an angle theta (in radians), builds the 
rotation matrix for the rotation about that axis through that angle. Based on 
Rodrigues rotation formula R = I + (sin theta) U + (1 - cos theta) U^2. */
void mat33AngleAxisRotation(GLdouble theta, GLdouble axis[3], GLdouble rot[3][3]){
	GLdouble U[3][3];
	GLdouble UScale[3][3];
	GLdouble U2Scale[3][3];
	GLdouble U2[3][3];
	GLdouble id[3][3];
	GLdouble rotHold[3][3];

	mat33FormMatrix(U, axis);
	mat333Multiply(U, U, U2);

	mat33Identity(id);
	mat33Scale(sin(theta), U, UScale);
	mat33Scale((1-cos(theta)), U2, U2Scale);
	mat333Add(id, UScale, rotHold);
	mat333Add(rotHold, U2Scale, rot);
}

/* Fills the matrix m from its two columns. */
void mat33Columns(GLdouble col0[3], GLdouble col1[3], GLdouble col2[3], 
	GLdouble m[3][3]) {
	for(int i=0;i<3;i++){
		m[i][0]=col0[i];
		m[i][1]=col1[i];
		m[i][2]=col2[i];
	}
}

void mat333Transpose(GLdouble rot[3][3], GLdouble isom[3][3]){
	for(int i=0;i<3;i++){
		for (int j=0; j<3; j++){
			isom[i][j]=rot[j][i];
		}
	}
}

void mat334Transpose(GLdouble rot[3][3], GLdouble isom[4][4]){
	for(int i=0;i<3;i++){
		for (int j=0; j<3; j++){
			isom[j][i]=rot[i][j];
		}
	}
}
/* Given two length-1 3D vectors u, v that are perpendicular to each other. 
Given two length-1 3D vectors a, b that are perpendicular to each other. Builds 
the rotation matrix that rotates u to a and v to b. */
void mat33BasisRotation(GLdouble u[3], GLdouble v[3], GLdouble a[3], GLdouble b[3], 
        GLdouble rot[3][3]){
	GLdouble w[3];
	GLdouble c[3];
	GLdouble S[3][3];
	GLdouble R[3][3];
	GLdouble RTrans[3][3];
	vec3Cross(u, v, w);
	vec3Cross(a, b, c);
	mat33Columns(u, v, w, R);
	mat33Columns(a, b, c, S);
	mat333Transpose(R, RTrans);
	mat333Multiply(S, RTrans, rot);
}

/*** 4 X 4 matrices ***/
void mat44Print(GLdouble m[4][4]) {
	for(int i=0;i<4;i++){
		printf("%f %f %f %f\n", m[i][0], m[i][1], m[i][2], m[i][3]);
	}
}

void mat44GLfloatPrint(GLfloat m[4][4]) {
	for(int i=0;i<4;i++){
		printf("%f %f %f %f\n", m[i][0], m[i][1], m[i][2], m[i][3]);
	}
}

void mat44Identity(GLdouble id[4][4]){
	for (int i=0; i<4; i++){
		id[i][0]=0.0;
		id[i][1]=0.0;
		id[i][2]=0.0;
		id[i][3]=0.0;
	}
	for (int j=0; j<4;j++){
		id[j][j]=1.0;
	}
}
/* Multiplies m by n, placing the answer in mTimesN. */
void mat444Multiply(GLdouble m[4][4], GLdouble n[4][4], GLdouble mTimesN[4][4]){
	for(int i=0;i<4;i++){
		for (int j=0; j<4; j++){
			mTimesN[i][j]=(m[i][0]*n[0][j])
			+(m[i][1]*n[1][j])
			+(m[i][2]*n[2][j])
			+(m[i][3]*n[3][j]);
		}
	}
}

/* Multiplies m by v, placing the answer in mTimesV. */
void mat441Multiply(GLdouble m[4][4], GLdouble v[4], GLdouble mTimesV[4]){
	for(int i=0;i<4;i++){
		mTimesV[i]=(m[i][0]*v[0])+(m[i][1]*v[1])+(m[i][2]*v[2])+(m[i][3]*v[3]);
	}
}

/* Given a rotation and a translation, forms the 4x4 homogeneous matrix 
representing the rotation followed in time by the translation. */
void mat44Isometry(GLdouble rot[3][3], GLdouble trans[3], GLdouble isom[4][4]){
	mat44Identity(isom);

	for(int i=0;i<3;i++){
		for (int j=0; j<3; j++){
			isom[i][j]=rot[i][j];
		}
	}
	for (int k=0; k<3; k++){
		isom[k][3]=trans[k];

	}
}

/* Given a rotation and translation, forms the 4x4 homogeneous matrix 
representing the inverse translation followed in time by the inverse rotation. 
That is, the isom produced by this function is the inverse to the isom 
produced by mat44Isometry on the same inputs. */
void mat44InverseIsometry(GLdouble rot[3][3], GLdouble trans[3], 
        GLdouble isom[4][4]){
	mat44Identity(isom);
	mat334Transpose(rot, isom);

	for (int k=0; k<3; k++){
		isom[k][3]=(-rot[0][k]*trans[0])
		-(rot[1][k]*trans[1])
		-(rot[2][k]*trans[2]);
	}
}

/* Builds a 4x4 matrix representing orthographic projection with a boxy viewing 
volume [left, right] x [bottom, top] x [far, near]. That is, on the near plane 
the box is the rectangle R = [left, right] x [bottom, top], and on the far 
plane the box is the same rectangle R. Keep in mind that 0 > near > far. Maps 
the viewing volume to [-1, 1] x [-1, 1] x [-1, 1]. */
void mat44Orthographic(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, 
        GLdouble far, GLdouble near, GLdouble proj[4][4]){
	mat44Identity(proj);
	proj[0][0]=(2.0/(right-left));
	proj[0][3]=((-right-left)/(right-left));
	proj[1][1]=(2.0/(top-bottom));
	proj[1][3]=((-top-bottom)/(top-bottom));
	proj[2][2]=-(2.0/(near-far));
	proj[2][3]=-((-near-far)/(near-far));
}

/* Builds a 4x4 matrix that maps a projected viewing volume 
[-1, 1] x [-1, 1] x [-1, 1] to screen [0, w - 1] x [0, h - 1] x [-1, 1]. */
void mat44Viewport(GLdouble width, GLdouble height, GLdouble view[4][4]){
	mat44Identity(view);
	view[0][0]=((width-1.0)/2.0);
	view[0][3]=((width-1.0)/2.0);
	view[1][1]=((height-1.0)/2.0);
	view[1][3]=((height-1.0)/2.0);
}

/* Builds a 4x4 matrix representing perspective projection. The viewing frustum 
is contained between the near and far planes, with 0 > near > far. On the near 
plane, the frustum is the rectangle R = [left, right] x [bottom, top]. On the 
far plane, the frustum is the rectangle (far / near) * R. Maps the viewing 
volume to [-1, 1] x [-1, 1] x [-1, 1]. */
void mat44Perspective(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, 
        GLdouble far, GLdouble near, GLdouble proj[4][4]){
	mat44Identity(proj);
	proj[0][0]=((-2.0*near)/(right-left));
	proj[0][2]=((right+left)/(right-left));
	proj[1][1]=((-2.0*near)/(top-bottom));
	proj[1][2]=((top+bottom)/(top-bottom));
	proj[2][2]=-((-near-far)/(near-far));
	proj[2][3]=-((2.0*near*far)/(near-far));
	proj[3][2]=-1.0;
	proj[3][3]=0.0;

}

/* We want to pass matrices into OpenGL, but there are two obstacles. First, 
our matrix library uses GLdouble matrices, but OpenGL 2.x expects GLfloat 
matrices. Second, C matrices are implicitly stored one-row-after-another, while 
OpenGL expects matrices to be stored one-column-after-another. This function 
plows through both of those obstacles. */
void mat44OpenGL(GLdouble m[4][4], GLfloat openGL[4][4]) {
	for (int i = 0; i < 4; i += 1)
		for (int j = 0; j < 4; j += 1)
			openGL[i][j] = m[j][i];
}

void mat44Combine(GLdouble rot[3][3], GLdouble transl[3], GLdouble model[4][4]){
	mat44Identity(model);

	for(int i=0;i<3;i++){
		for(int k=0;k<3;k++){
			model[i][k]=rot[i][k];
		}
	}
	for(int i=0;i<3;i++){
		model[i][3]=transl[i];
	}
}

void mat44Disect(GLdouble model[4][4], GLdouble rot[3][3], GLdouble transl[3]){
	for(int i=0;i<3;i++){
		for(int k=0;k<3;k++){
			rot[i][k]=model[i][k];
		}
	}
	for(int i=0;i<3;i++){
		transl[i]=model[i][3];
	}
}

void mat44Copy(GLdouble original[4][4], GLdouble new[4][4]){
	for(int i=0;i<4;i++){
		for(int k=0;k<4;k++){
			new[i][k]=original[i][k];
		}
	}
}