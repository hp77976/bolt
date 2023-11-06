#pragma once
#include "vectors/include.h"
#include "matrix.h"
#include "misc.h"
#include <math.h>
#include <cmath>

template <typename T>
struct transform
{
	matrix4x4<T> mt;
	matrix4x4<T> mi;

	transform()
	{
		set_identity(mt);
		set_identity(mi);
	};

	transform(const matrix4x4<T> &tr) : mt(tr) {invert(mt,mi);};

	transform(const matrix4x4<T> &t, const matrix4x4<T> &i) : mt(t), mi(i) {};

	transform operator*(const transform &t) const {return transform(mt * t.mt, t.mi * mi);};
};

template <typename T>
inline transform<T> inverse(const transform<T> &t) {return transform<T>(t.mi, t.mt);};

template <typename T>
inline vec3<T> transform_point(const transform<T> &t, const vec3<T> &p)
{
	T x = t.mt.m[0][0] * p.x + t.mt.m[0][1] * p.y + t.mt.m[0][2] * p.z + t.mt.m[0][3];
	T y = t.mt.m[1][0] * p.x + t.mt.m[1][1] * p.y + t.mt.m[1][2] * p.z + t.mt.m[1][3];
	T z = t.mt.m[2][0] * p.x + t.mt.m[2][1] * p.y + t.mt.m[2][2] * p.z + t.mt.m[2][3];
	T w = t.mt.m[3][0] * p.x + t.mt.m[3][1] * p.y + t.mt.m[3][2] * p.z + t.mt.m[3][3];

	vec3<T> v = vec3<T>(x,y,z);
	//return select(w == T(1.0f),v,v/w); //this is silly, x / 1 = x... eliminate the branch
	return v / w;
};

template <typename T>
inline vec3<T> transform_point_affine(const transform<T> &t, const vec3<T> &p)
{
	T x = t.mt.m[0][0] * p.x + t.mt.m[0][1] * p.y + t.mt.m[0][2] * p.z + t.mt.m[0][3];
	T y = t.mt.m[1][0] * p.x + t.mt.m[1][1] * p.y + t.mt.m[1][2] * p.z + t.mt.m[1][3];
	T z = t.mt.m[2][0] * p.x + t.mt.m[2][1] * p.y + t.mt.m[2][2] * p.z + t.mt.m[2][3];
	return vec3<T>(x,y,z);
};

template <typename T>
inline vec3<T> transform_vector(const transform<T> &t, const vec3<T> &v)
{
	T x = t.mt.m[0][0] * v.x + t.mt.m[0][1] * v.y + t.mt.m[0][2] * v.z;
	T y = t.mt.m[1][0] * v.x + t.mt.m[1][1] * v.y + t.mt.m[1][2] * v.z;
	T z = t.mt.m[2][0] * v.x + t.mt.m[2][1] * v.y + t.mt.m[2][2] * v.z;
	return vec3<T>(x,y,z);
};

template <typename T>
inline vec3<T> transform_normal(const transform<T> &t, const vec3<T> &v)
{
	T x = t.mi.m[0][0] * v.x + t.mi.m[1][0] * v.y + t.mi.m[2][0] * v.z;
	T y = t.mi.m[0][1] * v.x + t.mi.m[1][1] * v.y + t.mi.m[2][1] * v.z;
	T z = t.mi.m[0][2] * v.x + t.mi.m[1][2] * v.y + t.mi.m[2][2] * v.z;
	return vec3<T>(x,y,z);
};

template <typename T>
inline transform<T> translate(const vec3<T> &v)
{
	matrix4x4<T> t(
		1, 0, 0, v.x,
		0, 1, 0, v.y,
		0, 0, 1, v.z,
		0, 0, 0, 1
	);

	matrix4x4<T> i(
		1, 0, 0, -v.x,
		0, 1, 0, -v.y,
		0, 0, 1, -v.z,
		0, 0, 0, 1
	);

	return transform<T>(t,i);
};

template <typename T>
inline transform<T> scale(const vec3<T> &v)
{
	matrix4x4<T> t(
		v.x, 0,   0,   0,
		0,   v.y, 0,   0,
		0,   0,   v.z, 0,
		0,   0,   0,   1
	);

	matrix4x4<T> i(
		T(1.0f)/v.x, 0,        0,        0,
		0,        T(1.0f)/v.y, 0,        0,
		0,        0,        T(1.0f)/v.z, 0,
		0,        0,        0,        1
	);

	return transform<T>(t,i);
};

template <typename T>
inline transform<T> rotate(const vec3<T> &axis, T angle)
{
	T sinTheta, cosTheta;

	/* Make sure that the axis is normalized */
	vec3<T> naxis = normalize(axis);
	sincos(deg_to_rad(angle), &sinTheta, &cosTheta);

	matrix4x4<T> result;
	result(0, 0) = naxis.x * naxis.x + (1.0f - naxis.x * naxis.x)  * cosTheta;
	result(0, 1) = naxis.x * naxis.y * (1.0f - cosTheta) - naxis.z * sinTheta;
	result(0, 2) = naxis.x * naxis.z * (1.0f - cosTheta) + naxis.y * sinTheta;
	result(0, 3) = 0;

	result(1, 0) = naxis.x * naxis.y * (1.0f - cosTheta) + naxis.z * sinTheta;
	result(1, 1) = naxis.y * naxis.y + (1.0f - naxis.y * naxis.y)  * cosTheta;
	result(1, 2) = naxis.y * naxis.z * (1.0f - cosTheta) - naxis.x * sinTheta;
	result(1, 3) = 0;

	result(2, 0) = naxis.x * naxis.z * (1.0f - cosTheta) - naxis.y * sinTheta;
	result(2, 1) = naxis.y * naxis.z * (1.0f - cosTheta) + naxis.x * sinTheta;
	result(2, 2) = naxis.z * naxis.z + (1.0f - naxis.z * naxis.z)  * cosTheta;
	result(2, 3) = 0;

	result(3, 0) = 0;
	result(3, 1) = 0;
	result(3, 2) = 0;
	result(3, 3) = 1;

	/* The matrix is orthonormal */
	matrix4x4<T> transp;
	transpose(transp,result);
	return transform<T>(result, transp);
};

template <typename T>
inline transform<T> perspective(T fov, T clip_near, T clip_far)
{
	T recip = 1.0f / (clip_far - clip_near);

	/* Perform a scale so that the field of view is mapped
	* to the interval [-1, 1] */
	T cot = 1.0f / tan(deg_to_rad(fov / 2.0f));

	matrix4x4<T> trafo(
		cot,  0,    0,   0,
		0,    cot,  0,   0,
		0,    0,    clip_far * recip, -clip_near * clip_far * recip,
		0,    0,    1,   0
	);

	return transform(trafo);
};

template <typename T>
inline transform<T> look_at(const vec3<T> &p, const vec3<T> &t, const vec3<T> &up)
{
	vec3<T> dir = normalize(t-p);
	vec3<T> left = normalize(cross(up, dir));
	vec3<T> newUp = cross(dir, left);

	matrix4x4<T> result, inverse;
	result(0, 0)  = left.x;  result(1, 0)  = left.y;  result(2, 0)  = left.z;  result(3, 0)  = 0;
	result(0, 1)  = newUp.x; result(1, 1)  = newUp.y; result(2, 1)  = newUp.z; result(3, 1)  = 0;
	result(0, 2)  = dir.x;   result(1, 2)  = dir.y;   result(2, 2)  = dir.z;   result(3, 2)  = 0;
	result(0, 3)  = p.x;     result(1, 3)  = p.y;     result(2, 3)  = p.z;     result(3, 3)  = 1;

	//the inverse is simple to compute for this matrix, so do it directly here
	vec3<T> q(
		result(0, 0) * p.x + result(1, 0) * p.y + result(2, 0) * p.z,
		result(0, 1) * p.x + result(1, 1) * p.y + result(2, 1) * p.z,
		result(0, 2) * p.x + result(1, 2) * p.y + result(2, 2) * p.z
	);

	inverse(0, 0) = left.x; inverse(1, 0) = newUp.x; inverse(2, 0) = dir.x; inverse(3, 0) = 0;
	inverse(0, 1) = left.y; inverse(1, 1) = newUp.y; inverse(2, 1) = dir.y; inverse(3, 1) = 0;
	inverse(0, 2) = left.z; inverse(1, 2) = newUp.z; inverse(2, 2) = dir.z; inverse(3, 2) = 0;
	inverse(0, 3) = -q.x;   inverse(1, 3) = -q.y;    inverse(2, 3) = -q.z;  inverse(3, 3) = 1;

	return transform(result, inverse);
};

template <typename T>
inline void print(const transform<T> &t)
{
	print(t.mt);
	printf("\n");
	print(t.mi);
};