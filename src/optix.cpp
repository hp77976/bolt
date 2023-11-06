#include <optix.h>
#include <cuda.h>
#define CUDA_UTIL_DONT_USE_GL_INTEROP
#include "../ext/OptiX_Utility/optixu_on_cudau.h"
#include "../ext/OptiX_Utility/optix_util.h"

int main()
{
	CUcontext cu_ctx;
	optixu::Context ctx = optixu::Context::create(0,0,optixu::EnableValidation::False);

};