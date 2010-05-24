/*****************************************************************************
 * reMath contains vector and matrix class
 *
 * Created by rewolf 2010
 * andrew.flower@gmail.com
 *****************************************************************************/

#include "regl3.h"
#include "util.h"

///////////////////////////////////////////////////////////////////////////////
// reMatrix4 - column major indices
///////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------
reMatrix4::reMatrix4(){
	SetIdentity();
}

//--------------------------------------------------------
reMatrix4::reMatrix4(const reMatrix4& copy){
	memcpy((void*)m, (void*)copy.m, sizeof(m));
}

//--------------------------------------------------------
reMatrix4::reMatrix4(const float elems[16]){
	memcpy((void*)m, (void*)elems, sizeof(m));
}

//--------------------------------------------------------
string reMatrix4::str( void ){
	char out[256];
	sprintf(out,"\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n|%.2f\t%.2f\t%.2f\t%.2f|\n",
		m[0],m[1],m[2],m[3],m[4],m[5],m[6],m[7],m[8],m[9],m[10],m[11],m[12],m[13],m[14],m[15]);
	return string(out);
}

//--------------------------------------------------------
reMatrix4 reMatrix4::operator +(const reMatrix4 &mat)const{
	reMatrix4 out;
	FOR_i(16)
		out[i] = m[i]+mat.m[i];
	return out;
}

//--------------------------------------------------------
reMatrix4 reMatrix4::operator -(const reMatrix4 &mat)const{
	reMatrix4 out;
	FOR_i(16)
		out[i] = m[i]-mat.m[i];
	return out;
}

//--------------------------------------------------------
reMatrix4 reMatrix4::operator *(float scalar)const{
	reMatrix4 out;
	FOR_i(16)
		out[i] = m[i] * scalar;
	return out;
}

//--------------------------------------------------------
reMatrix4 reMatrix4::operator *(const reMatrix4 &mat)const{
	reMatrix4 out;

	int row=0;
	FOR_i(4){
		FOR_j(4){
			out.m[row+j] = m[row]*mat.m[j]+m[row+1]*mat.m[4+j] 
						 + m[row+2]*mat.m[8+j] + m[row+3]*mat.m[12+j];
		}
		row+=4;
	}
	return out;
}
//--------------------------------------------------------
void reMatrix4::operator *=(const reMatrix4 &mat){
	float temp[16];

	int row=0;
	FOR_i(4){
		FOR_j(4){
			temp[row+j] = m[row]*mat.m[j]+m[row+1]*mat.m[4+j] 
						 + m[row+2]*mat.m[8+j] + m[row+3]*mat.m[12+j];
		}
		row+=4;
	}
	memcpy(this, temp, sizeof(float)*16);
}
//--------------------------------------------------------
reMatrix4 operator*	(float scalar, const reMatrix4& mat){
	reMatrix4 out;
	FOR_i(16)
		out[i] = mat.m[i] * scalar;
	return out;
}

//--------------------------------------------------------
reMatrix4& reMatrix4::operator +=(const reMatrix4 &mat){
	FOR_i(16)
		m[i]+=mat.m[i];
	return *this;
}

//--------------------------------------------------------
reMatrix4& reMatrix4::operator -=(const reMatrix4 &mat){
	FOR_i(16)
		m[i]-=mat.m[i];
	return *this;
}

//--------------------------------------------------------
bool reMatrix4::operator ==(const reMatrix4 &mat)const{
	FOR_i(16)
		if (m[i]!=mat.m[i])
			return false;
	return true;
}

//--------------------------------------------------------
bool reMatrix4::operator !=(const reMatrix4 &mat)const{
	FOR_i(16)
		if (m[i]!=mat.m[i])
			return true;
	return false;
}

//--------------------------------------------------------
float& reMatrix4::operator[](int idx){
	return m[idx];
}

//--------------------------------------------------------
void reMatrix4::SetIdentity(){
	memset((void*)m, 0, sizeof(m));
	m[0] = m[5]= m[10] = m[15] = 1.0f;
}

//--------------------------------------------------------
reMatrix4 reMatrix4::Transpose(){
	reMatrix4 out;
	FOR_i(4){
		FOR_j(4){
			out.m[j*4+i] = m[i*4+j];
		}
	}
	return out;
}

//--------------------------------------------------------
float reMatrix4::Determinant(){
	return 	m[0]*(m[4]*m[8] - m[5]*m[7])
		- 	m[1]*(m[3]*m[8] - m[5]*m[6])
		+	m[2]*(m[3]*m[7] - m[4]*m[6]);
}

///////////////////////////////////////////////////////////////////////////////
// rePoint4
///////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------
rePoint4::rePoint4(){
	memset((void*)v, 0, 3*sizeof(float));
	w=1.0f;
}

//--------------------------------------------------------
rePoint4::rePoint4(const float elems[4]){
	memcpy((void*)v, (void*)elems, sizeof(v));
}

//--------------------------------------------------------
rePoint4::rePoint4(const rePoint4& copy){
	memcpy((void*)v, (void*)copy.v, sizeof(v));
}

//--------------------------------------------------------
reVector3 rePoint4::operator +(const rePoint4 &pt) const{
	reVector3 out;
	out.x = x + pt.x;
	out.y = y + pt.y;
	out.z = z + pt.z;
	return out;
}

//--------------------------------------------------------
reVector3 rePoint4::operator -(const rePoint4 &pt) const{
	reVector3 out;
	out.x = x - pt.x;
	out.y = y - pt.y;
	out.z = z - pt.z;
	return out;
}

//--------------------------------------------------------
rePoint4 rePoint4::operator +(const reVector3 &vec) const{
	rePoint4 out;
	out.x = x + vec.x;
	out.y = y + vec.y;
	out.z = z + vec.z;
	out.w = 1.0f;
	return out;
}

//--------------------------------------------------------
rePoint4 rePoint4::operator -(const reVector3 &vec) const{
	rePoint4 out;
	out.x = x - vec.x;
	out.y = y - vec.y;
	out.z = z - vec.z;
	out.w = 1.0f;
	return out;
}

//--------------------------------------------------------
rePoint4 rePoint4::operator *(float scalar) const{
	rePoint4 out;
	out.x = x * scalar;
	out.y = y * scalar;
	out.z = z * scalar;
	out.w = 1.0f;
	return out;
}

//--------------------------------------------------------
rePoint4 operator *(float scalar, rePoint4& pt){
	rePoint4 out;
	out.x = pt.x * scalar;
	out.y = pt.y * scalar;
	out.z = pt.z * scalar;
	out.w = 1.0f;
	return out;
}

//--------------------------------------------------------
rePoint4& rePoint4::operator +=(const rePoint4 &pt){
	x += pt.x;
	y += pt.y;
	z += pt.z;
	return *this;
}

//--------------------------------------------------------
rePoint4& rePoint4::operator -=(const rePoint4 &pt){
	x -= pt.x;
	y -= pt.y;
	z -= pt.z;
	return *this;
}

//--------------------------------------------------------
rePoint4& rePoint4::operator +=(const reVector3 &vec) {
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}

//--------------------------------------------------------
rePoint4& rePoint4::operator -=(const reVector3 &vec) {
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}

//--------------------------------------------------------
bool rePoint4::operator ==(const rePoint4& pt) const{
	if (x!=pt.x || y!=pt.y || z!=pt.z)
		return false;
	return true;
}

//--------------------------------------------------------
bool rePoint4::operator !=(const rePoint4& pt) const{
	if (x!=pt.x || y!=pt.y || z!=pt.z)
		return true;
	return false;
}

//--------------------------------------------------------
float& rePoint4::operator [](int idx){
	return v[idx];
}

//--------------------------------------------------------
string rePoint4::str(){
	char out[64];
	sprintf(out,"[%.2f, %.2f, %.2f, %.2f]",x,y,z,w);
	return string(out);
}

///////////////////////////////////////////////////////////////////////////////
// reVector3
///////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------
reVector3::reVector3(){
	memset((void*)v, 0, 3*sizeof(float));
}

//--------------------------------------------------------
reVector3::reVector3(float _x, float _y, float _z){
	x=_x; y=_y; z=_z;
}

//--------------------------------------------------------
reVector3::reVector3(const float elems[3]){
	memcpy((void*)v, (void*)elems, sizeof(v));
}

//--------------------------------------------------------
reVector3::reVector3(const reVector3& copy){
	memcpy((void*)v, (void*)copy.v, sizeof(v));
}

//--------------------------------------------------------
reVector3 reVector3::operator +(const reVector3 &vec) const{
	reVector3 out;
	out.x = x + vec.x;
	out.y = y + vec.y;
	out.z = z + vec.z;
	return out;
}

//--------------------------------------------------------
reVector3 reVector3::operator -(const reVector3 &vec) const{
	reVector3 out;
	out.x = x - vec.x;
	out.y = y - vec.y;
	out.z = z - vec.z;
	return out;
}

//--------------------------------------------------------
reVector3 reVector3::operator -() const{
	reVector3 out;
	out.x = - x;
	out.y = - y;
	out.z = - z;
	return out;
}

//--------------------------------------------------------
reVector3 reVector3::operator *(float scalar) const{
	reVector3 out;
	out.x = x * scalar;
	out.y = y * scalar;
	out.z = z * scalar;
	return out;
}

//--------------------------------------------------------
reVector3 operator *(float scalar, reVector3& vec){
	reVector3 out;
	out.x = vec.x * scalar;
	out.y = vec.y * scalar;
	out.z = vec.z * scalar;
	return out;
}

//--------------------------------------------------------
reVector3& reVector3::operator +=(const reVector3 &vec){
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}

//--------------------------------------------------------
reVector3& reVector3::operator -=(const reVector3 &vec){
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}

//--------------------------------------------------------
reVector3& reVector3::operator *=(float scalar){
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

//--------------------------------------------------------
bool reVector3::operator ==(const reVector3& vec) const{
	if (x!=vec.x || y!=vec.y || z!=vec.z)
		return false;
	return true;
}

//--------------------------------------------------------
bool reVector3::operator !=(const reVector3& vec) const{
	if (x!=vec.x || y!=vec.y || z!=vec.z)
		return true;
	return false;
}

//--------------------------------------------------------
float& reVector3::operator [](int idx){
	return v[idx];
}

//--------------------------------------------------------
string reVector3::str(){
	char out[64];
	sprintf(out,"<%.2f, %.2f, %.2f>",x,y,z);
	return string(out);
}

//--------------------------------------------------------
float reVector3::Mag()const{
	return sqrt(x*x+y*y+z*z);
}

//--------------------------------------------------------
float reVector3::Mag2()const{
	return x*x+y*y+z*z;
}

//--------------------------------------------------------
float reVector3::Dot(const reVector3 & v) const{
	return x*v.x + y*v.y + z*v.z;
}

//--------------------------------------------------------
reVector3 reVector3::Cross(const reVector3 &v) const{
	return reVector3(y*v.z-z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

//--------------------------------------------------------
void reVector3::Normalize(){
	float magInv = 1.0f/Mag();
	x*=magInv;
	y*=magInv;
	z*=magInv;
}

//--------------------------------------------------------
reVector3 reVector3::GetUnit()const{
	float magInv = 1.0f/Mag();
	return reVector3(x*magInv, y*magInv, z*magInv);
}

/******************************************************************************
 * SPECIAL MATRICES
 ******************************************************************************/

void ProjFrustum	(reMatrix4& in, float left, float right, float bottom, float top, float near, float far){
	reMatrix4 mat4;
	float* mat = in.m;
	// top row
	mat[0] = 2*near / (right - left);
	mat[2] = (right+left) / (right-left);
	// second row
	mat[5] = 2*near / (top - bottom);
	mat[6] = (top+bottom)/(top-bottom);
	// third row
	mat[10]= -(far+near)/(far-near);
	mat[11]= -(2*far*near)/(far-near);
	// last row
	mat[14]= -1.0f;
	mat[15]= .0f;

	in*=mat4;
}

void ProjPersp		(reMatrix4& in, float fovy, float aspect, float near, float far){
	reMatrix4 mat;
}

void ProjOrtho		(reMatrix4& in, float left, float right, float bottom, float top, float near, float far){
	reMatrix4 mat;
}

void TranslateMatrix (reMatrix4& in, float x, float y, float z){
	reMatrix4 mat;
	mat[3] =x;
	mat[7] =y;
	mat[11]=z;
	in*=mat;
}


