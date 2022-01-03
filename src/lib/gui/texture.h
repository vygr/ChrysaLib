#ifndef TEXTURE_H
#define TEXTURE_H

#include "../services/kernel_service.h"
#include <SDL.h>

class Texture
{
public:
	Texture(SDL_Texture *handle, int32_t w, int32_t h)
		: m_handle(handle)
		, m_w(w)
		, m_h(h)
	{}
	~Texture()
	{
		Kernel_Service::callback([&]()
		{
			SDL_Destroy_Texture(m_handle);
		});
	}
	SDL_Texture *m_handle;
	int32_t m_w;
	int32_t m_h;
};

#endif
