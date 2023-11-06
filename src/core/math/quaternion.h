#pragma once
#include "vectors/include.h"
#include "matrix.h"

struct quat
{
	float w, x, y, z;

	inline quat(float w_, float x_, float y_, float z_) : w(w_), x(x_), y(y_), z(z_) {};

	inline quat(float w_, const vec3f &v) : w(w_), x(v.x), y(v.y), z(v.z) {};

	inline quat(const matrix4x4<float> &m)
	{
		float ortho[4][4];
		memcpy(ortho, m.m, sizeof(float) * 16);
		ortho_normalize(ortho);

		const float trace = ortho[0][0] + ortho[1][1] + ortho[2][2] + 1.f;

		if(trace > 1e-6f)
		{
			const float s = sqrtf(trace) * 2.f;
			x = (ortho[1][2] - ortho[2][1]) / s;
			y = (ortho[2][0] - ortho[0][2]) / s;
			z = (ortho[0][1] - ortho[1][0]) / s;
			w = 0.25f * s;
		}
		else 
		{
			if(ortho[0][0] > ortho[1][1] && ortho[0][0] > ortho[2][2])
			{
				// Column 0: 
				const float s  = sqrtf(1.f + ortho[0][0] - ortho[1][1] - ortho[2][2]) * 2.f;
				x = 0.25f * s;
				y = (ortho[0][1] + ortho[1][0] ) / s;
				z = (ortho[2][0] + ortho[0][2] ) / s;
				w = (ortho[1][2] - ortho[2][1] ) / s;
			}
			else if(ortho[1][1] > ortho[2][2])
			{
				// Column 1: 
				const float s  = sqrtf(1.f + ortho[1][1] - ortho[0][0] - ortho[2][2]) * 2.f;
				x = (ortho[0][1] + ortho[1][0] ) / s;
				y = 0.25f * s;
				z = (ortho[1][2] + ortho[2][1] ) / s;
				w = (ortho[2][0] - ortho[0][2] ) / s;
			}
			else
			{
				// Column 2:
				const float s  = sqrtf(1.f + ortho[2][2] - ortho[0][0] - ortho[1][1]) * 2.f;
				x = (ortho[2][0] + ortho[0][2] ) / s;
				y = (ortho[1][2] + ortho[2][1] ) / s;
				z = 0.25f * s;
				w = (ortho[0][1] - ortho[1][0] ) / s;
			}
		}
	};

	inline quat operator*(const float a) const {return quat(w*a,x*a,y*a,z*a);};
	inline quat operator/(const float a) const {return quat(w/a,x/a,y/a,z/a);};
};

inline quat operator*(const float f, const quat &q)
{
	vec3f av = vec3f(q.x,q.y,q.z);
	return quat(q.w * f, av * f);
};

inline quat operator*(const quat &a, const quat &b) 
{
	vec3f av = vec3f(a.x,a.y,a.z);
	vec3f bv = vec3f(b.x,b.y,b.z);

	return quat(a.w * b.w - dot(av,bv),a.w * bv + b.w * av + cross(av,bv));
};

inline float dot(const quat &a, const quat &b)
{
	vec3f av = vec3f(a.x,a.y,a.z);
	vec3f bv = vec3f(b.x,b.y,b.z);
	return a.w * b.w + dot(av,bv);
};

inline quat normalize(const quat &a) {return (1.0f / std::sqrt(dot(a,a))) * a;};

inline matrix4x4<float> quat_to_rotation_matrix(const quat &a)
{
	matrix4x4<float> m;

	const float xx = a.x * a.x;
	const float yy = a.y * a.y;
	const float zz = a.z * a.z;
	const float xy = a.x * a.y;
	const float xz = a.x * a.z;
	const float yz = a.y * a.z;
	const float xw = a.x * a.w;
	const float yw = a.y * a.w;
	const float zw = a.z * a.w;

	m.m[0][0] = 1.f - 2.f * (yy + zz);
	m.m[1][0] = 2.f * (xy - zw);
	m.m[2][0] = 2.f * (xz + yw);
	m.m[0][1] = 2.f * (xy + zw);
	m.m[1][1] = 1.f - 2.f * (xx + zz);
	m.m[2][1] = 2.f * (yz - xw);
	m.m[0][2] = 2.f * (xz - yw);
	m.m[1][2] = 2.f * (yz + xw);
	m.m[2][2] = 1.f - 2.f * (xx + yy);

	// complete matrix
	m.m[0][3] = m.m[1][3] = m.m[2][3] = 0.f;
	m.m[3][0] = m.m[3][1] = m.m[3][2] = 0.f;
	m.m[3][3] = 1.f;

	return m;
};

inline quat rotation_from_vectors(const vec3f &a, const vec3f &b)
{
	//TODO: investigate y vs x acis rotation...
	if(a == -b)	return quat(0.0f,0.0f,1.0f,0.0f);
	
	vec3f half_ = normalize(a + b);
	return quat(dot(a,half_),cross(a,half_));
};

inline quat inv(const quat &q) {return quat(q.w,-q.x,-q.y,-q.z);};

inline vec3f rotate_quat(const vec3f &v, const quat &q)
{
	const quat vq = quat(0.0f,v.x,v.y,v.z);
	const quat rq = q * (vq * inv(q));
	return vec3f(rq.x,rq.y,rq.z);
};

inline quat get_rot(const vec3f &n) {return rotation_from_vectors(n,vec3f(0.0f,0.0f,1.0f));};

inline vec3f to_local(const vec3f &v, const quat &r) {return rotate_quat(v,r);};

inline vec3f to_global(const vec3f &v, const quat &r) {return rotate_quat(v,inv(r));};

inline vec3f complex(const quat &a) {return vec3f(a.x,a.y,a.z);};