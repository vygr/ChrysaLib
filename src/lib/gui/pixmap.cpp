#include "pixmap.h"
#include "texture.h"
#include "../services/gui_service.h"
#include <algorithm>

Pixmap::Pixmap(int32_t w, int32_t h)
	: m_w(w)
	, m_h(h)
	, m_stride(w * sizeof(uint32_t))
{
	m_data.resize(w * h);
}

void Pixmap::upload()
{
	Kernel_Service::callback([&]()
	{
		//create SDL surface from pixel buffer, convert to texture
		auto surface =  SDL_CreateRGBSurfaceFrom(&m_data[0], m_w, m_h, 32, m_stride, 0xff0000, 0xff00, 0xff, 0xff000000);
		auto texture = SDL_CreateTextureFromSurface(GUI_Service::m_renderer, surface);
		auto blend_mode = SDL_ComposeCustomBlendMode(
			SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
			SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
		SDL_SetTextureBlendMode(texture, blend_mode);
		SDL_FreeSurface(surface);
		m_texture = std::make_shared<Texture>(texture, m_w, m_h);
	});
}

uint32_t Pixmap::to_premul(uint32_t col)
{
	auto a = col >> 24;
	auto rb = col & 0xff00ff;
	auto g = col & 0xff00;
	rb = ((a * rb) >> 8) & 0xff00ff;
	g = ((a * g) >> 8) & 0xff00;
	a <<= 24;
	return a + rb + g;
}

uint32_t Pixmap::to_argb(uint32_t col)
{
	auto a = col >> 24;
	if (a != 0 && a != 0xff)
	{
		auto d = 256 * 256 / a;
		auto rb = col & 0xff00ff;
		auto g = col & 0xff00;
		rb = ((d * rb) >> 8) & 0xff00ff;
		g = ((d * g) >> 8) & 0xff00;
		col = (a << 24) + rb + g;
	}
	return col;
}

Pixmap *Pixmap::fill(uint32_t col)
{
	std::fill(begin(m_data), end(m_data), to_premul(col));
	return this;
}

Pixmap *Pixmap::as_argb()
{
	uint32_t cache_col = 0;
	uint32_t cache_argb = 0;
	std::transform(begin(m_data), end(m_data), begin(m_data), [&] (auto col)
	{
		if (cache_col != col)
		{
			cache_col = col;
			cache_argb = to_argb(col);
		}
		return cache_argb;
	});
	return this;
}

Pixmap *Pixmap::as_premul()
{
	uint32_t cache_col = 0;
	uint32_t cache_premul = 0;
	std::transform(begin(m_data), end(m_data), begin(m_data), [&] (auto col)
	{
		if (cache_col != col)
		{
			cache_col = col;
			cache_premul = to_premul(col);
		}
		return cache_premul;
	});
	return this;
}

Pixmap *Pixmap::resize(const Pixmap *src)
{
	if ((m_w * 2 == src->m_w) && (m_h * 2 == src->m_h))
	{
		//scale down by 2
		const auto dw = m_w;
		const auto sw = src->m_w;
		const auto agm = 0xff00ff00;
		const auto rbm = 0x00ff00ff;
		auto ddata = &m_data[0];
		auto ddata_end = &ddata[dw * m_h];
		auto sdata = &src->m_data[0];
		while (ddata != ddata_end)
		{
			auto ddata_end_line = &ddata[dw];
			while (ddata != ddata_end_line)
			{
				auto col = sdata[0];
				auto ag = (uint64_t)(col & agm);
				auto rb = col & rbm;

				col = sdata[1];
				sdata += sw;
				ag += col & agm;
				rb += col & rbm;

				col = sdata[0];
				ag += col & agm;
				rb += col & rbm;

				col = sdata[1];
				sdata -= sw - 2;
				ag += col & agm;
				rb += col & rbm;

				ag = (ag >> 2) & agm;
				rb = (rb >> 2) & rbm;
				*ddata++ = ag + rb;
			}
			sdata += sw;
		}
	}
	return this;
}
