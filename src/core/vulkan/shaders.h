#pragma once
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include <string>

inline std::vector<uint32_t> read_shader_file(const std::string &path)
{
	std::vector<uint32_t> shader_code = {};
	FILE* shader_file = fopen(path.c_str(),"rb");
	fseek(shader_file,0,SEEK_END);
	shader_code.resize(ftell(shader_file)/sizeof(uint32_t));
	fseek(shader_file,0,SEEK_SET);
	fread(shader_code.data(),shader_code.size()*sizeof(uint32_t),1,shader_file);
	fclose(shader_file);
	return shader_code;
};