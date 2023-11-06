#pragma once
#include <vector>

struct disc_dist
{
	std::vector<float> m_cdf;
	float m_sum, m_normalization;
	bool m_normalized;

	explicit inline disc_dist(int entries = 0)
	{
		reserve(entries);
		clear();
	};

	inline void clear()
	{
		m_cdf.clear();
		m_cdf.push_back(0.0f);
		m_normalized = true;
	};

	inline void reserve(int size) {m_cdf.reserve(size+1);};

	inline void append(float pdf_val) {m_cdf.push_back(m_cdf[m_cdf.size()-1]+pdf_val);};

	inline float operator[](int i) const {return m_cdf[i+1] - m_cdf[i];};

	inline bool is_normalized() const {return m_normalized;};

	inline float normalize()
	{
		m_sum = m_cdf[m_cdf.size()-1];
		if(m_sum > 0.0f)
		{
			m_normalization = 1.0f / m_sum;

			for(int i = 0; i < (int)m_cdf.size(); ++i)
				m_cdf[i] *= m_normalization;

			m_cdf[m_cdf.size()-1] = 1.0f;
			m_normalized = true;
		}
		else
		{
			m_normalization = 0.0f;
		}

		return m_sum;
	};

	inline int sample(float v) const
	{
		std::vector<float>::const_iterator entry =
			std::lower_bound(m_cdf.begin(),m_cdf.end(),v);
		
		int idx = std::min((int)m_cdf.size()-2,(int)std::max((long)0,entry-m_cdf.begin()-1));

		while(operator[](idx) == 0 && idx < (int)m_cdf.size() - 1)
			idx++;
		
		return idx;
	};
};