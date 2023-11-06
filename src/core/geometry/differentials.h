#pragma once
#include "../math/include.h"
#include "tri.h"

struct diff_geo
{
	struct bsdf_record_data
	{
		vec3f coords = 0.0f;
		int64_t tri_idx = 0;
	} data;

	vec3f pos  = 0.0f;
	vec3f norm = 0.0f;
	vec3f dpdu = 0.0f, dpdv = 0.0f;
	vec3f dndu = 0.0f, dndv = 0.0f;
	vec3f tangent = 0.0f, bitangent = 0.0f;
	float bt_sign = 0.0f;
	vec2f uv = 0.0f;
	const tri* handle = nullptr;
	bool scattered = false;

	diff_geo() {};

	diff_geo(
		const vec3f &p, const vec3f &nn,
		const vec3f &dpdu_, const vec3f &dpdv_,
		const vec3f &dndu_, const vec3f &dndv_,
		const vec2f &uv_, const tri* t
	) : pos(p), norm(nn), dpdu(dpdu_), dpdv(dpdv_), dndu(dndu_), dndv(dndv_), uv(uv_), handle(t)
	{
		tangent = dpdu;
		bitangent = dpdv;
		bt_sign = 1.0f;
		norm = normalize(cross(dpdu,dpdv));
		scattered = false;
	}

	diff_geo(
		const vec3f &p, const vec3f &n, 
		const vec3f &dpdu_, const vec3f &dpdv_,
		const vec3f &dndu_, const vec3f &dndv_,
		const vec3f &t, const vec3f &bt,
		float bt_sign_, vec2f uv_, const tri* tp_
	) : pos(p), norm(n), dpdu(dpdu_), dpdv(dpdv_), dndu(dndu_), dndv(dndv_), tangent(t), bitangent(bt),
		bt_sign(bt_sign_), uv(uv_), handle(tp_) {scattered = false;};
};