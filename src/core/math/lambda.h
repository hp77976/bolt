#pragma once
#include <stdint.h>
#include <cmath>
#include <stdio.h>
#include <functional>

class GaussLobattoIntegrator {
	public:
	typedef std::function<float (float)> Integrand;
		
	public:
	float m_absError, m_relError;
	size_t m_maxEvals;
	bool m_useConvergenceEstimate;
	bool m_warn;
	static const float m_alpha;
	static const float m_beta;
	static const float m_x1;
	static const float m_x2;
	static const float m_x3;

	GaussLobattoIntegrator(size_t maxEvals,
						float absError = 0,
						float relError = 0,
						bool useConvergenceEstimate = true,
						bool warn = true);

	float integrate(const Integrand &f, float a, float b, size_t *evals = NULL) const;

	public:
	float adaptiveGaussLobattoStep(
		const std::function<float (float)>& f,
		float a, float b, float fa, float fb, float is, size_t &evals
	) const;

	float calculateAbsTolerance(
		const std::function<float (float)>& f, float a, float b, size_t &evals
	) const;
};