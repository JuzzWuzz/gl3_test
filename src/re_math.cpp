/*****************************************************************************
 * reMath contains vector and matrix class
 *
 * Author: rewolf 2010
 * andrew.flower@gmail.com
 *****************************************************************************/

#include <stdlib.h>
#include <string>
#include <cstring>
#include <cmath>
using namespace std;

#include "re_math.h"


namespace reMath{

	/**************************************************************************
	 * matrix4 
	 **************************************************************************/

	//-------------------------------------------------------------------------
	// Constructs an identity matrix
	matrix4::matrix4(){
		SetIdentity();
	}

	//-------------------------------------------------------------------------
	// Constructs a copy of the given matrix
	matrix4::matrix4(const matrix4& copy){
		memcpy((void*)m, (void*)copy.m, sizeof(m));
	}

	//-------------------------------------------------------------------------
	// Constructs a matrix with the given elements
	matrix4::matrix4(const float elems[16]){
		memcpy((void*)m, (void*)elems, sizeof(m));
	}

	//-------------------------------------------------------------------------
	// STR returns a formatted string representation of the matrix
	string 
	matrix4::str( void ){
		char out[256];
		// column-major
		sprintf(out,"\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n",
			m[0],m[4],m[8],m[12],m[1],m[5],m[9],m[13],m[2],m[6],m[10],m[14],m[3],m[7],m[11],m[15]);
		return string(out);
	}

	//-------------------------------------------------------------------------
	// OPERATOR+ performs matrix addition
	matrix4 
	matrix4::operator +(const matrix4 &mat)const{
		matrix4 out;
		for (int i = 0; i < 16; i++)
			out[i] = m[i]+mat.m[i];
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs matrix subtraction
	matrix4 
	matrix4::operator -(const matrix4 &mat)const{
		matrix4 out;
		for (int i = 0; i < 16; i++)
			out[i] = m[i]-mat.m[i];
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-scalar multiplication
	matrix4 
	matrix4::operator *(float scalar)const{
		matrix4 out;
		for (int i = 0; i < 16; i++)
			out[i] = m[i] * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix multiplication
	matrix4 
	matrix4::operator *(const matrix4 &mat)const{
		matrix4 out;

		int col;
		for (int i = 0; i < 4; i++){
			col=0;
			for (int j = 0; j < 4; j++){
				out.m[i+col] = 	m[i] 		* 	mat.m[col] 		+
								m[i+4]		*	mat.m[col+1] 	+
								m[i+8]		*	mat.m[col+2] 	+
								m[i+12]		* 	mat.m[col+3];

				col+=4;
			}
		}
		return out;
	}
	
	//-------------------------------------------------------------------------
	// OPERATOR*= performs (inplace) matrix multiplications
	void 
	matrix4::operator *=(const matrix4 &mat){
		float temp[16];

		int col;
		for (int i = 0; i < 4; i++){
			col=0;
			for (int j = 0; j < 4; j++){
				temp[i+col] = 	m[i] 		* 	mat.m[col]	 	+
								m[i+4]		*	mat.m[col+1] 	+
								m[i+8]		*	mat.m[col+2] 	+
								m[i+12]		* 	mat.m[col+3];
				col+=4;
			}
		}
		memcpy(this->m, temp, sizeof(float)*16);
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-vector4 multiplication
	vector4
	matrix4::operator *(const vector4& vec)const{
		vector4 product;
		product.x = m[0]*vec.x+m[4]*vec.y+m[8]*vec.z+m[12]*vec.w;
		product.y = m[1]*vec.x+m[5]*vec.y+m[9]*vec.z+m[13]*vec.w;
		product.z = m[2]*vec.x+m[6]*vec.y+m[10]*vec.z+m[14]*vec.w;
		product.w = m[3]*vec.x+m[7]*vec.y+m[11]*vec.z+m[15]*vec.w;
		return product;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs matrix-vector3 multiplication
	vector3
	matrix4::operator *(const vector3& vec)const {
		vector3 product;
		product.x = m[0]*vec.x+m[4]*vec.y+m[8]*vec.z;
		product.y = m[1]*vec.x+m[5]*vec.y+m[9]*vec.z;
		product.z = m[2]*vec.x+m[6]*vec.y+m[10]*vec.z;
		return product;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs scalar-matrix multiplication
	matrix4 
	operator*(float scalar, const matrix4& mat){
		matrix4 out;
		for (int i = 0; i < 16; i++)
			out[i] = mat.m[i] * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR+= performs (in-place) matrix addition
	matrix4& 
	matrix4::operator +=(const matrix4 &mat){
		for (int i = 0; i < 16; i++)
			m[i]+=mat.m[i];
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR-= performs (in-place) matrix subtraction
	matrix4& 
	matrix4::operator -=(const matrix4 &mat){
		for (int i = 0; i < 16; i++)
			m[i]-=mat.m[i];
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR== performs a per-element equality test
	bool 
	matrix4::operator ==(const matrix4 &mat)const{
		for (int i = 0; i < 16; i++)
			if (m[i]!=mat.m[i])
				return false;
		return true;
	}

	//-------------------------------------------------------------------------
	// OPERATOR!= performs a per-element equality test, return true if any fail
	bool 
	matrix4::operator !=(const matrix4 &mat)const{
		for (int i = 0; i < 16; i++)
			if (m[i]!=mat.m[i])
				return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR[] returns a reference to the element at the given index.
	float& 
	matrix4::operator[](int idx){
		return m[idx];
	}

	//-------------------------------------------------------------------------
	// SETIDENTITY Resets the matrix to an identity matrix
	void 
	matrix4::SetIdentity(){
		memset((void*)m, 0, sizeof(m));
		m[0] = m[5]= m[10] = m[15] = 1.0f;
	}

	//-------------------------------------------------------------------------
	// TRANSPOSE Returns the transpose of the matrix.
	matrix4 
	matrix4::Transpose(){
		matrix4 out;
		for (int i = 0; i < 4; i ++){
			for (int j = 0; j < 4; j++){
				out.m[j*4+i] = m[i*4+j];
			}
		}
		return out;
	}

	//-------------------------------------------------------------------------
	// DETERMINANT Returns the determinant of the matrix.
	float 
	matrix4::Determinant(){
		// calculate the common 2x2 determinants
		float m_10_14 = m[10]*m[15] - m[14]*m[11]; // m10*m15 - m14m11
		float m_6_14  = m[6] *m[15] - m[14]*m[7];
		float m_6_10  = m[6] *m[11] - m[10]*m[7];

		float m_2_14  = m[2] *m[15] - m[14]*m[3];
		float m_2_10  = m[2] *m[11] - m[10]*m[3];
		float m_2_6   = m[2] *m[7]  - m[6] *m[3];

		// return the main determinant (4 * 3x3 dets)
		return 	+ m[0] * (m[5]*m_10_14 	- m[9]*m_6_14	+ m[13]*m_6_10)
				- m[4] * (m[1]*m_10_14	- m[9]*m_2_14	+ m[13]*m_2_10)
				+ m[8] * (m[1]*m_6_14	- m[5]*m_2_14	+ m[13]*m_2_6)
				- m[12]* (m[1]*m_6_10	- m[5]*m_2_10	+ m[9] *m_2_6);
	}

	/**************************************************************************
	 * vector4 - not fully functional
	 **************************************************************************/

	//-------------------------------------------------------------------------
	// Constructs a zero vector with w = 1
	vector4::vector4(){
		memset((void*)v, 0, 3*sizeof(float));
		w=1.0f;
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given elements
	vector4::vector4(const float elems[4]){
		memcpy((void*)v, (void*)elems, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// Constructs a copy of the given vector
	vector4::vector4(const vector4& copy){
		memcpy((void*)v, (void*)copy.v, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// OPERATOR+ performs vector addition, returning 
	vector3 
	vector4::operator +(const vector4 &pt) const{
		vector3 out;
		out.x = x + pt.x;
		out.y = y + pt.y;
		out.z = z + pt.z;
		return out;
	}

	//-------------------------------------------------------------------------
	vector3 
	vector4::operator -(const vector4 &pt) const{
		vector3 out;
		out.x = x - pt.x;
		out.y = y - pt.y;
		out.z = z - pt.z;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4 vector4::operator +(const vector3 &vec) const{
		vector4 out;
		out.x = x + vec.x;
		out.y = y + vec.y;
		out.z = z + vec.z;
		out.w = 1.0f;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4 vector4::operator -(const vector3 &vec) const{
		vector4 out;
		out.x = x - vec.x;
		out.y = y - vec.y;
		out.z = z - vec.z;
		out.w = 1.0f;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4 vector4::operator *(float scalar) const{
		vector4 out;
		out.x = x * scalar;
		out.y = y * scalar;
		out.z = z * scalar;
		out.w = 1.0f;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4 operator *(float scalar, vector4& pt){
		vector4 out;
		out.x = pt.x * scalar;
		out.y = pt.y * scalar;
		out.z = pt.z * scalar;
		out.w = 1.0f;
		return out;
	}

	//-------------------------------------------------------------------------
	vector4& vector4::operator +=(const vector4 &pt){
		x += pt.x;
		y += pt.y;
		z += pt.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	vector4& vector4::operator -=(const vector4 &pt){
		x -= pt.x;
		y -= pt.y;
		z -= pt.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	vector4& vector4::operator +=(const vector3 &vec) {
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	vector4& vector4::operator -=(const vector3 &vec) {
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	bool vector4::operator ==(const vector4& pt) const{
		if (x!=pt.x || y!=pt.y || z!=pt.z)
			return false;
		return true;
	}

	//-------------------------------------------------------------------------
	bool vector4::operator !=(const vector4& pt) const{
		if (x!=pt.x || y!=pt.y || z!=pt.z)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	float& vector4::operator [](int idx){
		return v[idx];
	}

	//-------------------------------------------------------------------------
	string vector4::str(){
		char out[64];
		sprintf(out,"[%.2f, %.2f, %.2f, %.2f]",x,y,z,w);
		return string(out);
	}
	
	/**************************************************************************
	 * vector3 
	 **************************************************************************/

	//-------------------------------------------------------------------------
	// Constructs a zero vector
	vector3::vector3(){
		memset((void*)v, 0, 3*sizeof(float));
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given components
	vector3::vector3(float _x, float _y, float _z){
		x=_x; y=_y; z=_z;
	}

	//-------------------------------------------------------------------------
	// Constructs a vector from the given components
	vector3::vector3(const float elems[3]){
		memcpy((void*)v, (void*)elems, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// Constructs a copy of the given vector
	vector3::vector3(const vector3& copy){
		memcpy((void*)v, (void*)copy.v, sizeof(v));
	}

	//-------------------------------------------------------------------------
	// OPERATOR+ performs vector addition
	vector3 
	vector3::operator +(const vector3 &vec) const{
		vector3 out;
		out.x = x + vec.x;
		out.y = y + vec.y;
		out.z = z + vec.z;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs vector subtraction
	vector3 
	vector3::operator -(const vector3 &vec) const{
		vector3 out;
		out.x = x - vec.x;
		out.y = y - vec.y;
		out.z = z - vec.z;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR- performs vector negation.
	vector3 
	vector3::operator -() const{
		vector3 out;
		out.x = - x;
		out.y = - y;
		out.z = - z;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs vector-scalar multiplication
	vector3 
	vector3::operator *(float scalar) const{
		vector3 out;
		out.x = x * scalar;
		out.y = y * scalar;
		out.z = z * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR* performs scalar-vector multiplication
	vector3 
	operator *(float scalar, vector3& vec){
		vector3 out;
		out.x = vec.x * scalar;
		out.y = vec.y * scalar;
		out.z = vec.z * scalar;
		return out;
	}

	//-------------------------------------------------------------------------
	// OPERATOR+= performs (in-place) vector addition
	vector3& 
	vector3::operator +=(const vector3 &vec){
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR-= performs (in-place) vector subtraction
	vector3& 
	vector3::operator -=(const vector3 &vec){
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR*= performs (in-place) scalar multiplication
	vector3& 
	vector3::operator *=(float scalar){
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	//-------------------------------------------------------------------------
	// OPERATOR== tests whether this vector is equal to another
	bool 
	vector3::operator ==(const vector3& vec) const{
		if (x!=vec.x || y!=vec.y || z!=vec.z)
			return false;
		return true;
	}

	//-------------------------------------------------------------------------
	// OPERATOR!= tests whether this vector is not equal to another
	bool
	vector3::operator !=(const vector3& vec) const{
		if (x!=vec.x || y!=vec.y || z!=vec.z)
			return true;
		return false;
	}

	//-------------------------------------------------------------------------
	// OPERATOR[] returns a reference to the element at the given index
	float& 
	vector3::operator [](int idx){
		return v[idx];
	}

	//-------------------------------------------------------------------------
	// STR returns a formatted string representation of the vector
	string 
	vector3::str(){
		char out[64];
		sprintf(out,"<%.2f, %.2f, %.2f>",x,y,z);
		return string(out);
	}

	//-------------------------------------------------------------------------
	// MAG returns the length/magnitude of the vector
	float 
	vector3::Mag()const{
		return sqrt(x*x+y*y+z*z);
	}

	//-------------------------------------------------------------------------
	// MAG2 returns the magnitude squared
	float 
	vector3::Mag2()const{
		return x*x+y*y+z*z;
	}

	//-------------------------------------------------------------------------
	// DOT returns the dot product with the given vector
	float 
	vector3::Dot(const vector3 & v) const{
		return x*v.x + y*v.y + z*v.z;
	}

	//-------------------------------------------------------------------------
	// CROSS returns the cross product with the given vector
	vector3 
	vector3::Cross(const vector3 &v) const{
		return vector3(y*v.z-z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
	}

	//-------------------------------------------------------------------------
	// NORMALIZE scales this vector to magnitude 1
	void 
	vector3::Normalize(){
		float magInv = 1.0f/Mag();
		x*=magInv;
		y*=magInv;
		z*=magInv;
	}

	//-------------------------------------------------------------------------
	// Returns a normalized version of this vector
	vector3 
	vector3::GetUnit()const{
		float magInv = 1.0f/Mag();
		return vector3(x*magInv, y*magInv, z*magInv);
	}

	/******************************************************************************
	 * Projection Transform Functions
	 ******************************************************************************/

	//-------------------------------------------------------------------------
	// FRUSTUM_PROJ Returns a matrix that projects a vector to the screen with 
	// perspective.  Allows for assymmetric frustums.
	matrix4
	frustum_proj (float left, float right, float bottom, float top, float near, float far){
		matrix4 mat4;
		float* mat = mat4.m;
		// first column
		mat[0] = 2*near / (right - left);
		// second 
		mat[5] = 2*near / (top - bottom);
		// third 
		mat[8] = (right+left) / (right-left);
		mat[9] = (top+bottom)/(top-bottom);
		mat[10]= -(far+near)/(far-near);
		mat[11]= -1.0f;
		// last column
		mat[14]= -(2*far*near)/(far-near);
		mat[15]= .0f;

		return mat4;
	}

	//-------------------------------------------------------------------------
	// PERSPECTIVE_PROJ Returns a matrix that projects a vector to the screen 
	// using a symmetric frustum to create the perspective effect.  The 
	// vertical Field-Of-View fovy, is in RADIANS
	matrix4
	perspective_proj(float fovy, float aspect, float near, float far){
		matrix4 mat4;
		float* mat = mat4.m;
		float f = 1/tan(fovy*.5f);
		// first column
		mat[0] = f/aspect;
		// second
		mat[5] = f;
		// third
		mat[10] = (far+near)/(near-far);
		mat[11] = -1.0f;
		// last column
		mat[14] = 2*far*near/(near - far);

		return mat4;
	}

	//-------------------------------------------------------------------------
	// ORTHO_PROJ projects a vector to the screen orthogonally
	matrix4
	ortho_proj(float left, float right, float bottom, float top, float near, float far){
		matrix4 mat4;
		float* mat = mat4.m;

		// first column
		mat[0] = 2.0f/(right-left);
		// second
		mat[5] = 2.0f/(top-bottom);
		// third
		mat[10] = -2.0f/(far-near);
		// last column
		mat[12] = -(right+left)/(right=left);
		mat[13] = -(top+bottom)/(top-bottom);
		mat[14] = -(far+near)/(far-near);
		mat[15] = 1.0f;

		return mat4;
	}

	//-------------------------------------------------------------------------
	// ORTHO_PROJ projects a vector to the screen orthogonally but assumes
	// a near and far plane of -1 and 1 respectively.
	matrix4 
	ortho2d_proj(float left, float right, float bottom, float top){
		matrix4 mat4;
		float* mat = mat4.m;

		// first column
		mat[0] = 2.0f/(right-left);
		// second
		mat[5] = 2.0f/(top-bottom);
		// third
		mat[10] = -1.0f;
		// last column
		mat[12] = -(right+left)/(right=left);
		mat[13] = -(top+bottom)/(top-bottom);
		mat[14] = .0f;
		mat[15] = 1.0f;

		return mat4;
	}

	/******************************************************************************
	 * Transform Functions
	 * post-fix mult
	 ******************************************************************************/

	//-------------------------------------------------------------------------
	// TRANSLATE_TR returns a matrix that may be used for vector translation
	// if the vector is pre-multiplied by this matrix.
	matrix4
	translate_tr (float x, float y, float z){
		matrix4 mat;
		mat[12] =x;
		mat[13] =y;
		mat[14] =z;
		return mat;
	}

	//-------------------------------------------------------------------------
	// ROTATE_TR returns a rotation transform matrix for pre-multiplication
	// of vectors. The angle must be specified in RADIANS
	matrix4
	rotate_tr (float angle, float x, float y, float z){
		matrix4 mat4;
		float* mat = mat4.m;
		float c = cosf(angle);
		float s = sinf(angle);

		// first column
		mat[0] = x*x*(1-c)+c; 
		mat[1] = y*x*(1-c)+z*s;
		mat[2] = x*z*(1-c)-y*s;
		mat[3] = .0f;
		// second
		mat[4] = x*y*(1-c)-z*s;
		mat[5] = y*y*(1-c)+c;
		mat[6] = y*z*(1-c)+x*s;
		mat[7] = .0f;
		// third
		mat[8] = x*z*(1-c)+y*s;
		mat[9] = y*z*(1-c)-x*s;
		mat[10] = z*z*(1-c)+c;
		mat[11] = .0f;
		// last column
		mat[12] = .0f;
		mat[13] = .0f;
		mat[14] = .0f;
		mat[15] = 1.0f;
		return mat4;
	}

	//-------------------------------------------------------------------------
	// SCALE_TR returns a scaling transformation matrix that can scale vectors
	// via pre-multiplication of the vector.
	matrix4
	scale_tr (float x, float y, float z){
		matrix4 mat4;

		mat4.m[0]=x;
		mat4.m[5]=y;
		mat4.m[10]=z;
		
		return mat4;
	}
}

