#include "pixmap.h"
#include "../services/gui_service.h"

Pixmap::Pixmap(int32_t w, int32_t h)
	: m_w(w)
	, m_h(h)
	, m_stride(w * h)
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
