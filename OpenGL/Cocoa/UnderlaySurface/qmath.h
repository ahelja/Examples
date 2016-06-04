#ifndef __QMATH__
#define __QMATH__

#include <float.h>

#ifdef __cplusplus
extern "C" {
#endif

enum TQ3Boolean {
    kQ3False                    = 0,
    kQ3True                     = 1
};

typedef enum TQ3Boolean TQ3Boolean;

typedef struct {
    float                           x;
    float                           y;
    float                           z;
} TQ3Vector3D;

typedef TQ3Vector3D TQ3Point3D;


typedef struct {
    float                           value[4][4];
} TQ3Matrix4x4;


/******************************************************************************
 **                                                                          **
 **                             Quaternion                                   **
 **                                                                          **
 *****************************************************************************/

typedef struct {
    float                           w;
    float                           x;
    float                           y;
    float                           z;
} TQ3Quaternion;


/******************************************************************************
 **																			 **
 **								Matrix Functions							 **
 **																			 **
 *****************************************************************************/

TQ3Matrix4x4 *Q3Matrix4x4_SetIdentity(
	TQ3Matrix4x4				*matrix4x4);

TQ3Matrix4x4 *Q3Matrix4x4_Multiply(
	const TQ3Matrix4x4			*matrixA,
	const TQ3Matrix4x4			*matrixB,
	TQ3Matrix4x4				*result);

TQ3Matrix4x4 *Q3Matrix4x4_SetQuaternion(
	TQ3Matrix4x4 				*matrix, 
	const TQ3Quaternion 		*quaternion);


/******************************************************************************
 **																			 **
 **								Quaternion Routines						     **
 **																			 **
 *****************************************************************************/

TQ3Quaternion *Q3Quaternion_SetIdentity(
	TQ3Quaternion				*quaternion);
	
TQ3Quaternion *Q3Quaternion_Invert(
	const TQ3Quaternion 		*quaternion,
	TQ3Quaternion				*result);

TQ3Quaternion *Q3Quaternion_Normalize(
	const TQ3Quaternion			*quaternion,
	TQ3Quaternion				*result);

float Q3Quaternion_Dot(
	const TQ3Quaternion 		*q1, 
	const TQ3Quaternion 		*q2);

TQ3Quaternion *Q3Quaternion_Multiply(
	const TQ3Quaternion			*q1, 
	const TQ3Quaternion			*q2,
	TQ3Quaternion				*result);

TQ3Quaternion *Q3Quaternion_SetRotateAboutAxis(
	TQ3Quaternion 				*quaternion, 
	const TQ3Vector3D 			*axis, 
	float 						angle);

TQ3Quaternion *Q3Quaternion_SetMatrix(
	TQ3Quaternion 				*quaternion, 
	const TQ3Matrix4x4 			*matrix);

TQ3Quaternion *Q3Quaternion_SetRotateVectorToVector(
	TQ3Quaternion 				*quaternion, 
	const TQ3Vector3D 			*v1, 
	const TQ3Vector3D 			*v2);

TQ3Quaternion *Q3Quaternion_InterpolateLinear(
	const TQ3Quaternion 		*q1, 
	const TQ3Quaternion 		*q2, 
	float 						t,
	TQ3Quaternion 				*result);

TQ3Vector3D *Q3Vector3D_TransformQuaternion(
	const TQ3Vector3D 			*vector3D, 
	const TQ3Quaternion 		*quaternion,
	TQ3Vector3D					*result);

#ifdef __cplusplus
}
#endif

#endif /* __QMATH__ */



