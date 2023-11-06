#pragma once
#include <stdio.h>
#include <stdexcept>

inline void null_throw(void* p, std::string msg)
{
	if(p != nullptr)
		return;

	printf("%s",msg.c_str());
	throw std::runtime_error("Null value error!\n");
};