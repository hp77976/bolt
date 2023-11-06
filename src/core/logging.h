#pragma once
#include <stdio.h>
#include <stdexcept>
#include <string>

enum log_type
{
	LOG_STATUS,
	LOG_DEBUG,
	LOG_WARN,
	LOG_ERROR
};

inline void log(int type, std::string msg)
{
	if(type == LOG_ERROR)
	{
		throw std::runtime_error("Error: " + msg);
	}
	else
	{
		printf("Log: %s",msg.c_str());
	}
};