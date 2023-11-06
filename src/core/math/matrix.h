#pragma once
#include <memory.h>
#include <memory>
#include <cstring>
#include <stdio.h>

#include "vectors/include.h"

struct mat2
{
	union{__m256 x; float m[2][2] = {{0.0f},{0.0f}};};

	inline mat2() {};

	inline mat2(float a[2][2])
	{
		m[0][0] = a[0][0]; m[0][1] = a[0][1];
		m[1][0] = a[1][0]; m[1][1] = a[1][1];
	};

	inline mat2(float a[4])
	{
		m[0][0] = a[0]; m[0][1] = a[1]; 
		m[1][0] = a[2]; m[1][1] = a[3];
	};

	inline mat2(float a00, float a01, float a10, float a11)
	{
		m[0][0] = a00; m[0][1] = a01;
		m[1][0] = a10; m[1][1] = a11;
	};

	inline mat2 operator*(float value) const
	{
		mat2 result;
		for(int i=0; i < 2; ++i)
			for(int j=0; j < 2; ++j)
				result.m[i][j] = m[i][j]*value;
		return result;
	};

	inline mat2& operator=(const mat2 &mat)
	{
		for(int i = 0; i < 2; i++)
			for(int j = 0; j < 2; j++)
				m[i][j] = mat.m[i][j];
		return *this;
	};

	inline mat2(const mat2 &mtx) {memcpy(m,mtx.m,sizeof(float)*2*2);};

	inline float& operator()(int i, int j) {return m[i][j];};

	inline const float& operator()(int i, int j) const {return m[i][j];};

	inline vec2f operator*(const vec2f &v) const
	{
		return vec2f(
			m[0][0] * v.x + m[0][1] * v.y,
			m[1][0] * v.x + m[1][1] * v.y
		);
	};
};

template <typename T>
struct alignas(32) matrix4x4
{
	T m[4][4] = {{0.0f},{0.0f}};

	inline matrix4x4() {};

	inline matrix4x4(T a[4][4])
	{
		m[0][0] = a[0][0]; m[0][1] = a[0][1]; m[0][2] = a[0][2]; m[0][3] = a[0][3];
		m[1][0] = a[1][0]; m[1][1] = a[1][1]; m[1][2] = a[1][2]; m[1][3] = a[1][3];
		m[2][0] = a[2][0]; m[2][1] = a[2][1]; m[2][2] = a[2][2]; m[2][3] = a[2][3];
		m[3][0] = a[3][0]; m[3][1] = a[3][1]; m[3][2] = a[3][2]; m[3][3] = a[3][3];
	};

	inline matrix4x4(T a[16])
	{
		m[0][0] = a[ 0]; m[0][1] = a[ 1]; m[0][2] = a[ 2]; m[0][3] = a[ 3];
		m[1][0] = a[ 4]; m[1][1] = a[ 5]; m[1][2] = a[ 6]; m[1][3] = a[ 7];
		m[2][0] = a[ 8]; m[2][1] = a[ 9]; m[2][2] = a[10]; m[2][3] = a[11];
		m[3][0] = a[12]; m[3][1] = a[13]; m[3][2] = a[14]; m[3][3] = a[15];
	};

	inline matrix4x4(
		const T &a00, const T &a01, const T &a02, const T &a03,
		const T &a10, const T &a11, const T &a12, const T &a13,
		const T &a20, const T &a21, const T &a22, const T &a23,
		const T &a30, const T &a31, const T &a32, const T &a33
	)
	{
		m[0][0] = a00; m[0][1] = a01; m[0][2] = a02; m[0][3] = a03;
		m[1][0] = a10; m[1][1] = a11; m[1][2] = a12; m[1][3] = a13;
		m[2][0] = a20; m[2][1] = a21; m[2][2] = a22; m[2][3] = a23;
		m[3][0] = a30; m[3][1] = a31; m[3][2] = a32; m[3][3] = a33;
	};

	inline matrix4x4 operator*(const T &value) const
	{
		matrix4x4 result;
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
				result.m[i][j] = m[i][j] * value;
		return result;
	};

	inline matrix4x4& operator=(const matrix4x4 &mat)
	{
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 4; j++)
				m[i][j] = mat.m[i][j];
		return *this;
	};

	inline matrix4x4(const matrix4x4 &mtx) {memcpy(m,mtx.m,sizeof(T)*4*4);};

	inline T& operator()(int i, int j) {return m[i][j];};

	inline const T& operator()(int i, int j) const {return m[i][j];};
};

/*inline void print(const matrix4x4 &a)
{
	printf("%2.3f\t%2.3f\t%2.3f\t%2.3f\n",a.m[0][0],a.m[0][1],a.m[0][2],a.m[0][3]);
	printf("%2.3f\t%2.3f\t%2.3f\t%2.3f\n",a.m[1][0],a.m[1][1],a.m[1][2],a.m[1][3]);
	printf("%2.3f\t%2.3f\t%2.3f\t%2.3f\n",a.m[2][0],a.m[2][1],a.m[2][2],a.m[2][3]);
	printf("%2.3f\t%2.3f\t%2.3f\t%2.3f\n",a.m[3][0],a.m[3][1],a.m[3][2],a.m[3][3]);
};*/

template <typename T>
inline matrix4x4<T> operator*(const matrix4x4<T> &mat1, const matrix4x4<T> &mat2)
{
	matrix4x4<T> result;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			T sum = 0.0f;

			for(int k = 0; k < 4; k++)
				sum += mat1.m[i][k] * mat2.m[k][j];
			
			result.m[i][j] = sum;
		}
	}

	return result;
};

template <typename T>
inline void set_identity(matrix4x4<T> &m)
{
	for(int i=0; i < 4; ++i)
		for(int j = 0; j < 4; j++)
			m.m[i][j] = (i == j) ? T(1.0f) : T(0.0f);
};

template <typename T>
inline void transpose(const matrix4x4<T> &src, matrix4x4<T> &target)
{
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			target.m[i][j] = src.m[j][i];
};

template <typename T>
inline bool invert(const matrix4x4<T> &src, matrix4x4<T> &target)
{
	int indxc[4], indxr[4];
	int ipiv[4];
	memset(ipiv, 0, sizeof(int)*4);
	memcpy(target.m, src.m, 4*4*sizeof(T));

	for(int i = 0; i < 4; i++)
	{
		int irow = -1, icol = -1;
		T big = 0;
		for(int j = 0; j < 4; j++)
		{
			if(ipiv[j] != 1)
			{
				for(int k = 0; k < 4; k++)
				{
					if(ipiv[k] == 0)
					{
						if(abs(target.m[j][k]) >= big)
						{
							big = abs(target.m[j][k]);
							irow = j;
							icol = k;
						}
					}
					else if(ipiv[k] > 1)
					{
						return false;
					}
				}
			}
		}
		
		++ipiv[icol]; 

		if(irow != icol)
			for(int k = 0; k < 4; ++k)
				std::swap(target.m[irow][k], target.m[icol][k]);
		
		indxr[i] = irow;
		indxc[i] = icol;

		if(target.m[icol][icol] == 0)
			return false;
		
		T pivinv = T(1.0f) / target.m[icol][icol];
		target.m[icol][icol] = T(1.0f);
		
		for(int j = 0; j < 4; j++)
			target.m[icol][j] *= pivinv;

		for(int j = 0; j < 4; j++)
		{
			if(j != icol)
			{
				T save = target.m[j][icol];
				target.m[j][icol] = 0;
				for(int k = 0; k < 4; k++)
					target.m[j][k] -= target.m[icol][k]*save;
			}
		}
	}

	for(int j = 4-1; j >= 0; j--)
		if(indxr[j] != indxc[j])
			for(int k = 0; k < 4; k++)
				std::swap(target.m[k][indxr[j]], target.m[k][indxc[j]]);
	
	return true;
};

inline void ortho_normalize(float m[4][4])
{
	float len, temp[3][3];
	for(u_int i = 0; i < 3; ++i)
		for(u_int j = 0; j < 3; ++j)
			temp[i][j] = m[i][j];

	// normalize x
	len = sqrtf(temp[0][0] * temp[0][0] + temp[0][1] * temp[0][1] + temp[0][2] * temp[0][2]);
	len = (len == 0.f) ? 1.f : 1.f / len;
	temp[0][0] *= len; temp[0][1] *= len; temp[0][2] *= len;

	// z = x cross y
	temp[2][0] = (temp[0][1] * temp[1][2] - temp[0][2] * temp[1][1]);
	temp[2][1] = (temp[0][2] * temp[1][0] - temp[0][0] * temp[1][2]);
	temp[2][2] = (temp[0][0] * temp[1][1] - temp[0][1] * temp[1][0]);

	// normalize z
	len = sqrtf(temp[2][0] * temp[2][0] + temp[2][1] * temp[2][1] + temp[2][2] * temp[2][2]);
	len = (len == 0.f) ? 1.f : 1.f / len;
	temp[2][0] *= len; temp[2][1] *= len; temp[2][2] *= len;

	// y = z cross x
	temp[1][0] = (temp[2][1] * temp[0][2] - temp[2][2] * temp[0][1]);
	temp[1][1] = (temp[2][2] * temp[0][0] - temp[2][0] * temp[0][2]);
	temp[1][2] = (temp[2][0] * temp[0][1] - temp[2][1] * temp[0][0]);

	// normalize y
	len = sqrtf(temp[1][0] * temp[1][0] + temp[1][1] * temp[1][1] + temp[1][2] * temp[1][2]);
	len = (len == 0.f) ? 1.f : 1.f / len;
	temp[1][0] *= len; temp[1][1] *= len; temp[1][2] *= len;

	// update matrix
	m[0][0] = temp[0][0]; m[0][1] = temp[0][1]; m[0][2] = temp[0][2];
	m[1][0] = temp[1][0]; m[1][1] = temp[1][1]; m[1][2] = temp[1][2];
	m[2][0] = temp[2][0]; m[2][1] = temp[2][1]; m[2][2] = temp[2][2];
};