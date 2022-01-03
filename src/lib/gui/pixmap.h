#ifndef PIXMAP_H
#define PIXMAP_H

#include "texture.h"
#include <vector>

class Pixmap
{
public:
	Pixmap(int32_t w, int32_t h);
	int32_t m_w;
	int32_t m_h;
	int32_t m_stride;
	std::vector<uint32_t> m_data;
	std::shared_ptr<Texture> m_texture;
	void upload();
};

#endif
