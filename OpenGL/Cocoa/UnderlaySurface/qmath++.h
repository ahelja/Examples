const double PI = 3.1415926515;

/*	
	Vector utility class
	--------------------
*/

struct Vec : TQ3Vector3D
{
	void Set(const float x, const float y, const float z = 0)
	{
		this->x = x, this->y = y; this->z = z;
	}

	void operator += (const Vec& v)
	{
		x += v.x, y += v.y, z += v.z;
	}
	
	void operator -= (const Vec& v)
	{
		x -= v.x, y -= v.y, z -= v.z;
	}
	
	void operator *= (const float d)
	{
		x *= d, y *= d, z *= d;
	}
	
	void operator *= (const Vec& v)
	{
		x *= v.x, y *= v.y, z *= v.z;
	}
	
	void operator /= (const float d)
	{
		float d_inv = 1 / d;

		x *= d_inv; y *= d_inv, z *= d_inv;
	}
	
	float& operator [] (int i)
	{
		switch(i)
		{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			default: goto error;
		}

	error:

		return x;
	}
	
	
	float operator[] (int i) const
	{
		switch(i)
		{
			case 0: return x;
			case 1: return y;
			case 2: return z;
			default: goto error;
		}

	error:
		
		return 0;
	}
			
	Vec operator - () const
	{
		return Vec::New(-x,-y,-z);
	}
	
	float rz()
	{
		if (fabs(x) + fabs(y) < 0.001)
			return 0;

		return -atan2(x,y);
	}
	
	Vec PointOnUnitSphere(const Vec& p, float radius) const
	{
		Vec tmp = p;
		
		tmp -= *this;
		
		tmp *= 1/radius;
		
		tmp.y *= -1;

		float length2 = Length2(tmp);
		
		if (length2 < 1.0)
			tmp.z = sqrt(1.0 - length2);
		else 
		{
			tmp *= 1/(sqrt(length2));
			tmp.z = 0;
		}
		
		return tmp;
	}

	static Vec New(float x, float y, float z = 0)
	{
		Vec v;
		v.Set(x,y,z);
		return v;
	}

	static float dot(const Vec& a, const Vec& b)
	{
		return a.x*b.x + a.y*b.y + a.z*b.z;
	}
	
	static Vec cross(const Vec& a, const Vec& b)
	{
		return Vec::New(a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y);
	}
			
	static float Length(const Vec& v)
	{
		return sqrt(Length2(v));
	}
	
	static float Length2(const Vec& v)
	{
		return v.x*v.x + v.y*v.y +v.z*v.z;
	}

	static Vec Normalize(const Vec& v)
	{
		float inv_length = 1 / Vec::Length(v);
		return Vec::New(v.x*inv_length, v.y*inv_length, v.z*inv_length);
	}
};


inline Vec operator + (const Vec& a, const Vec& b)
{
	return Vec::New(a.x+b.x, a.y+b.y, a.z+b.z);
}

inline Vec operator - (const Vec& a, const Vec& b)
{
	return Vec::New(a.x-b.x, a.y-b.y, a.z-b.z);
}

inline Vec operator * (const Vec& a, const float d)
{
	return Vec::New(a.x*d, a.y*d, a.z*d);
}

inline Vec operator * (const float d, const Vec& a)
{
	return a*d;
}

inline Vec operator / (const Vec& a, const float d)
{
	return Vec::New(a.x/d, a.y/d, a.z/d);
}

inline Vec operator * (const Vec& a, const Vec& b)
{
	return Vec::New(a.x*b.x, a.y*b.y, a.z*b.z);
}

inline Vec reflect(const Vec& a, const Vec& b)
{
	return 2.0f * (Vec::dot(b,a)*b) - a;
}

extern const Vec x_axis, y_axis, z_axis;
extern const Vec base_axes[3];
extern const Vec zero_vec, dir_vec, up_vec;

typedef Vec Pos;


/*	------
	Matrix
	------
*/

struct Matrix : public TQ3Matrix4x4
{
	static Matrix New()
	{
		Matrix m;
		Q3Matrix4x4_SetIdentity(&m);
		return m;
	}
};

/*	----------
	Quaternion
	----------
*/

struct Quat : public TQ3Quaternion
{
	static Quat New()
	{
		Quat q;
		return Quat::Reset(q);
	}
	
	static Quat New(const Vec& v0, const Vec& v1)
	{
		Quat q;
		Q3Quaternion_SetRotateVectorToVector(&q, &v0, &v1);
		return q;
	}
	
	static Quat New(const Vec& axis, const float angle)
	{
		Quat q;
		Q3Quaternion_SetRotateAboutAxis(&q, &axis, angle);
		return q;
	}

	static Quat Reset(Quat& q)
	{
		Q3Quaternion_SetIdentity(&q);
		return q;
	}		

	Matrix GetMatrix() const
	{
		Matrix mat;
		
		Q3Matrix4x4_SetQuaternion(&mat, this);

		return mat;
	}
	
	void GetAxisAngle(Vec& axis, double& angle) const
	{
	    double length2 = x*x + y*y + z*z;
	    
    	if (length2 < FLT_EPSILON)
    	{
 	      	axis = y_axis;
	       	angle = 0;
   		}
		else
		{        	
        	angle = 2.0*acos(w);
        	
        	double invlen = 1.0/sqrt(length2);
        	
        	axis.x = x*invlen;
        	axis.y = y*invlen;
        	axis.z = z*invlen;
    	}
	}	

	static Vec GetEulersYXZ(const Quat& q) 
	{
		Vec rot;

		Matrix mat = q.GetMatrix();
		
		rot.x = asin(-mat.value[2][1]);
		
		if (rot.x < PI/2)
		{
			if (rot.x > -PI/2)
			{
				rot.y = atan2(mat.value[2][0], mat.value[2][2]);
				rot.z = atan2(mat.value[0][1], mat.value[1][1]);
			}
			else
			{
				rot.y = -atan2(-mat.value[1][0], mat.value[0][0]);
				rot.z = 0;
			}
		}
		else
		{
			rot.y = atan2(mat.value[1][0], mat.value[0][0]);
			rot.z = 0;
		}
		
		return rot;
	}		

	static Vec GetEulersZXY(Quat& q)
	{
		Vec rot;

		Matrix mat = q.GetMatrix();
		
		rot.x = asin(mat.value[1][2]);
		
		if (rot.x < PI/2)
		{
			if (rot.x > -PI/2)
			{
				rot.z = atan2(-mat.value[1][0], mat.value[1][1]);
				rot.y = atan2(-mat.value[0][2], mat.value[2][2]);
			}
			else
			{
				rot.z = -atan2(mat.value[2][0], mat.value[0][0]);
				rot.y = 0;
			}
		}
		else
		{
			rot.z = atan2(mat.value[2][0], mat.value[0][0]);
			rot.y = 0;
		}
		
		return rot;
	}		

	Quat operator - () const
	{
		Quat tmp;
		
		Q3Quaternion_Invert(this, &tmp);
		
		return tmp;
	}

	Vec operator * (const Vec& vec) const
	{
		Vec tmp;
		
		Q3Vector3D_TransformQuaternion(&vec, this, &tmp);
		
		return tmp;
	}
	
	Quat operator * (const Quat& q2) const
	{
		Quat tmp = *this;
		
		return tmp *= q2;
	}

	Quat operator *= (const Quat& q2)
	{
		Q3Quaternion_Multiply(this, &q2, this);
	
		return *this;
	}

	Quat RotateTo(const Quat& q1, float t)
	{
		Q3Quaternion_InterpolateLinear(this, &q1, t, this);
		
		return *this;
	}
	
	Quat RotateTo(const Vec& v, float t)
	{
		return RotateTo(Quat::New(dir_vec, v), t);
	}
	
	static void Normalize(Quat& q)
	{
		Q3Quaternion_Normalize(&q, &q);
	}
};

inline const Vec& operator *= (Vec& v, const Quat& q)
{
	return v = q * v;
}


/*	-------
	QMatrix
	-------
*/

struct QMatrix
{
public:

	Quat rot;
	Vec pos, scale;

	/*	----------------------------------
	    Incremental rotation on local axes
	*/ 

	void x_rot(float angle)
	{
		rot *= Quat::New(rot * x_axis, angle);
	}		

	void y_rot(float angle)
	{
		rot *= Quat::New(rot * y_axis, angle);
	}		
		
	void z_rot(float angle)
	{
		rot *= Quat::New(rot * z_axis, angle);
	}
	
	static QMatrix& Reset(QMatrix& q)
	{
		Quat::Reset(q.rot);
		q.pos = Vec::New(0,0,0);
		q.scale = Vec::New(1,1,1);
		
		return q;
	}
	
	static QMatrix New(const Quat& quat, Pos pos = Pos::New(0,0,0), Vec scale = Vec::New(1,1,1))
	{
		QMatrix q;
		q.rot = quat;
		q.pos = pos;
		q.scale = scale;
		
		return q;
	}	

	static QMatrix New(const Vec& v0, const Vec& v1)
	{
		return QMatrix::New(Quat::New(v0,v1));
	}
}; 

enum RGBColors
{
	rgb_clr,
	rgb_blk,
	rgb_red,
	rgb_grn,
	rgb_blu,
	rgb_yel,
	rgb_cyn,
	rgb_mag,
	rgb_wht,
	NumRGBAColors
};

typedef GLfloat RGBASpec[4];

const RGBASpec rgba_specs[NumRGBAColors] =
{
	{ 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0 },
	{ 1.0, 0.0, 0.0, 1.0 },
	{ 0.0, 1.0, 0.0, 1.0 },
	{ 0.0, 0.0, 1.0, 1.0 },
	{ 1.0, 1.0, 0.0, 1.0 },
	{ 0.0, 1.0, 1.0, 1.0 },
	{ 1.0, 0.0, 1.0, 1.0 },
	{ 1.0, 1.0, 1.0, 1.0 },
};

struct RGBA
{
	RGBASpec spec;
	
	RGBA(RGBColors color = rgb_wht)
	{
		memcpy(&spec, &rgba_specs[color], sizeof(RGBASpec));
	}
	
	RGBA(float r, float g, float b, float a = 1.0)
	{
		spec[0] = r, spec[1] = g, spec[2] = b, spec[3] = a;
	}
};



class MatrixFrame
{
	GLuint stack;

public:
	
	bool pushed;

	MatrixFrame(GLuint matrix_stack = GL_MODELVIEW)
	{
		glMatrixMode(stack = matrix_stack);

		glPushMatrix();
		
		pushed = true;
	}
	
	~MatrixFrame()
	{
		UnFrame();
	}
	
	void UnFrame()
	{
		if (!pushed)
			return;
			
		if (stack == GL_PROJECTION)
			glMatrixMode(GL_PROJECTION);

		glPopMatrix();

		if (stack == GL_PROJECTION)
			glMatrixMode(GL_MODELVIEW);

		pushed = false;
	}
	
	void Reset()
	{
		glLoadIdentity();
	}	

	Matrix GetMatrix()
	{
		Matrix m;
		
		glGetFloatv(stack == GL_MODELVIEW ? GL_MODELVIEW_MATRIX : GL_PROJECTION_MATRIX, (float *) m.value);
		
		return m;
	}	

	void operator <<= (const QMatrix& q)
	{
		*this <<= q.pos;

		*this <<= q.rot;

		glScalef(q.scale.x, q.scale.y, q.scale.z);
	}
	
	void operator <<= (const Vec& vec)
	{
		glTranslatef(vec.x, vec.y, vec.z);
	}
	
	void operator <<= (const Quat& rot)
	{
		Matrix mat = rot.GetMatrix();

		*this <<= mat;
	}

	void operator <<= (const Matrix& mat)
	{
		glMultMatrixf((float *) mat.value);
	}
};

class ProjectionFrame : public MatrixFrame
{
public:

	ProjectionFrame() : MatrixFrame(GL_PROJECTION) { }
};
