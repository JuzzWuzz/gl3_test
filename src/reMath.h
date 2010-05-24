
#ifndef _REMATH_H_
#define _REMATH_H_

// Forward declarations
class reMatrix4;
class rePoint4;
class reVector3;

/******************************************************************************
 * TRANSFORMS AND MATH
 ******************************************************************************/

/******************************************************************************
 * rePoint4
 * A Class to represent homogeneous coordinates
 ******************************************************************************/
class rePoint4{
public:
	rePoint4();
	rePoint4(const rePoint4& copy);
	rePoint4(const float elems[4]);

	reVector3		operator+		(const rePoint4& pt) const;
	reVector3		operator-		(const rePoint4& pt) const;
	rePoint4		operator+		(const reVector3& vec) const;
	rePoint4		operator-		(const reVector3& vec) const;
	rePoint4		operator*		(float scalar) const;
	friend rePoint4 operator*		(float scalar, const rePoint4& pt);

	rePoint4&		operator+=		(const rePoint4& pt);
	rePoint4&		operator-=		(const rePoint4& pt);
	rePoint4&		operator+=		(const reVector3& pt);
	rePoint4&		operator-=		(const reVector3& pt);

	bool			operator==		(const rePoint4& pt) const;
	bool			operator!=		(const rePoint4& pt) const;

	float&			operator[]		(int idx) ;
	string			str				(void);


public:
	union{
		struct {float x,y,z,w;};
		float v[4];
	};
};
rePoint4 operator*	(float scalar, const rePoint4& pt);

/******************************************************************************
 * reVector3
 * A class to handle 3D vectors and their operations
 ******************************************************************************/
class reVector3{
public:
	reVector3();
	reVector3(float x, float y, float z);
	reVector3(const reVector3& copy);
	reVector3(const float elems[3]);

	reVector3		operator+		(const reVector3& vec) const;
	reVector3		operator-		(const reVector3& vec) const;
	reVector3		operator-		() const;
	reVector3		operator*		(float scalar) const;
	friend reVector3 operator*	(float scalar, const reVector3& vec);

	reVector3&	operator+=		(const reVector3& vec);
	reVector3&	operator-=		(const reVector3& vec);
	reVector3&	operator*=		(float scalar);

	bool			operator==		(const reVector3& vec) const;
	bool			operator!=		(const reVector3& vec) const;

	float&			operator[]		(int idx) ;
	string			str				(void);

	float			Mag				(void) const;
	float			Mag2			(void) const;

	float			Dot				(const reVector3&)const;
	reVector3		Cross			(const reVector3&)const;

	void			Normalize		(void);
	reVector3		GetUnit			(void) const;


public:
	union{
		struct {float x,y,z;};
		float v[3];
	};
};
reVector3 operator*	(float scalar, const reVector3& vec);

/******************************************************************************
 * reMatrix4
 * A class to represent 4x4 matrices that are often used in spatial 
 * transformations. Note column-major indices
 ******************************************************************************/
class reMatrix4{
public:
	reMatrix4();
	reMatrix4(const reMatrix4& copy);
	reMatrix4(const float elems[16]);

	reMatrix4		operator+		(const reMatrix4& mat) const;
	reMatrix4		operator-		(const reMatrix4& mat) const;
	reMatrix4		operator*		(float scalar) const;
	friend reMatrix4 operator*	(float scalar, const reMatrix4& mat);
	reMatrix4		operator*		(const reMatrix4& mat) const;
	void			operator*=		(const reMatrix4& mat);
	reMatrix4&		operator+=		(const reMatrix4& mat);
	reMatrix4&		operator-=		(const reMatrix4& mat);

	bool			operator==		(const reMatrix4& mat) const;
	bool			operator!=		(const reMatrix4& mat) const;

	float&			operator[]		(int idx) ;

	string			str				(void);
	
	void			SetIdentity		(void);
	reMatrix4		Transpose		(void);
	float			Determinant		(void);

public:
	float	m[16];
};
reMatrix4 operator*	(float scalar, const reMatrix4&);



/******************************************************************************
 * SPECIAL MATRICES
 ******************************************************************************/

void ProjFrustum	(reMatrix4& in, float left, float right, float bottom, float top, float near, float far);
void ProjPersp		(reMatrix4& in, float fovy, float aspect, float near, float far);
void ProjOrtho		(reMatrix4& in, float left, float right, float bottom, float top, float near, float far);

void TranslateMatrix(reMatrix4& in, float x, float y, float z);

#endif
