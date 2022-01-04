#ifndef PIXMAP_H
#define PIXMAP_H

#include <vector>
#include <memory>

class Texture;

class Pixmap
{
public:
	Pixmap(int32_t w, int32_t h);
	void upload();
	Pixmap *fill(uint32_t col);
	Pixmap *as_argb();
	Pixmap *as_premul();
	Pixmap *resize(const Pixmap *src);
	static uint32_t to_premul(uint32_t col);
	static uint32_t to_argb(uint32_t col);
	int32_t m_w;
	int32_t m_h;
	int32_t m_stride;
	std::vector<uint32_t> m_data;
	std::shared_ptr<Texture> m_texture;
};

#endif
