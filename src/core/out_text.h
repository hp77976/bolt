#pragma once
#include <stdint.h>
#include <string>
#include "math/include.h"

#include "../../ext/Imath/src/Imath/ImathVec.h"
//this has an error for some reason, i'm not sure why. it compiles though
#include "../../ext/openexr/src/lib/OpenEXR/ImfRgbaFile.h"
#include "../../ext/openexr/src/lib/OpenEXR/ImfOutputFile.h"
#include "../../ext/openexr/src/lib/OpenEXR/ImfRgba.h"

inline int xytoi(int width, int x, int y)
{
	return x+(y*width);
};

inline void write_px(Imf::Rgba* pxs, int idx, vec3f val)
{
	pxs[idx].r = val.x;
	pxs[idx].g = val.y;
	pxs[idx].b = val.z;
};

struct px_char
{
	int8_t data[7*12] = {
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0
	};

	static px_char to_pc(std::string rr[12])
	{
		px_char ret;
		for(int y = 0; y < 12; y++)
			for(int x = 0; x < 7; x++)
				ret.data[x+(y*7)] = rr[y].at(x) == 'f' ? 1 : 0;
		return ret;
	};

	static px_char space()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       "; 
		rr[ 3] = "       ";
		rr[ 4] = "       ";
		rr[ 5] = "       ";
		rr[ 6] = "       ";
		rr[ 7] = "       ";
		rr[ 8] = "       ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char period()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       "; 
		rr[ 3] = "       ";
		rr[ 4] = "       ";
		rr[ 5] = "       ";
		rr[ 6] = "       ";
		rr[ 7] = "   ff  ";
		rr[ 8] = "   ff  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char colon()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       "; 
		rr[ 3] = "   ff  ";
		rr[ 4] = "   ff  ";
		rr[ 5] = "       ";
		rr[ 6] = "       ";
		rr[ 7] = "   ff  ";
		rr[ 8] = "   ff  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n0()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "   ff  ";
		rr[ 2] = "  f  f ";
		rr[ 3] = " f   ff";
		rr[ 4] = " f  f f";
		rr[ 5] = " f f  f";
		rr[ 6] = " ff   f";
		rr[ 7] = "  f  f ";
		rr[ 8] = "   ff  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n1()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "   f   ";
		rr[ 2] = "  ff   ";
		rr[ 3] = " f f   ";
		rr[ 4] = "   f   ";
		rr[ 5] = "   f   ";
		rr[ 6] = "   f   ";
		rr[ 7] = "   f   ";
		rr[ 8] = " fffff ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n2()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "  ffff ";
		rr[ 2] = " f    f";
		rr[ 3] = "      f";
		rr[ 4] = "      f";
		rr[ 5] = "     f ";
		rr[ 6] = "   ff  ";
		rr[ 7] = "  f    ";
		rr[ 8] = " ffffff";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n3()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "  ffff ";
		rr[ 2] = " f    f";
		rr[ 3] = "      f";
		rr[ 4] = "    ff ";
		rr[ 5] = "      f";
		rr[ 6] = "      f";
		rr[ 7] = " f    f";
		rr[ 8] = "  ffff ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n4()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = " f   f ";
		rr[ 2] = " f   f ";
		rr[ 3] = " f   f ";
		rr[ 4] = " ffffff";
		rr[ 5] = "     f ";
		rr[ 6] = "     f ";
		rr[ 7] = "     f ";
		rr[ 8] = "     f ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n5()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = " fffff ";
		rr[ 2] = " f     ";
		rr[ 3] = " f     ";
		rr[ 4] = " ffff  ";
		rr[ 5] = "     f ";
		rr[ 6] = "     f ";
		rr[ 7] = " f   f ";
		rr[ 8] = "  fff  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n6()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "   ff  ";
		rr[ 2] = "  f    ";
		rr[ 3] = " f     ";
		rr[ 4] = " ffff  ";
		rr[ 5] = " f   f ";
		rr[ 6] = " f   f ";
		rr[ 7] = " f   f ";
		rr[ 8] = "  fff  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n7()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = " ffffff";
		rr[ 2] = "     ff";
		rr[ 3] = "     f ";
		rr[ 4] = "    f  ";
		rr[ 5] = "    f  ";
		rr[ 6] = "   f   ";
		rr[ 7] = "   f   ";
		rr[ 8] = "   f   ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n8()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "   ff  ";
		rr[ 2] = "  f  f ";
		rr[ 3] = "  f  f ";
		rr[ 4] = "   ff  ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "   ff  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n9()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "  fff  ";
		rr[ 2] = " f   f ";
		rr[ 3] = " f   f ";
		rr[ 4] = "  ffff ";
		rr[ 5] = "     f ";
		rr[ 6] = "    f  ";
		rr[ 7] = "    f  ";
		rr[ 8] = "   f   ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char a()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       ";
		rr[ 3] = "  ff   ";
		rr[ 4] = " f  f  ";
		rr[ 5] = "  fff  ";
		rr[ 6] = " f  f  ";
		rr[ 7] = " f  f  ";
		rr[ 8] = "  ff f ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char b()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "  f    ";
		rr[ 2] = "  f    ";
		rr[ 3] = "  f    ";
		rr[ 4] = "  fff  ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "  fff  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char c()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "   fff ";
		rr[ 5] = "  f    ";
		rr[ 6] = "  f    ";
		rr[ 7] = "  f    ";
		rr[ 8] = "   fff ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char d()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "     f ";
		rr[ 2] = "     f ";
		rr[ 3] = "     f ";
		rr[ 4] = "   fff ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "   fff ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char e()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "   ff  ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  ffff ";
		rr[ 7] = "  f    ";
		rr[ 8] = "   fff ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char f()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "    ff ";
		rr[ 2] = "   f   ";
		rr[ 3] = "   f   ";
		rr[ 4] = "  fff  ";
		rr[ 5] = "   f   ";
		rr[ 6] = "   f   ";
		rr[ 7] = "   f   ";
		rr[ 8] = "   f   ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char g()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "  fff  ";
		rr[ 5] = " f   f ";
		rr[ 6] = " f   f ";
		rr[ 7] = " f   f ";
		rr[ 8] = "  ffff ";
		rr[ 9] = "     f ";
		rr[10] = " f   f ";
		rr[11] = "  fff  ";
		return to_pc(rr);
	};

	static px_char h()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "  f    ";
		rr[ 2] = "  f    ";
		rr[ 3] = "  f    ";
		rr[ 4] = "  fff  ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "  f  f ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char i()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       ";
		rr[ 3] = "   f   ";
		rr[ 4] = "       ";
		rr[ 5] = "   f   ";
		rr[ 6] = "   f   ";
		rr[ 7] = "   f   ";
		rr[ 8] = "   f   ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char j()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "   f   ";
		rr[ 3] = "       ";
		rr[ 4] = "   f   ";
		rr[ 5] = "   f   ";
		rr[ 6] = "   f   ";
		rr[ 7] = "   f   ";
		rr[ 8] = "   f   ";
		rr[ 9] = "   f   ";
		rr[10] = " ff    ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char k()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "  f    ";
		rr[ 2] = "  f    ";
		rr[ 3] = "  f  f ";
		rr[ 4] = "  f f  ";
		rr[ 5] = "  ff   ";
		rr[ 6] = "  f f  ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "  f  f ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char l()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "   f   "; 
		rr[ 2] = "   f   ";
		rr[ 3] = "   f   ";
		rr[ 4] = "   f   ";
		rr[ 5] = "   f   ";
		rr[ 6] = "   f   ";
		rr[ 7] = "   f   ";
		rr[ 8] = "    f  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char m()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = " ff ff ";
		rr[ 5] = "f  f  f";
		rr[ 6] = "f  f  f";
		rr[ 7] = "f  f  f";
		rr[ 8] = "f  f  f";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char n()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "  fff  ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "  f  f ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char o()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "   ff  ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "   ff  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char p()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "   ff  ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "  fff  ";
		rr[ 9] = "  f    ";
		rr[10] = "  f    ";
		rr[11] = "  f    ";
		return to_pc(rr);
	};

	static px_char q()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "   ff  ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "   fff ";
		rr[ 9] = "     f ";
		rr[10] = "     f ";
		rr[11] = "     f ";
		return to_pc(rr);
	};

	static px_char r()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "  f ff ";
		rr[ 5] = "  ff   ";
		rr[ 6] = "  f    ";
		rr[ 7] = "  f    ";
		rr[ 8] = "  f    ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char s()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = "   ff  ";
		rr[ 5] = "  f    ";
		rr[ 6] = "   f   ";
		rr[ 7] = "    f  ";
		rr[ 8] = "  ff   ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char t()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "   f   "; 
		rr[ 3] = "   f   ";
		rr[ 4] = " fffff ";
		rr[ 5] = "   f   ";
		rr[ 6] = "   f   ";
		rr[ 7] = "   f   ";
		rr[ 8] = "   f   ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char u()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = " f  f  ";
		rr[ 5] = " f  f  ";
		rr[ 6] = " f  f  ";
		rr[ 7] = " f  f  ";
		rr[ 8] = "  ff f ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char v()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = " f   f ";
		rr[ 5] = " f   f ";
		rr[ 6] = "  f f  ";
		rr[ 7] = "  f f  ";
		rr[ 8] = "   f   ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char w()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = " f f f ";
		rr[ 5] = " f f f ";
		rr[ 6] = " f f f ";
		rr[ 7] = " f f f ";
		rr[ 8] = "  f f  ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char x()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = " f   f ";
		rr[ 5] = "  f f  ";
		rr[ 6] = "   f   ";
		rr[ 7] = "  f f  ";
		rr[ 8] = " f   f ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static px_char y()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       ";
		rr[ 2] = "       "; 
		rr[ 3] = "       ";
		rr[ 4] = "  f  f ";
		rr[ 5] = "  f  f ";
		rr[ 6] = "  f  f ";
		rr[ 7] = "  f  f ";
		rr[ 8] = "   fff ";
		rr[ 9] = "     f ";
		rr[10] = "     f ";
		rr[11] = "  fff  ";
		return to_pc(rr);
	};

	static px_char z()
	{
		std::string rr[12];
		rr[ 0] = "       ";
		rr[ 1] = "       "; 
		rr[ 2] = "       ";
		rr[ 3] = "       ";
		rr[ 4] = " fffff ";
		rr[ 5] = "    f  ";
		rr[ 6] = "   f   ";
		rr[ 7] = "  f    ";
		rr[ 8] = " fffff ";
		rr[ 9] = "       ";
		rr[10] = "       ";
		rr[11] = "       ";
		return to_pc(rr);
	};

	static void to_px(px_char pc, Imf::Rgba* px, int position, int width, int height)
	{
		for(int y = 0; y < 12; y++)
			for(int x = 0; x < 7; x++)
				write_px(px,x+(position*8)+y*width,(float)pc.data[x+y*7]);
	};
};

inline void write_letter(char c, Imf::Rgba* px, int width, int height, int position)
{
	px_char pc;
	switch(c)
	{
		case('a'):
			pc = px_char::a(); break;
		case('b'):
			pc = px_char::b(); break;
		case('c'):
			pc = px_char::c(); break;
		case('d'):
			pc = px_char::d(); break;
		case('e'):
			pc = px_char::e(); break;
		case('f'):
			pc = px_char::f(); break;
		case('g'):
			pc = px_char::g(); break;
		case('h'):
			pc = px_char::h(); break;
		case('i'):
			pc = px_char::i(); break;
		case('j'):
			pc = px_char::j(); break;
		case('k'):
			pc = px_char::k(); break;
		case('l'):
			pc = px_char::l(); break;
		case('m'):
			pc = px_char::m(); break;
		case('n'):
			pc = px_char::n(); break;
		case('o'):
			pc = px_char::o(); break;
		case('p'):
			pc = px_char::p(); break;
		case('q'):
			pc = px_char::q(); break;
		case('r'):
			pc = px_char::r(); break;
		case('s'):
			pc = px_char::s(); break;
		case('t'):
			pc = px_char::t(); break;
		case('u'):
			pc = px_char::u(); break;
		case('v'):
			pc = px_char::v(); break;
		case('w'):
			pc = px_char::w(); break;
		case('x'):
			pc = px_char::x(); break;
		case('y'):
			pc = px_char::y(); break;
		case('z'):
			pc = px_char::z(); break;
		case('0'):
			pc = px_char::n0(); break;
		case('1'):
			pc = px_char::n1(); break;
		case('2'):
			pc = px_char::n2(); break;
		case('3'):
			pc = px_char::n3(); break;
		case('4'):
			pc = px_char::n4(); break;
		case('5'):
			pc = px_char::n5(); break;
		case('6'):
			pc = px_char::n6(); break;
		case('7'):
			pc = px_char::n7(); break;
		case('8'):
			pc = px_char::n8(); break;
		case('9'):
			pc = px_char::n9(); break;
		case(' '):
			pc = px_char::space(); break;
		case('.'):
			pc = px_char::period(); break;
		case(':'):
			pc = px_char::colon(); break;
		case('\0'):
			break;
		case('\n'):
			break;
		default:
			throw std::runtime_error("Unsupported Character!\n");
	}

	px_char::to_px(pc,px,position,width,height);
};

void write_text_on_film(std::string text, Imf::Rgba* px, int pos, int w, int h)
{
	for(uint64_t i = 0; i < text.size(); i++)
	{
		write_letter(text.at(i),px,w,h,pos);
		pos++;
	}
};