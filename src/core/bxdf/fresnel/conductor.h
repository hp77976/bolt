#pragma once
#include "../../math/include.h"
#include "../../spectrum.h"

inline float fr_conductor_exact(const float cti, const float eta, const float k)
{
	float cosThetaI2 = cti*cti;
	float sinThetaI2 = 1-cosThetaI2;
	float sinThetaI4 = sinThetaI2*sinThetaI2;

	float temp1 = eta*eta - k*k - sinThetaI2;
	float a2pb2 = safe_sqrt(temp1*temp1 + 4*k*k*eta*eta);
	float a     = safe_sqrt(0.5f * (a2pb2 + temp1));

	float term1 = a2pb2 + cosThetaI2;
	float term2 = 2*a*cti;

	float Rs2 = (term1 - term2) / (term1 + term2);

	float term3 = a2pb2*cosThetaI2 + sinThetaI4;
	float term4 = term2*sinThetaI2;

	float Rp2 = Rs2 * (term3 - term4) / (term3 + term4);

	return 0.5f * (Rp2 + Rs2);
};