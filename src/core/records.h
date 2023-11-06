#pragma once
#include "math/include.h"
#include "geometry/tri.h"
#include "geometry/differentials.h"
#include "shape.h"
#include <stdexcept>

enum EBSDFType {
	ENull                = 0x00001,
	EDiffuseReflection   = 0x00002,
	EDiffuseTransmission = 0x00004,
	EGlossyReflection    = 0x00008,
	EGlossyTransmission  = 0x00010,
	EDeltaReflection     = 0x00020,
	EDeltaTransmission   = 0x00040,
	EDelta1DReflection   = 0x00080,
	EDelta1DTransmission = 0x00100,

	/// The lobe is not invariant to rotation around the normal
	EAnisotropic         = 0x01000,
	/// The BSDF depends on the UV coordinates
	ESpatiallyVarying    = 0x02000,
	/// Flags non-symmetry (e.g. transmission in dielectric materials)
	ENonSymmetric        = 0x04000,
	/// Supports interactions on the front-facing side
	EFrontSide           = 0x08000,
	/// Supports interactions on the back-facing side
	EBackSide            = 0x10000,
	/// Uses extra random numbers from the supplied sampler instance
	EUsesSampler         = 0x20000
};

enum e_type_combos {
	/// Any reflection component (scattering into discrete, 1D, or 2D set of directions)
	EReflection   = EDiffuseReflection | EDeltaReflection | EDelta1DReflection | EGlossyReflection,
	/// Any transmission component (scattering into discrete, 1D, or 2D set of directions)
	ETransmission = EDiffuseTransmission|EDeltaTransmission|EDelta1DTransmission|EGlossyTransmission|ENull,
	/// Diffuse scattering into a 2D set of directions
	EDiffuse      = EDiffuseReflection | EDiffuseTransmission,
	/// Non-diffuse scattering into a 2D set of directions
	EGlossy       = EGlossyReflection | EGlossyTransmission,
	/// Scattering into a 2D set of directions
	ESmooth       = EDiffuse | EGlossy,
	/// Scattering into a discrete set of directions
	EDelta        = ENull | EDeltaReflection | EDeltaTransmission,
	/// Scattering into a 1D space of directions
	EDelta1D      = EDelta1DReflection | EDelta1DTransmission,
	/// Any kind of scattering
	EAll          = EDiffuse | EGlossy | EDelta | EDelta1D
};

enum transport_mode
{
	TM_RADIANCE = 0,
	TM_IMPORTANCE = 1
};

enum e_measure
{
	E_INVALID_MEASURE = 0,
	E_SOLID_ANGLE = 1, //direction?
	E_LENGTH = 2, //does not appear to be used in BDPT
	E_AREA = 3, //needs direction sample?
	E_DISCRETE = 4, //position? doesn't need direction sample?
};

struct bsdf_record;
struct hit_record
{
	//wi and wo always point away from each other
	tri* tp = nullptr;
	shape m_shape;
	vec3f pos;
	float dist;
	vec3f wi;
	vec3f wo;
	vec2f uv;
	vec3f bary;
	frame<> sh_frame;

	//extension for vulkan
	bool hit_light;
	//float light_pdf;

	hit_record() {};

	void from_bsdf(const bsdf_record &b_rec);

	inline material* get_material() const {return m_shape.get_material();};

	inline void fill_records(diff_geo &dg)
	{
		vec3f p0 = tp->a.p;
		vec3f p1 = tp->b.p;
		vec3f p2 = tp->c.p;

		vec3f n0 = tp->a.n;
		vec3f n1 = tp->b.n;
		vec3f n2 = tp->c.n;

		bary = dg.data.coords;
		pos = p0 * bary.x + p1 * bary.y + p2 * bary.z;

		vec3f s1 = p1 - p0;
		vec3f s2 = p2 - p0;
		vec3f face_norm = cross(s1,s2);
		float length = len(face_norm);
		if(face_norm != 0.0f)
			face_norm /= length;

		dg.dpdu = s1;
		dg.dpdv = s2;

		//TODO: get UVs working again
		//dg.uv = u0 * b.x + u1 * b.y + u2 * b.z;

		sh_frame.n = normalize(n0*bary.x+n1*bary.y+n2*bary.z);
		if(dot(face_norm,sh_frame.n) < 0.0f)
			face_norm = -face_norm;

		calc_shading_frame(sh_frame.n,dg.dpdu,sh_frame);
	};
};

struct bsdf_record8;
struct hit_record8
{
	tri* tp[8] = {nullptr};
	shape m_shape[8];
	vec3f8 pos;
	float8 dist;
	vec3f8 wi;
	vec3f8 wo;
	vec2f8 uv;
	vec3f8 bary;
	frame<float8> sh_frame;

	inline hit_record get(int i) const
	{
		hit_record ret;
		ret.tp = tp[i];
		ret.m_shape = m_shape[i];
		ret.pos = vec3f(pos.x[i],pos.y[i],pos.z[i]);
		ret.dist = dist[i];
		ret.wi = vec3f(wi.x[i],wi.y[i],wi.z[i]);
		ret.wo = vec3f(wo.x[i],wo.y[i],wo.z[i]);
		ret.uv = vec2f(uv.x[i],uv.y[i]);
		ret.bary = vec3f(bary.x[i],bary.y[i],bary.z[i]);
		ret.sh_frame.n = vec3f(sh_frame.n.x[i],sh_frame.n.y[i],sh_frame.n.z[i]);
		ret.sh_frame.s = vec3f(sh_frame.s.x[i],sh_frame.s.y[i],sh_frame.s.z[i]);
		ret.sh_frame.t = vec3f(sh_frame.t.x[i],sh_frame.t.y[i],sh_frame.t.z[i]);
		return ret;
	};

	inline material* get_material(int i) const {return m_shape[i].get_material();};

	void from_bsdf(const bsdf_record8 &b_rec);
};

struct bsdf_record
{
	//li and lo always point away from each other
	//li is incoming (regardless of mode!) and lo is outgoing
	vec3f li;
	float pdf;
	vec3f lo;
	float eta;
	vec3f bary;
	int32_t mode; //radiance or importance
	vec2f uv;
	int64_t tri_index;
	int32_t req_type;
	int32_t req_component;
	int32_t sampled_type;
	int32_t sampled_component;

	bsdf_record() {};

	bsdf_record(const hit_record &h_rec)
	{
		li = h_rec.sh_frame.to_local(h_rec.wi);
		lo = h_rec.sh_frame.to_local(h_rec.wo);
		uv = h_rec.uv;
		bary = h_rec.bary;
	};
};

inline void hit_record::from_bsdf(const bsdf_record &b_rec)
{
	wi = sh_frame.to_world(b_rec.li);
	wo = sh_frame.to_world(b_rec.lo);
};

struct bsdf_record8
{
	vec3f8 li;
	float8 pdf;
	vec3f8 lo;
	float8 eta;
	vec3f8 bary;
	vec2f8 uv;
	int32_8 req_type;
	int32_8 req_component;
	int32_8 sampled_type;
	int32_8 sampled_component;
	int32_8 mode; //radiance or importance

	bsdf_record8() {};

	bsdf_record8(const hit_record8 &h_rec)
	{
		li = h_rec.sh_frame.to_local(h_rec.wi);
		lo = h_rec.sh_frame.to_local(h_rec.wo);
		uv = h_rec.uv;
		bary = h_rec.bary;
	};

	bsdf_record get(int i) const
	{
		bsdf_record b_rec;
		b_rec.li = vec3f(li.x[i],li.y[i],li.z[i]);
		b_rec.pdf = pdf[i];
		b_rec.lo = vec3f(lo.x[i],lo.y[i],lo.z[i]);
		b_rec.bary = vec3f(bary.x[i],bary.y[i],bary.z[i]);
		b_rec.uv = vec2f(uv.x[i],uv.y[i]);
		b_rec.req_type = req_type[i];
		b_rec.req_component = req_component[i];
		b_rec.sampled_type = sampled_type[i];
		b_rec.sampled_component = sampled_component[i];
		b_rec.mode = mode[i];
		return b_rec;
	};

	void set(int i, const bsdf_record &b_rec)
	{
		li.x.set(i,b_rec.li.x);
		li.y.set(i,b_rec.li.y);
		li.z.set(i,b_rec.li.z);
		pdf.set(i,b_rec.pdf);
		lo.x.set(i,b_rec.lo.x);
		lo.y.set(i,b_rec.lo.y);
		lo.z.set(i,b_rec.lo.z);
		bary.x.set(i,b_rec.bary.x);
		bary.y.set(i,b_rec.bary.y);
		bary.z.set(i,b_rec.bary.z);
		uv.x.set(i,b_rec.uv.x);
		uv.y.set(i,b_rec.uv.y);
		req_type.set(i,b_rec.req_type);
		req_component.set(i,b_rec.req_component);
		sampled_type.set(i,b_rec.sampled_type);
		sampled_component.set(i,b_rec.sampled_component);
		mode.set(i,b_rec.mode);
	};
};

inline void hit_record8::from_bsdf(const bsdf_record8 &b_rec)
{
	wi = sh_frame.to_world(b_rec.li);
	wo = sh_frame.to_world(b_rec.lo);
};

class direction_record
{
	public:
	vec3f dir = 0.0f;
	float pdf = 0.0f;
	e_measure measure = e_measure(0);

	inline direction_record() {};
	//inline ~direction_record() {};

	inline direction_record(const vec3f &dir_, e_measure m_ = E_SOLID_ANGLE) : dir(dir_), measure(m_) {};

	inline direction_record(const hit_record &rec, e_measure m_ = E_SOLID_ANGLE) : 
		dir(rec.sh_frame.to_local(rec.wi)), measure(m_) {};
};

class direction_record8
{
	public:
	vec3f8 dir = float8(0.0f);
	float8 pdf = float8(0.0f);
	e_measure measure[8] = {e_measure(0)};

	direction_record8() {};

	direction_record8(const vec3f8 d) : dir(d)
	{
		for(int32_t i = 0; i < 8; i++)
			measure[i] = E_SOLID_ANGLE;
	};

	direction_record8(const vec3f8 d, const e_measure m[8]) : dir(d)
	{
		for(int32_t i = 0; i < 8; i++)
			measure[i] = m[i];
	};

	direction_record8(const hit_record8 &rec)
	{
		dir = rec.sh_frame.to_local(rec.wi);
		for(int32_t i = 0; i < 8; i++)
			measure[i] = E_SOLID_ANGLE;
	};

	direction_record8(const hit_record8 &rec, e_measure m[8])
	{
		dir = rec.sh_frame.to_local(rec.wi);
		for(int32_t i = 0; i < 8; i++)
			measure[i] = m[i];
	};
};

//combined position and direct record.
//this may waste a tiny bit of space... but it simplifies everything
class pd_record
{
	public:
	void* object = nullptr;

	vec3f pos  = 0.0f;
	vec3f norm = 0.0f;
	vec3f dir  = 0.0f;
	
	vec3f ref_pos  = 0.0f;
	vec3f ref_norm = 0.0f;

	vec2f uv = 0.0f;

	union
	{
		float dist = 0.0f;
		float length;
	};

	float time = 0.0f;
	float pdf  = 0.0f;
	e_measure measure = e_measure(0);
	bool is_light = false;

	inline pd_record() {};
	//inline ~pd_record() {};

	inline pd_record(const vec3f &ref_, e_measure m_ = E_AREA) : ref_pos(ref_), measure(m_) {};

	inline pd_record(const vec3f &ref_, float time_) : ref_pos(ref_), ref_norm(0.0f), time(time_) {};

	inline pd_record(const hit_record &rec, e_measure measure_ = E_AREA) : 
		object(nullptr), pos(rec.pos), norm(rec.sh_frame.n),
		ref_pos(rec.pos), ref_norm(0.0f), 
		uv(rec.uv), measure(measure_)
	{
		//TODO: redo this so ref norm only set under specific circumstances!
		ref_norm = rec.sh_frame.n;
		object = nullptr;
	};
};

struct pd_record8
{
	void* object[8] = {nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr};

	vec3f8 pos  = vec3f8(0.0f);
	vec3f8 norm = vec3f8(0.0f);
	vec3f8 dir  = vec3f8(0.0f);
	
	vec3f8 ref_pos  = vec3f8(0.0f);
	vec3f8 ref_norm = vec3f8(0.0f);

	vec2f8 uv = vec2f8(0.0f);

	union
	{
		float8 dist = float8(0.0f);
		float8 length;
	};

	float8 time = float8(0.0f);
	float8 pdf = float8(0.0f);
	e_measure measure[8] = {e_measure(0)};
	bool is_light[8] = {false,false,false,false,false,false,false,false};

	pd_record8() {};

	pd_record8(const hit_record8 &rec) : 
		pos(rec.pos), norm(rec.sh_frame.n),
		ref_pos(rec.pos), ref_norm(0.0f), 
		uv(rec.uv)
	{
		for(int i = 0; i < 8; i++)
			measure[i] = E_AREA;
		ref_norm = rec.sh_frame.n;
		for(int i = 0; i < 8; i++)
			object[i] = nullptr;
	};

	pd_record8(const hit_record8 &rec, e_measure measure_[8]) : 
		pos(rec.pos), norm(rec.sh_frame.n),
		ref_pos(rec.pos), ref_norm(0.0f), 
		uv(rec.uv)
	{
		for(int i = 0; i < 8; i++)
			measure[i] = measure_[i];
		ref_norm = rec.sh_frame.n;
		for(int i = 0; i < 8; i++)
			object[i] = nullptr;
	};

	inline pd_record get(int i) const
	{
		pd_record ret;
		ret.object = object[i];
		ret.pos  = vec3f(pos.x[i],pos.y[i],pos.z[i]);
		ret.norm = vec3f(norm.x[i],norm.y[i],norm.z[i]);
		ret.dir  = vec3f(dir.x[i],dir.y[i],dir.z[i]);
		ret.ref_pos  = vec3f(ref_pos.x[i],ref_pos.y[i],ref_pos.z[i]);
		ret.ref_norm = vec3f(ref_norm.x[i],ref_norm.y[i],ref_norm.z[i]);
		ret.uv = vec2f(uv.x[i],uv.y[i]);
		ret.dist = dist[i];
		ret.pdf = pdf[i];
		ret.measure = measure[i];
		ret.is_light = is_light[i];
		return ret;
	};
};