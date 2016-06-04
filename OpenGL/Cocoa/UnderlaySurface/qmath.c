#include <math.h>
#include "qmath.h"

/******************************************************************************
 **																			 **
 ** 	Copyright (C) 1992-2001 Apple Computer, Inc.  All rights reserved.	 **
 ** 																		 **
 *****************************************************************************/

#define Assert(x)

#define Fabs(value) fabs(value)
#define Sqrt(value) sqrt(value)
#define Cos(theta) cos(theta)
#define Sin(theta) sin(theta)
#define Acos(theta) acos(theta)


typedef enum
{
	VectorComponent_X,
	VectorComponent_Y,
	VectorComponent_Z
}  VectorComponent;

/******************************************************************************
 **																			 **
 **					Internal Vector Support Functions						 **
 **																			 **
 *****************************************************************************/

/*===========================================================================*\
 *
 *	Routine:	Vector3D_Scale()
 *
 *	Comments:	
 *
\*===========================================================================*/

static TQ3Vector3D *Vector3D_Scale(
	const TQ3Vector3D	*vector3D, 
	float				scalar,
	TQ3Vector3D			*result)
{
	Assert(vector3D != NULL);
	Assert(result != NULL);

	result->x = vector3D->x * scalar;
	result->y = vector3D->y * scalar;
	result->z = vector3D->z * scalar;
	
	return (result);	
}

/*===========================================================================*\
 *
 *	Routine:	Vector3D_Normalize()
 *
 *	Comments:	
 *
\*===========================================================================*/

static TQ3Vector3D *Vector3D_Normalize(
	const TQ3Vector3D	*vector3D,
	TQ3Vector3D			*result)
{
	register float	length, oneOverLength;
	
	Assert(vector3D != NULL);
	Assert(result != NULL);

	length = (vector3D->x * vector3D->x) +
			 (vector3D->y * vector3D->y) +
			 (vector3D->z * vector3D->z);			/* sum the squares of the vector */

	length = Sqrt(length);					/* dist = sqrt(sum) */
				   		 
	/*
     *  Check for zero-length vector
     */

    if (length < FLT_EPSILON)
       	*result = *vector3D;
    else
    {
    	oneOverLength = 1.0f/length;
	
		result->x = vector3D->x * oneOverLength;
		result->y = vector3D->y * oneOverLength;
		result->z = vector3D->z * oneOverLength;
	}
	
	return (result);
}

/*===========================================================================*\
 *
 *	Routine:	Vector3D_Normalize_Precise()
 *
 *	Comments:	
 *
\*===========================================================================*/

static TQ3Vector3D *Vector3D_Normalize_Precise(
	const TQ3Vector3D	*vector3D,
	TQ3Vector3D			*result)
{
	register float	length, oneOverLength;

	Assert(vector3D != NULL);
	Assert(result != NULL);

	length = (vector3D->x * vector3D->x) +
			 (vector3D->y * vector3D->y) +
			 (vector3D->z * vector3D->z);

	length = Sqrt(length);
				   		 
	/*
     *  Check for zero-length vector
     */
    if (length < FLT_EPSILON)
       	*result = *vector3D;
    else
    {
    	oneOverLength = 1.0f/length;
	
		result->x = vector3D->x * oneOverLength;
		result->y = vector3D->y * oneOverLength;
		result->z = vector3D->z * oneOverLength;
	}
	
	return (result);
}

/*===========================================================================*\
 *
 *	Routine:	Vector3D_Add()
 *
 *	Comments:	
 *
\*===========================================================================*/

static TQ3Vector3D *Vector3D_Add(
	const TQ3Vector3D	*v1, 
	const TQ3Vector3D	*v2, 
	TQ3Vector3D			*result)
{
	Assert(v1 != NULL);
	Assert(v2 != NULL);
	Assert(result != NULL);

	result->x = v1->x + v2->x;
	result->y = v1->y + v2->y;
	result->z = v1->z + v2->z;

	return (result);		
}

/*===========================================================================*\
 *
 *	Routine:	Vector3D_Subtract()
 *
 *	Comments:	
 *
\*===========================================================================*/

static TQ3Vector3D *Vector3D_Subtract(
	const TQ3Vector3D	*v1, 
	const TQ3Vector3D	*v2,
	TQ3Vector3D			*result)
{
	Assert(v1 != NULL);
	Assert(v2 != NULL);
	Assert(result != NULL);

	result->x = v1->x - v2->x;
	result->y = v1->y - v2->y;
	result->z = v1->z - v2->z;

	return (result);		
}

/*===========================================================================*\
 *
 *	Routine:	Vector3D_Cross()
 *
 *	Comments:	
 *
\*===========================================================================*/

static TQ3Vector3D *Vector3D_Cross(
	const TQ3Vector3D	*v1, 
	const TQ3Vector3D	*v2,
	TQ3Vector3D			*result)
{
	TQ3Vector3D					s1, s2;
	register const TQ3Vector3D	*s1Ptr, *s2Ptr;
	
	Assert(v1 != NULL);
	Assert(v2 != NULL);
	Assert(result != NULL);

	/*
	 *  Check for argument-bashing
	 */
	if (v1 == result) {
		s1 = *v1;
		s1Ptr = &s1;
	} else {
		s1Ptr = v1;
	}	
	
	if (v2 == result) {
		s2 = *v2;
		s2Ptr = &s2;
	} else {
		s2Ptr = v2;
	}

	result->x =   s1Ptr->y * s2Ptr->z - s2Ptr->y * s1Ptr->z;
	result->y = -(s1Ptr->x * s2Ptr->z - s2Ptr->x * s1Ptr->z);
	result->z =   s1Ptr->x * s2Ptr->y - s2Ptr->x * s1Ptr->y;
	
	return (result);
}

/*===========================================================================*\
 *
 *	Routine:	Vector3D_Dot()
 *
 *	Comments:	
 *
\*===========================================================================*/

static float Vector3D_Dot(
	const TQ3Vector3D	*v1, 
	const TQ3Vector3D	*v2)
{
	Assert(v1 != NULL);
	Assert(v2 != NULL);

	return (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
}

/*===========================================================================*\
 *
 *	Routine:	OrthogonalVector()
 *
 *	Comments:	Returns a vector that is orthogonal to src and is of unit 
 *				length.
 *				Src should be normalized.
 *				Note that this isnÕt fully constrained -- it has many 
 *				solutions.
 *
\*===========================================================================*/

static void Vector3D_OrthogonalVector(
	const TQ3Vector3D	*src, 
	TQ3Vector3D			*dst)
{
	float				best;
	float				temp;
	VectorComponent	component;
	TQ3Vector3D			orth;
	TQ3Vector3D			proj;
	
	Assert(src != NULL);
	Assert(dst != NULL);
	
	/* 
	 *  Find the component that is most orthogonal to the vector 
	 */
	best = Fabs(src->x);
	component = VectorComponent_X;
	
	temp = Fabs(src->y);
	if (best > temp) {
		best = temp;
		component = VectorComponent_Y;
	}
	
	temp = Fabs(src->z);
	if (best > temp) {
		best = temp;
		component = VectorComponent_Z;
	}
	
	orth.x = orth.y = orth.z = 0.0;
	switch (component) {
		case VectorComponent_X : {
			orth.x = 1.0;
			break;
		}
		case VectorComponent_Y : {
			orth.y = 1.0;
			break;
		}
		case VectorComponent_Z : {
			orth.z = 1.0;
			break;
		}
	}
	
	/* 
	 *  Find the orthogonal projection onto the vector 
	 */
	proj = *src;
	Vector3D_Scale(&proj, Vector3D_Dot(src, &orth), &proj);
	
	*dst = orth;
	Vector3D_Subtract(&proj, dst, dst);
	
	Vector3D_Normalize(dst, dst);
}

/******************************************************************************
 **																			 **
 **								Matrix API Calls							 **
 **																			 **
 *****************************************************************************/
 
/*===========================================================================*\
 *
 *	Routine:	Q3Matrix4x4_SetIdentity()
 *
 *	Comments:	
 *
\*===========================================================================*/

TQ3Matrix4x4 *Q3Matrix4x4_SetIdentity(
	TQ3Matrix4x4		*matrix4x4)
{
	register float	*f;
	register float 	f0	= 0.0F;
	register float	f1	= 1.0F;
	
	Assert(matrix4x4 != NULL);

	f = &(matrix4x4->value[0][0]);
	
	*f++ = f1;	*f++ = f0;	*f++ = f0;	*f++ = f0;
	*f++ = f0;	*f++ = f1;	*f++ = f0;	*f++ = f0;
	*f++ = f0;	*f++ = f0;	*f++ = f1;	*f++ = f0;
	*f++ = f0;	*f++ = f0;	*f++ = f0;	*f++ = f1;
	
	return (matrix4x4);
}

/******************************************************************************
 **																			 **
 **							Quaternion API Calls							 **
 **																			 **
 *****************************************************************************/

/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_SetIdentity()
 *
 *	Comments:	
 *
\*===========================================================================*/

TQ3Quaternion *Q3Quaternion_SetIdentity(
	TQ3Quaternion	*quaternion)
{
	Assert(quaternion != NULL);

	quaternion->w = 1.0;
	quaternion->x = 0.0;
	quaternion->y = 0.0;
	quaternion->z = 0.0;
	
	return (quaternion);
}

/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_Invert()
 *
 *	Comments:	
 *
\*===========================================================================*/

TQ3Quaternion *Q3Quaternion_Invert(
	const TQ3Quaternion 	*quaternion,
	TQ3Quaternion			*result)
{
	Assert(quaternion != NULL);
	Assert(result != NULL);

	result->w =  quaternion->w;
	result->x = -quaternion->x;
	result->y = -quaternion->y;
	result->z = -quaternion->z;
	
	return (result);
}


/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_Normalize()
 *
 *	Comments:	
 *
\*===========================================================================*/

TQ3Quaternion *Q3Quaternion_Normalize(
	const TQ3Quaternion	*quaternion,
	TQ3Quaternion		*result)
{
	float		scale;
	
	Assert(quaternion != NULL);
	Assert(result != NULL);

	scale = quaternion->w * quaternion->w + 
			quaternion->x * quaternion->x + 
			quaternion->y * quaternion->y + 
			quaternion->z * quaternion->z;
	
	if (scale < FLT_EPSILON) {
		*result = *quaternion;
		return (result);
	}
	
	if (1.0f - FLT_EPSILON < scale && scale < 1.0f + FLT_EPSILON) {
		*result = *quaternion;
		return (result);
	}
	
	scale = 1.0f / Sqrt(scale);
	
	result->w = quaternion->w * scale;
	result->x = quaternion->x * scale;
	result->y = quaternion->y * scale;
	result->z = quaternion->z * scale;
	
	return (result);
}

/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_Multiply()
 *
 *	Comments:	
 *
\*===========================================================================*/

TQ3Quaternion *Q3Quaternion_Multiply(
	const TQ3Quaternion	*q1, 
	const TQ3Quaternion	*q2,
	TQ3Quaternion		*result)
{
	float	aW;
	float	aX;
	float	aY;
	float	aZ;
	float	bW;
	float	bX;
	float	bY;
	float	bZ;
	
	Assert(q1 != NULL);
	Assert(q2 != NULL);
	Assert(result != NULL);

	aW = q2->w;
	aX = q2->x;
	aY = q2->y;
	aZ = q2->z;
	
	bW = q1->w;
	bX = q1->x;
	bY = q1->y;
	bZ = q1->z;
	
	result->w = aW * bW - aX * bX - aY * bY - aZ * bZ;
	result->x = aW * bX + aX * bW + aY * bZ - aZ * bY;
	result->y = aW * bY + aY * bW + aZ * bX - aX * bZ;
	result->z = aW * bZ + aZ * bW + aX * bY - aY * bX;
	
	return (result);
}

/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_SetRotateAboutAxis()
 *
 *	Comments:	
 *
\*===========================================================================*/

TQ3Quaternion *Q3Quaternion_SetRotateAboutAxis(
	TQ3Quaternion 		*quaternion, 
	const TQ3Vector3D 	*axis, 
	float 				angle)
{
	float	scale;
	
	Assert(quaternion != NULL);
	Assert(axis != NULL);

	angle *= 0.5f;
	scale = Sin(angle);
	
	quaternion->w = Cos(angle);
	quaternion->x = axis->x * scale;
	quaternion->y = axis->y * scale;
	quaternion->z = axis->z * scale;
	
	return (quaternion);
}

/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_SetRotateVectorToVector()
 *
\*===========================================================================*/

TQ3Quaternion *Q3Quaternion_SetRotateVectorToVector(
	TQ3Quaternion 		*quaternion, 
	const TQ3Vector3D 	*v1, 
	const TQ3Vector3D 	*v2)
{
	TQ3Vector3D		half;
	TQ3Vector3D		cross;
	
	Assert(quaternion != NULL);
	Assert(v1 != NULL);
	Assert(v2 != NULL);
	
	/* Find the half-vector */
	half = *v1;
	Vector3D_Add((TQ3Vector3D *)v2, &half, &half);
	
	/* Protect against opposite vectors */
	if (!Vector3D_Normalize_Precise(&half, &half)) {
		Vector3D_OrthogonalVector((TQ3Vector3D *)v2, &half);
	}
	
	/* Find the quaternion */
	Vector3D_Cross(&half, (TQ3Vector3D *)v2, &cross);
	
	quaternion->w = Vector3D_Dot(&half, v2);
	quaternion->x = cross.x;
	quaternion->y = cross.y;
	quaternion->z = cross.z;
	
	return (quaternion);
}


/*===========================================================================*\
 *
 *	Routine:	Q3Matrix4x4_SetQuaternion()
 *
 *	Comments:	For unit quaternions only!
 *
\*===========================================================================*/

TQ3Matrix4x4 *Q3Matrix4x4_SetQuaternion(
	TQ3Matrix4x4 			*matrix, 
	const TQ3Quaternion		*quaternion)
{
	float	w, wz, wy, wx;
	float	x, xz, xy, xx;
	float	y, yz, yy;
	float	z, zz;
	
	Assert(matrix != NULL);
	Assert(quaternion != NULL);
	
	w = quaternion->w;
	x = quaternion->x;
	y = quaternion->y;
	z = quaternion->z;
	
	xx = x * 2.0f;
	yy = y * 2.0f;
	zz = z * 2.0f;
	
	wz = w * zz;
	wy = w * yy;
	wx = w * xx;
	
	xz = x * zz;
	xy = x * yy;
	xx = x * xx;
	
	yz = y * zz;
	yy = y * yy;
	
	zz = z * zz;
	
	matrix->value[0][0] = 1.0f - yy - zz;
	matrix->value[0][1] = xy + wz;
	matrix->value[0][2] = xz - wy;
	matrix->value[0][3] = 0.0;
	
	matrix->value[1][0] = xy - wz;
	matrix->value[1][1] = 1.0f - xx - zz;
	matrix->value[1][2] = yz + wx;
	matrix->value[1][3] = 0.0;
	
	matrix->value[2][0] = xz + wy;
	matrix->value[2][1] = yz - wx;
	matrix->value[2][2] = 1.0f - xx - yy;
	matrix->value[2][3] = 0.0;
	
	matrix->value[3][0] = 0.0;
	matrix->value[3][1] = 0.0;
	matrix->value[3][2] = 0.0;
	matrix->value[3][3] = 1.0;
	
	return (matrix);
}

/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_Interpolate_Fast()
 *
 *	Comments:	Does chord inbetween, and renormalizes to get back to the 
 *				unit 4-sphere
 *
\*===========================================================================*/

TQ3Quaternion *Q3Quaternion_InterpolateFast(
	const TQ3Quaternion 	*q1, 
	const TQ3Quaternion 	*q2, 
	float 					t,
	TQ3Quaternion			*result)
{
	Assert(q1 != NULL);
	Assert(q2 != NULL);
	Assert(result != NULL);
	
	result->w = q1->w + t * (q2->w - q1->w);
	result->x = q1->x + t * (q2->x - q1->x);
	result->y = q1->y + t * (q2->y - q1->y);
	result->z = q1->z + t * (q2->z - q1->z);
	
	return (Q3Quaternion_Normalize(result, result));
}


/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_Dot()
 *
 *	Comments:	
 *
\*===========================================================================*/

float Q3Quaternion_Dot(
	const TQ3Quaternion 	*q1, 
	const TQ3Quaternion 	*q2)
{
	Assert(q1 != NULL);
	Assert(q2 != NULL);

	return (q1->w * q2->w + q1->x * q2->x + q1->y * q2->y + q1->z * q2->z);
}


/*===========================================================================*\
 *
 *	Routine:	Q3Quaternion_InterpolateLinear()
 *
 *	Comments:	Does great arc inbetween on the unit 4-sphere
 *
\*===========================================================================*/

TQ3Quaternion *Q3Quaternion_InterpolateLinear(
	const TQ3Quaternion 	*q1, 
	const TQ3Quaternion 	*q2, 
	float 					t,
	TQ3Quaternion 			*result) 
{
	float		theta;
	float		scale;
	float		s0;
	float		s1;
	
	Assert(q1 != NULL);
	Assert(q2 != NULL);
	Assert(result != NULL);
	
	theta = Q3Quaternion_Dot(q1, q2);
	
	if (theta <= -1.0f + FLT_EPSILON || theta >= 1.0f - FLT_EPSILON) {
		/* 
		 *	Src0 and q2 are the same (or opposites), so 
		 *	interpolation is a no-op 
		 */
		if (t <= 0.5f) {
			*result = *q1;
		} else {
			*result = *q2;
		}
		
		return (result);
	}
	
	theta = Acos(theta);
	scale = 1.0f / Sin(theta);
	
	/*
	 *  THETA AND SCALE ARE THE SAME FOR ALL VALUES OF t, 
	 *  BUT THERE IS NOWHERE TO PUT THEM AS STATE VARIABLES 
	 */
	s0 = Sin((1.0f - t) * theta) * scale;
	s1 = Sin(       t  * theta) * scale;
	
	result->w = s0 * q1->w + s1 * q2->w;
	result->x = s0 * q1->x + s1 * q2->x;
	result->y = s0 * q1->y + s1 * q2->y;
	result->z = s0 * q1->z + s1 * q2->z;
	
	return (result);
}


/*===========================================================================*\
 *
 *	Routine:	Vector3D_TransformQuaternion()
 *
 *	Comments:	
 *
\*===========================================================================*/

TQ3Vector3D *Q3Vector3D_TransformQuaternion(
	const TQ3Vector3D 		*vector3D, 
	const TQ3Quaternion 	*quaternion,
	TQ3Vector3D				*result)
{
	TQ3Quaternion	tmp1, tmp2;
	TQ3Quaternion	qInverse;
	
	/*
	 *  "Convert" vector to quaternion
	 */
	tmp1.w = 0.0;
	tmp1.x = vector3D->x;
	tmp1.y = vector3D->y;
	tmp1.z = vector3D->z;
	
	/*
	 *  Get inverse of quaternion
	 */
	qInverse.w =  quaternion->w;
	qInverse.x = -quaternion->x;
	qInverse.y = -quaternion->y;
	qInverse.z = -quaternion->z;
	
	/*
	 *  Multiply on the left by the quaternion inverse
	 */
	Q3Quaternion_Multiply(&qInverse, &tmp1, &tmp2);
	
	/*
	 *  Multiply on the right by the quaternion
	 */
	Q3Quaternion_Multiply(&tmp2, quaternion, &tmp1);
	
	result->x = tmp1.x;
	result->y = tmp1.y;
	result->z = tmp1.z;
	
	return (result);
}


/*===========================================================================*\
 *
 *	Routine:	Q3Point3D_TransformQuaternion()
 *
 *	Comments:	
 *
\*===========================================================================*/

TQ3Point3D *Q3Point3D_TransformQuaternion(
	const TQ3Point3D 		*point3D, 
	const TQ3Quaternion 	*quaternion,
	TQ3Point3D				*result)
{
	TQ3Quaternion	tmp1, tmp2;
	TQ3Quaternion	qInverse;
	
	/*
	 *  "Convert" point to quaternion
	 */
	tmp1.w = 0.0;
	tmp1.x = point3D->x;
	tmp1.y = point3D->y;
	tmp1.z = point3D->z;
	
	/*
	 *  Get inverse of quaternion
	 */
	qInverse.w =  quaternion->w;
	qInverse.x = -quaternion->x;
	qInverse.y = -quaternion->y;
	qInverse.z = -quaternion->z;
	
	/*
	 *  Multiply on the left by the quaternion inverse
	 */
	Q3Quaternion_Multiply(&qInverse, &tmp1, &tmp2);
	
	/*
	 *  Multiply on the right by the quaternion
	 */
	Q3Quaternion_Multiply(&tmp2, quaternion, &tmp1);
	
	result->x = tmp1.x;
	result->y = tmp1.y;
	result->z = tmp1.z;
	
	return (result);
}
