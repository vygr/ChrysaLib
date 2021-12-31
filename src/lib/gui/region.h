#ifndef REGION_H
#define REGION_H

#include <forward_list>
#include <stdint.h>

//rectangle math
class Rect
{
public:
	Rect(int32_t x, int32_t y, int32_t x1, int32_t y1)
		: m_x(x)
		, m_y(y)
		, m_x1(x1)
		, m_y1(y1)
	{}
	int32_t m_x;
	int32_t m_y;
	int32_t m_x1;
	int32_t m_y1;
};

class Region
{
public:
	Region() {}
	std::forward_list<Rect> m_region;
	Rect bounds();
	Region *free();
	Region *translate(int32_t rx, int32_t ry);
	Region *clip_rect(const Rect &clip);
	Region *remove_rect(const Rect &clip);
	Region *paste_rect(const Rect &clip);
	Region *copy_rect(Region &dest, const Rect &clip);
	Region *cut_rect(Region &dest, const Rect &clip);
	Region *remove_region(Region &dest, int32_t rx, int32_t ry);
	Region *paste_region(Region &dest, int32_t rx, int32_t ry);
	Region *copy_region(Region &dest, const Region &copy, int32_t rx, int32_t ry);
};

#endif
