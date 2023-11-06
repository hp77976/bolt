#pragma once
#include "vectors/include.h"

namespace math
{
	template <typename T>
	inline T dot(const vec2<T> &a, const vec2<T> &b) {return fmadd(a.x,b.x,a.y*b.y);};

	template <typename T>
	inline T dot(const vec3<T> &a, const vec3<T> &b) {return fmadd(a.x,b.x,fmadd(a.y*b.y,a.z*b.z));};

	template <typename T>
	inline T dot(const vec4<T> &a, const vec4<T> &b) {return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;};

	template <typename T>
	inline vec3<T> cross(const vec3<T> &a, const vec3<T> &b)
	{
		T x = fmsub(a.y,b.z,a.z*b.y);
		T y = fmsub(a.z,b.x,a.x*b.z);
		T z = fmsub(a.x,b.y,a.y*b.x);
		return vec3<T>(x,y,z);
	};

	template <typename T>
	inline T clamp(T val, T low = 0.0f, float high = FP_INFINITE)
	{
		return select(val<low,max(val,low),min(val,high));
	};

	template <typename T>
	inline T deg_to_rad(const T &d) {return d * (T(M_PI) / T(180));};

	template <typename T>
	inline T rad_to_deg(const T &r) {return r * (T(180) / T(M_PI));};

	//no idea what this does
	inline int32_t floor_2_int(const float a) {return static_cast<int32_t>(floorf(a));};

	template <typename T>
	inline T cos_theta(const vec3<T> &a) {return a.z;};

	template <typename T>
	inline T tan_theta(const vec3<T> &v)
	{
		T temp = fnmad(v.z,v.z,T(1.0f));
		return select(temp <= T(0.0f),T(0.0f),sqrt(temp)/v.z);
	};

	template <typename T>
	inline T cos2theta(const vec3<T> &a) {return a.z * a.z;};

	template <typename T>
	inline T sin2theta(const vec3<T> &a) {return max(T(0.0f),T(1.0f) - cos2theta(a));};

	template <typename T>
	inline T sin_theta(const vec3<T> &a) {return sqrt(sin2theta(a));};

	template <typename T>
	inline T sin_theta2(const vec3<T> &a)
	{
		return max(T(0.0f),fnmadd(cos_theta(a),cos_theta(a),T(1.0f)));
	};

	template <typename T>
	inline T sin_phi(const vec3<T> &a)
	{
		T sin_theta_ = sin_theta(a);
		return select(sin_theta_ == T(0),T(0),clamp(a.y/sin_theta_,T(-1),T(1)));
	};

	template <typename T>
	inline T cos_phi(const vec3<T> &a)
	{
		T sin_theta_ = sin_theta(a);
		return select(sin_theta_ == T(0),T(1),clamp(a.x/sin_theta_,T(-1),T(1)));
	};

	template <typename T>
	inline T get_phi(const T &a, const T &b)
	{
		return T((M_PI) * 0.5f) * t_sqrt(a * b / (T(1.0f) - a * (T(1.0f) - b)));
	};

	template <typename T>
	inline T hypot2(const T &a, const T &b)
	{
		T r;

		T abs_a = abs(a);
		T abs_b = abs(b);

		T c0 = abs_a > abs_b;
		T c1 = b != T(0.0f);

		r = select(c0,b/a,a/b);
		r = select(c0,abs_a,abs_b) * sqrt(fmadd(r,r,T(1.0f)));
		r = select(c1,r,T(0.0f));

		return r;
	};

	template <typename T>
	inline T same_hemi(const vec3<T> &a, const vec3<T> &b) {return a.z * b.z > T(0.0f);};

	template <typename T>
	inline vec2<T> concentric_sample_disk(const vec2<T> &u)
	{
		T r, theta;
		T sx = fmsub(T(2.0f),u.x,T(1.0f));
		T sy = fmsub(T(2.0f),u.y,T(1.0f));

		r = select(sx>=-sy,select(sx>sy,sx,sy),select(sx<=sy,-sx,-sy));
		theta = select(
			sx>=-sy,
			select(sx>sy,select(sy>T(0.0f),sy/r,T(8.0f)+sy/r),T(2.0f)-sx/r),
			select(sx<=sy,T(4.0f)-sy/r,T(6.0f)+sx/r)
		);

		theta *= T(M_PI) / T(4.0f);

		r = select(sx == T(0.0f) && sy == T(0.0f),T(0.0f),r);
		
		return vec2<T>(r * cos(theta), r * sin(theta));
	};

	template <>
	inline vec2<float> concentric_sample_disk(const vec2<float> &u)
	{
		float r, theta;
		float sx = 2.0f * u.x - 1.0f;
		float sy = 2.0f * u.y - 1.0f;

		if(sx == 0.0f && sy == 0.0f)
			return vec2<float>(0.0f,0.0f);

		if(sx >= -sy)
		{
			if(sx > sy)
			{
				r = sx;
				if(sy > 0.0f)
					theta = sy / r;
				else
					theta = 8.0f + sy / r;
			}
			else
			{
				r = sy;
				theta = 2.f - sx / r;
			}
		}
		else
		{
			if(sx <= sy)
			{
				r = -sx;
				theta = 4.0f - sy / r;
			}
			else
			{
				r = -sy;
				theta = 6.0f + sx / r;
			}
		}

		theta *= float(M_PI) / 4.0f;
		return vec2<float>(r * cos(theta), r * sin(theta));
	};

	template <typename T>
	inline T square_to_cos_hemi_pdf(const vec3<T> &v) {return (T)M_1_PI * cos_theta(v);};

	template <typename T>
	inline vec3<T> cos_sample_hemi(const vec2<T> &u)
	{
		vec2<T> d = concentric_sample_disk(u);
		T z = sqrt(max(0.0f,fnmadd(d.y,d.y,fnmadd(d.x,d.x,1.0f))));
		return vec3<T>(d.x,d.y,z);
	};

	template <typename T>
	inline vec3<T> square_to_uni_sphere(const vec2<T> &sample)
	{
		T z = fnmadd(T(2.0f),sample.y,T(1.0f));
		T r = safe_sqrt(fnmadd(z,z,T(1.0f)));
		
		T sin_phi, cos_phi;
		sincos(T(2.0f * M_PI) * sample.x, &sin_phi, &cos_phi);

		return vec3<T>(r * cos_phi, r * sin_phi, z);
	};

	template <typename T>
	inline vec3<T> square_to_uni_cone(const T &cos_cutoff, const vec2<T> &sample)
	{
		T ct = fmadd(sample.x,cos_cutoff,(T(1.0f)-sample.x));
		T st = safe_sqrt(fnmadd(ct,ct,T(1.0f)));
		T sp, cp;
		sincos(T(2.0f*M_PI)*sample.y,&sp,&cp);
		return vec3<T>(cp*st,sp*st,ct);
	};

	template <typename T>
	inline T square_to_uni_cone_pdf(const T &cos_cutoff) {return T(M_2_PI) / (T(1.0f) - cos_cutoff);};

	template <typename T>
	inline T within(const T &a, const T &min, const T &max) {return ((a > min) && (a < max));};

	template <typename T>
	inline vec2<T> square_to_uni_disk_concentric(const vec2<T> &s)
	{
		T r1 = fmsub(T(2.0f),s.x,T(1.0f));
		T r2 = fmsub(T(2.0f),s.y,T(1.0f));
		T p, r;
		T mp4 = T(M_PI/4.0f);

		r = select(r1==T(0.0f)&&r2==T(0.0f),T(0.0f),select(r1*r1>r2*r2,r1,r2));
		p = select(r1==T(0.0f)&&r2==T(0.0f),T(0.0f),select(
			r1*r1>r2*r2,
			mp4*(r2/r1),
			fnmadd((r1/r2),mp4,T(M_PI/2.0f))
		));

		T cos_p, sin_p;
		sincos(p,&sin_p,&cos_p);
		return vec2<T>(r*cos_p,r*sin_p);
	};

	template <>
	inline vec2<float> square_to_uni_disk_concentric(const vec2<float> &s)
	{
		float r1 = 2.0f * s.x - 1.0f;
		float r2 = 2.0f * s.y - 1.0f;

		float p, r;
		if(r1 == 0.0f && r2 == 0.0f)
		{
			r = p = 0.0f;
		}
		else if(r1 * r1 > r2 * r2)
		{
			r = r1;
			p = (M_PI / 4.0f) * (r2 / r1);
		}
		else
		{
			r = r2;
			p = (M_PI / 2.0f) - (r1 / r2) * (M_PI / 4.0f);
		}

		float cos_p, sin_p;
		sincosf(p,&sin_p,&cos_p);
		return vec2<float>(r*cos_p,r*sin_p);
	};

	template <typename T>
	inline vec3<T> square_to_cos_hemi(const vec2<T> &s)
	{
		vec2<T> p = square_to_uni_disk_concentric(s);
		T z = safe_sqrt(fnmadd(p.y,p.y,fnmadd(p.x,p.x,T(1.0f))));
		z = select(z==T(0.0f),T(1e-10f),z);
		return vec3<T>(p.x,p.y,z);
	};

	template <typename T>
	inline T mi_weight(T a, T b) {return (a * a) / (fmadd(b,b,(a * a)));};

	template <typename T>
	inline T solve_quadratic(const T &a, const T &b, const T &c, T &x0, T &x1)
	{
		if(!any_false(a == T(0.0f))) //if all true
		{
			if(!any_false(b != T(0.0f)))
			{
				x0 = x1 = -c / b;
				return T(0b11111111111111111111111111111111);
			}

			return T(0.0f);
		}

		T discrim = fmsub(b,b,T(4.0f)*a*c);

		if(!any_false(discrim < T(0.0f)))
			return T(0.0f);
		
		T temp, sqrt_discrim = sqrt(discrim);

		temp = -0.5f * (b + select(b<T(0.0f),-sqrt_discrim,sqrt_discrim));

		x0 = temp / a;
		x1 = c / temp;

		x0 = select(x0>x1,x1,x0);
		x1 = select(x0>x1,x0,x1);

		return T(0b11111111111111111111111111111111);
	};

	template <>
	inline float solve_quadratic(
		const float &a, const float &b, const float &c, float &x0, float &x1
	)
	{
		if(a == 0.0f)
		{
			if(b != 0.0f)
			{
				x0 = x1 = -c / b;
				return true;
			}
			return false;
		}

		float discrim = b * b - 4.0f * a * c;

		if(discrim < 0.0f)
			return false;
		
		float temp, sqrt_discrim = std::sqrt(discrim);

		if(b < 0.0f)
			temp = -0.5f * (b - sqrt_discrim);
		else
			temp = -0.5f * (b + sqrt_discrim);

		x0 = temp / a;
		x1 = c / temp;

		if(x0 > x1)
			std::swap(x0,x1);

		return true;
	};
};