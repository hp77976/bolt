#include "film.h"
#include "../base/filter.h"
#include "out_text.h"

#include "../../ext/Imath/src/Imath/ImathVec.h"
//this has an error for some reason, i'm not sure why. it compiles though
#include "../../ext/openexr/src/lib/OpenEXR/ImfRgbaFile.h"
#include "../../ext/openexr/src/lib/OpenEXR/ImfOutputFile.h"
#include "../../ext/openexr/src/lib/OpenEXR/ImfRgba.h"

void film::splat(const spectrum &s, const vec2f &p)
{
	vec2f pos = p - 0.5f;
	
	//TODO: make film bigger by a few px so min/max don't need to be done
	// this would allow for splatting pixels on without regard for being
	// near the edges and would allow for this to be heavily vectorized

	vec2i min = vec2i(
		std::max((int)std::ceil(pos.x - 1.0f),0),
		std::max((int)std::ceil(pos.y - 1.0f),0)
	);
	vec2i max = vec2i(
		std::min((int)std::floor(pos.x + 1.0f),m_size.x-1),
		std::min((int)std::floor(pos.y + 1.0f),m_size.y-1)
	);

	alignas(32) float m_weights_x[32] = {0.0f};
	alignas(32) float m_weights_y[32] = {0.0f};

	for(int x = min.x, idx = 0; x <= max.x; ++x)
		m_weights_x[idx++] = m_filter->eval_discretized(x - pos.x);
		//5,5 -> [[4,4],[6,6], ([4->6] - 5) -1 0 1
	
	for(int y = min.y, idx = 0; y <= max.y; ++y)
		m_weights_y[idx++] = m_filter->eval_discretized(y - pos.y);

	alignas(32) float final_weights[32] = {0.0f};
	int final_px[32] = {0};
	int final_count = 0;

	for(int y = min.y, yr = 0; y <= max.y; ++y, ++yr)
	{
		float weight_y = m_weights_y[yr];
		int px = y * m_size.x + min.x;
		
		for(int x = min.x, xr = 0; x <= max.x; ++x, ++xr)
		{
			float weight = m_weights_x[xr] * weight_y;
			w[px] += weight;

			final_weights[final_count] = weight;
			final_px[final_count] = px;
			final_count++;
		}
	}

	//moved this outside of the above loop. helps speed a *tiny* bit.
	//this gives an 18% boost in performance for this specific function
	for(int i = 0; i < final_count; i++)
		c[final_px[i]] += final_weights[i] * s;
};

void write_exr(const char* path, int width, int height, vec3f* const rgb, bool write_info)
{
	Imf::Rgba* pixels = new Imf::Rgba[width * height];

	for(int i = 0; i < width * height; i++)
	{
		pixels[i].r = std::max(rgb[i].r,0.0f);
		pixels[i].g = std::max(rgb[i].g,0.0f);
		pixels[i].b = std::max(rgb[i].b,0.0f);
			
		pixels[i].a = 1.0f;
	}

	if(write_info)
	{
		//draw a black box across the top
		for(int y = 0; y < 16; y++)
		{
			for(int x = 0; x < 256; x++)
			{
				pixels[x+(y*width)].r = 0.0f;
				pixels[x+(y*width)].g = 0.0f;
				pixels[x+(y*width)].b = 0.0f;
			}
		}

		std::string render_text = "rendered in bolt.";
		write_text_on_film(render_text,pixels,0,width,height);
	}
	
	Imf::RgbaOutputFile file (path, width, height, Imf::WRITE_RGBA);
	file.setFrameBuffer(pixels,1,width);
	file.writePixels(height);

	delete[] pixels; pixels = nullptr;

	printf("Wrote %i pixels to file.\n",width * height);
};

void film::write(const char* path, bool write_info)
{
	vec3f* rgb = new vec3f[m_size.x * m_size.y];
	
	for(int i = 0; i < (m_size.x * m_size.y); i++)
		rgb[i] = spectrum_to_linear_rgb(c[i]);

	write_exr("output.exr",m_size.x,m_size.y,rgb,write_info);
};

void write_film_stupid(std::string path, std::vector<float> data, uint32_t w, uint32_t h)
{
	std::vector<Imf::Rgba> pixels = {};

	for(int32_t i = 0; i < w * h; i++)
	{
		float r = data[i*4+0];
		float g = data[i*4+1];
		float b = data[i*4+2];
		float a = data[i*4+3];
		Imf::Rgba pixel = Imf::Rgba(r,g,b,a);
		pixels.push_back(pixel);
	}
	printf("wrote: %lu pixels\n",pixels.size());
	Imf::RgbaOutputFile file(path.c_str(), w, h, Imf::WRITE_RGBA);
	file.setFrameBuffer(pixels.data(),1,h);
	file.writePixels(w);
};