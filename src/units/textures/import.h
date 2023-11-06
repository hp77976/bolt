#pragma once
#include "../../core/scene.h"
#include "../../base/texture.h"
#include <stdio.h>

inline color_socket* read_color_socket(FILE* f, scene* s)
{
	uint64_t img_id;
	fread(&img_id,sizeof(img_id),1,f);
	
	float color[4];
	vec3f rgb;
	color_socket* t2 = nullptr;

	if(img_id == 0)
		return new const_color(f);
	
	//the rest of this doesn't work yet
	/*std::string img_name = read_string(f);

	logger::log(LOG_DEBUG,"Attempted to open image: " + img_name + "\n");
	if(mms.find(img_name) == mms.end())
		logger::log(LOG_ERROR,"Failed to find image!\n");

	mipmap* mm = mms.at(img_name);

	char flag;
	fread(&flag,sizeof(flag),1,f);
	//printf("Got flag: %c\n",flag);
	
	bool got_map = false;
	float scale[2] = {1.0f};
	float offset[2] = {0.0f};

	bool got_uv = false;
	std::string uv_name;

	bool got_color = false;

	switch(flag)
	{
		case('m'): //mapping node
		{
			got_map = true;
			fread(scale,sizeof(float),2,f);
			fread(offset,sizeof(float),2,f);
			uv_name = read_string(f);
			uv_map* um = uvs.at(uv_name).second;
			t2 = new texture_2d(mm,vec2f(scale[0],scale[1]),vec2f(offset[0],offset[1]),um);
			break;
		}
		case('u'): //uv node
		{
			got_uv = true;
			uv_name = read_string(f);
			uv_map* um = uvs.at(uv_name).second;
			t2 = new texture_2d(mm,vec2f(scale[0],scale[1]),vec2f(offset[0],offset[1]),um);
			break;
		}
		case('c'): //color
		{
			got_color = true;
			fread(color,sizeof(float),4,f);
			rgb.r = color[0]; rgb.g = color[1]; rgb.b = color[2];
			t2 = new texture_2d(rgb);
			break;
		}
		default:
			throw std::runtime_error("Unknown char encountered!\n");
	}*/

	return t2;
};