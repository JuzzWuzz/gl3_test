
#ifndef _REMATH_H_
#define _REMATH_H_

/*
 * Some Notes:
 *  - Angles are specified in Radians
 *  - Matrices are stored in column major ordering
 *  - Transformations should pre-multiply vectors. eg. V2 = M * V1
 *	- The vector4 homogeneous coordinates are not implemented correctly yet, use 
 *	  vector3 instead.
 */

namespace reMath{
	// Forward declarations
	class matrix4;
	class vector4;
	class vector3;

	/******************************************************************************
	 * TRANSFORMS AND MATH
	 ******************************************************************************/

	/******************************************************************************
	 * vector4
	 * A Class to represent homogeneous coordinates - not fully functional
	 ******************************************************************************/
	class vector4{
	public:
		vector4();
		vector4(const vector4& copy);
		vector4(const float elems[4]);

		vector3		operator+		(const vector4& vec) const;
		vector3		operator-		(const vector4& vec) const;
		vector4		operator+		(const vector3& vec) const;
		vector4		operator-		(const vector3& vec) const;
		vector4		operator*		(float scalar) const;
		friend vector4 operator*		(float scalar, const vector4& pt);

		vector4&	operator+=		(const vector4& vec);
		vector4&	operator-=		(const vector4& vec);
		vector4&	operator+=		(const vector3& vec);
		vector4&	operator-=		(const vector3& vec);

		bool		operator==		(const vector4& vec) const;
		bool		operator!=		(const vector4& vec) const;

		float&		operator[]		(int idx) ;
		string		str				(void);


	public:
		union{
			struct {float x,y,z,w;};
			float v[4];
		};
	};
	vector4 operator*	(float scalar, const vector4& pt);

	/******************************************************************************
	 * vector3
	 * A class to handle 3D vectors and their operations
	 ******************************************************************************/
	class vector3{
	public:
		vector3();
		vector3(float x, float y, float z);
		vector3(const vector3& copy);
		vector3(const float elems[3]);

		vector3		operator+		(const vector3& vec) const;
		vector3		operator-		(const vector3& vec) const;
		vector3		operator-		() const;
		vector3		operator*		(float scalar) const;
		friend vector3 operator*	(float scalar, const vector3& vec);

		vector3&	operator+=		(const vector3& vec);
		vector3&	operator-=		(const vector3& vec);
		vector3&	operator*=		(float scalar);

		bool		operator==		(const vector3& vec) const;
		bool		operator!=		(const vector3& vec) const;

		float&		operator[]		(int idx) ;
		string		str				(void);

		float		Mag				(void) const;
		float		Mag2			(void) const;

		float		Dot				(const vector3&)const;
		vector3		Cross			(const vector3&)const;

		void		Normalize		(void);
		vector3		GetUnit			(void) const;


	public:
		union{
			struct {float x,y,z;};
			float v[3];
		};
	};
	vector3 operator*	(float scalar, const vector3& vec);

	/******************************************************************************
	 * matrix4
	 * A class to represent 4x4 matrices that are often used in vector
	 * transformations. Note column-major indices
	 ******************************************************************************/
	class matrix4{
	public:
		matrix4();
		matrix4(const matrix4& copy);
		matrix4(const float elems[16]);

		matrix4		operator+		(const matrix4& mat) const;
		matrix4		operator-		(const matrix4& mat) const;
		matrix4		operator*		(float scalar) const;
		friend matrix4 operator*	(float scalar, const matrix4& mat);
		matrix4		operator*		(const matrix4& mat) const;
		vector4		operator*		(const vector4& vec) const;
		vector3		operator*		(const vector3& vec) const;
		void		operator*=		(const matrix4& mat);
		matrix4&	operator+=		(const matrix4& mat);
		matrix4&	operator-=		(const matrix4& mat);

		bool		operator==		(const matrix4& mat) const;
		bool		operator!=		(const matrix4& mat) const;

		float&		operator[]		(int idx) ;

		string		str				(void);
		
		void		SetIdentity		(void);
		matrix4		Transpose		(void);
		float		Determinant		(void);

	public:
		float	m[16];
	};
	matrix4 operator*	(float scalar, const matrix4&);



	/******************************************************************************
	 * Transform Matrices
	 ******************************************************************************/

	matrix4 frustum_proj		(float left, float right, float bottom, float top, float near, float far);
	matrix4 perspective_proj	(float fovy, float aspect, float near, float far);
	matrix4 ortho_proj			(float left, float right, float bottom, float top, float near, float far);
	matrix4 ortho2d_proj		(float left, float right, float bottom, float top);

	matrix4 translate_tr	(float x, float y, float z);
	matrix4 rotate_tr		(float angle, float x, float y, float z);
	matrix4 scale_tr		(float x, float y, float z);
}
#endif

