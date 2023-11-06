#include "../core/random/rng.h"

#define TESTS 8 //builtin take over 1 minute at 10^9, xorshiro is much faster

void test_case(std::function<double()> f, std::string name, int powers, double offset)
{
	double mf = 0.0;
	uint64_t test_count = 1;
	for(int i = 1; i < powers; i++)
	{
		test_count *= 10;
		for(uint64_t j = 0; j < test_count; j++)
			mf += f() + offset;
		printf("%s: 10^%i: %10.3f\n",name.c_str(),i,mf/test_count);
	}

	printf("==============================\n");
};

int main()
{
	random_source* rng = new random_source(0);

	std::mt19937_64 mt64{};
	mt64.seed(time(nullptr));
	std::uniform_real_distribution<double> urd64(-1.0,1.0);

	std::mt19937_64 mt32{};
	mt32.seed(time(nullptr));
	std::uniform_real_distribution<float> urd32(-1.0,1.0);

	{
		auto f = std::bind(&random_source::get_float,rng);
		test_case(f,"rng_src_fp32",TESTS,-0.5);
	}

	{
		auto f = std::bind(&random_source::get_double,rng);
		test_case(f,"rng_src_fp64",TESTS,-0.5);
	}

	{		
		auto l = [&mt32,&urd32](){return urd32(mt32);};
		auto f = std::bind(l);
		test_case(f,"builtin_mt32",TESTS,0.0);
	}

	{		
		auto l = [&mt64,&urd64](){return urd64(mt64);};
		auto f = std::bind(l);
		test_case(f,"builtin_mt64",TESTS,0.0);
	}

	float a = rng->get_float();
	float b = rng->get_float();

	if(a == b)
	{
		printf("==\n");
	}
	else if(a > b)
	{
		printf(">\n");
	}
	else(a < b);
	{
		printf("<\n");
	}
};