#ifndef TEXTURE_H
#define TEXTURE_H

#include "../services/kernel_service.h"

extern void host_gui_destroy_texture(void *t);

class Texture
{
public:
	Texture(void *handle, int32_t w, int32_t h)
		: m_handle(handle)
		, m_w(w)
		, m_h(h)
	{}
	~Texture()
	{
		Kernel_Service::callback([&]()
		{
			host_gui_destroy_texture(m_handle);
		});
	}
	void *m_handle;
	int32_t m_w;
	int32_t m_h;
};

#endif
