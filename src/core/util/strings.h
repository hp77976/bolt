#pragma once
#include <stdio.h>
#include <string>
#include <string.h>
#include <memory>
#include "../logging.h"

inline std::string read_string(FILE* f)
{
	uint32_t char_count = 0;
	fread(&char_count,sizeof(char_count),1,f);

	if(char_count == 0)
		log(LOG_ERROR,"Zero length string!\n");

	char* c_str = new char[char_count];
	fread(c_str,sizeof(char),char_count,f);

	std::string str = std::string(c_str,char_count);

	delete[] c_str;

	return str;
};