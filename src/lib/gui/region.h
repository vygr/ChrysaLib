#ifndef REGION_H
#define REGION_H

#include <forward_list>

//rectangle math
class Rect
{
public:
	Rect(int x, int y, int x1, int y1)
		: m_x(x)
		, m_y(y)
		, m_x1(x1)
		, m_y1(y1)
	{}
	int m_x;
	int m_y;
	int m_x1;
	int m_y1;
};

class Region
{
public:
	Region()
	{}
	std::forward_list<Rect> m_region;
	Rect bounds();
	Region *free();
	Region *translate(int rx, int ry);
	Region *clip_rect(const Rect &clip);
	Region *remove_rect(const Rect &clip);
	Region *paste_rect(const Rect &clip);
	Region *copy_rect(Region &dest, const Rect &clip);
	Region *cut_rect(Region &dest, const Rect &clip);
	Region *remove_region(Region &dest, int rx, int ry);
	Region *paste_region(Region &dest, int rx, int ry);
	Region *copy_region(Region &dest, const Region &copy, int rx, int ry);
};

#endif
