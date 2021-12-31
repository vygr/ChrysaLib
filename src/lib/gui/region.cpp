#include "region.h"

#include <algorithm>
#include <assert.h>

Region *Region::free()
{
	//clear region
	m_region.clear();
	return this;
}

Region *Region::translate(int32_t rx, int32_t ry)
{
	//translate all rects
	for (auto &rect : m_region)
	{
		rect.m_x += rx;
		rect.m_y += ry;
		rect.m_x1 += rx;
		rect.m_y1 += ry;
	}
	return this;
}

Rect Region::bounds()
{
	//bounds of region, else null Rect
	if (m_region.empty()) return Rect(0,0,0,0);
	auto itr = begin(m_region);
	auto bounds = *itr;
	std::for_each (std::next(itr), end(m_region), [&] (const auto &rect)
	{
		bounds.m_x = std::min(bounds.m_x, rect.m_x);
		bounds.m_y = std::min(bounds.m_y, rect.m_y);
		bounds.m_x1 = std::max(bounds.m_x1, rect.m_x1);
		bounds.m_y1 = std::max(bounds.m_y1, rect.m_y1);
	});
	return bounds;
}

Region *Region::clip_rect(const Rect &clip)
{
	//clip region to rect
	assert(clip.m_x1 > clip.m_x);
	assert(clip.m_y1 > clip.m_y);
	auto itr = m_region.before_begin();
	for (;;)
	{
		auto last_itr = itr;
		if (++itr == end(m_region)) break;

		auto &rect = *itr;
		if (rect.m_x >= clip.m_x1
			|| rect.m_y >= clip.m_y1
			|| rect.m_x1 <= clip.m_x
			|| rect.m_y1 <= clip.m_y)
		{
			//rect is outside clip area
			m_region.erase_after(last_itr);
			itr = last_itr;
		}
		else
		{
			//intersetion
			rect.m_x = std::max(rect.m_x, clip.m_x);
			rect.m_y = std::max(rect.m_y, clip.m_y);
			rect.m_x1 = std::min(rect.m_x1, clip.m_x1);
			rect.m_y1 = std::min(rect.m_y1, clip.m_y1);
		}
	}
	return this;
}

Region *Region::copy_rect(Region &dest, const Rect &clip)
{
	assert(clip.m_x1 > clip.m_x);
	assert(clip.m_y1 > clip.m_y);
	Rect *new_rect;
	auto itr = m_region.before_begin();
	for (;;)
	{
		auto last_itr = itr;
		if (++itr == end(m_region)) break;

		auto &rect = *itr;
		if (rect.m_x >= clip.m_x1
			|| rect.m_y >= clip.m_y1
			|| rect.m_x1 <= clip.m_x
			|| rect.m_y1 <= clip.m_y)
		{
			//rect is outside clip area
			continue;
		}
		//new rect
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();

		//jump to correct splitting code
		if (rect.m_x >= clip.m_x) goto copy_split1;
		if (rect.m_y >= clip.m_y) goto copy_split2;
		if (clip.m_x1 >= rect.m_x1) goto copy_split4;
		if (clip.m_y1 >= rect.m_y1) goto copy_xyx1;

	copy_xyx1y1:
		//clip.m_x + clip.m_y + clip.m_x1 + clip.m_y1 inside
		new_rect->m_x = clip.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = clip.m_y1;
		continue;

	copy_split1:
		//jump to correct splitting code
		if (rect.m_y >= clip.m_y) goto copy_split3;
		if (clip.m_x1 >= rect.m_x1) goto copy_split5;
		if (clip.m_y1 >= rect.m_y1) goto copy_yx1;

	copy_yx1y1:
		//clip.m_y + clip.m_x1 + clip.m_y1 inside
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = clip.m_y1;
		continue;

	copy_split2:
		//jump to correct splitting code
		if (clip.m_x1 >= rect.m_x1) goto copy_split6;
		if (clip.m_y1 >= rect.m_y1) goto copy_xx1;

	copy_xx1y1:
		//clip.m_x + clip.m_x1 + clip.m_y1 inside
		new_rect->m_x = clip.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = clip.m_y1;
		continue;

	copy_split3:
		//jump to correct splitting code
		if (clip.m_x1 >= rect.m_x1) goto copy_split7;
		if (clip.m_y1 >= rect.m_y1) goto copy_x1;

	copy_x1y1:
		//clip.m_x1 + clip.m_y1 inside
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = clip.m_y1;
		continue;

	copy_split4:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto copy_xy;

	copy_xyy1:
		//clip.m_x + clip.m_y + clip.m_y1 inside
		new_rect->m_x = clip.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		continue;

	copy_split5:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto copy_y;

	copy_yy1:
		//clip.m_y + clip.m_y1 inside
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		continue;

	copy_split6:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto copy_x;

	copy_xy1:
		//clip.m_x + clip.m_y1 inside
		new_rect->m_x = clip.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		continue;

	copy_split7:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto copy_encl;

	copy_y1:
		//clip.m_y1 inside
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		continue;

	copy_xyx1:
		//clip.m_x + clip.m_y + clip.m_x1 inside
		new_rect->m_x = clip.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = rect.m_y1;
		continue;

	copy_encl:
		//region is enclosed
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		continue;

	copy_x:
		//clip.m_x inside
		new_rect->m_x = clip.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		continue;

	copy_y:
		//clip.m_y inside
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		continue;

	copy_xy:
		//clip.m_x + clip.m_y inside
		new_rect->m_x = clip.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		continue;

	copy_x1:
		//clip.m_x1 inside
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = rect.m_y1;
		continue;

	copy_xx1:
		//clip.m_x + clip.m_x1 inside
		new_rect->m_x = clip.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = rect.m_y1;
		continue;

	copy_yx1:
		//clip.m_y + clip.m_x1 inside
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = rect.m_y1;
	}
	return this;
}

Region *Region::cut_rect(Region &dest, const Rect &r)
{
	assert(r.m_x1 > r.m_x);
	assert(r.m_y1 > r.m_y);
	Rect *new_rect;
	Rect clip = r;
	auto itr = m_region.before_begin();
	for (;;)
	{
		auto last_itr = itr;
		if (++itr == end(m_region)) break;

		auto &rect = *itr;
		if (rect.m_x >= clip.m_x1
			|| rect.m_y >= clip.m_y1
			|| rect.m_x1 <= clip.m_x
			|| rect.m_y1 <= clip.m_y)
		{
			//rect is outside clip area
			continue;
		}

		//jump to correct splitting code
		if (rect.m_x >= clip.m_x) goto cut_split1;
		if (rect.m_y >= clip.m_y) goto cut_split2;
		if (clip.m_x1 >= rect.m_x1) goto cut_split4;
		if (clip.m_y1 >= rect.m_y1) goto cut_xyx1;

	cut_xyx1y1:
		//clip.m_x + clip.m_y + clip.m_x1 + clip.m_y1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = clip.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	cut_split1:
		//jump to correct splitting code
		if (rect.m_y >= clip.m_y) goto cut_split3;
		if (clip.m_x1 >= rect.m_x1) goto cut_split5;
		if (clip.m_y1 >= rect.m_y1) goto cut_yx1;

	cut_yx1y1:
		//clip.m_y + clip.m_x1 + clip.m_y1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	cut_split2:
		//jump to correct splitting code
		if (clip.m_x1 >= rect.m_x1) goto cut_split6;
		if (clip.m_y1 >= rect.m_y1) goto cut_xx1;

	cut_xx1y1:
		//clip.m_x + clip.m_x1 + clip.m_y1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = clip.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	cut_split3:
		//jump to correct splitting code
		if (clip.m_x1 >= rect.m_x1) goto cut_split7;
		if (clip.m_y1 >= rect.m_y1) goto cut_x1;

	cut_x1y1:
		//clip.m_x1 + clip.m_y1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	cut_split4:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto cut_xy;

	cut_xyy1:
		//clip.m_x + clip.m_y + clip.m_y1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = clip.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	cut_split5:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto cut_y;

	cut_yy1:
		//clip.m_y + clip.m_y1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	cut_split6:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto cut_x;

	cut_xy1:
		//clip.m_x + clip.m_y1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = clip.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	cut_split7:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto cut_encl;

	cut_y1:
		//clip.m_y1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	cut_xyx1:
		//clip.m_x + clip.m_y + clip.m_x1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = clip.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//left part
		rect.m_y = clip.m_y;
		rect.m_x1 = clip.m_x;
		continue;

	cut_encl:
		//region is enclosed
		dest.m_region.push_front(rect);
		m_region.erase_after(last_itr);
		itr = last_itr;
		continue;

	cut_x:
		//clip.m_x inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = clip.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//left part
		rect.m_x1 = clip.m_x;
		continue;

	cut_y:
		//clip.m_y inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//top part
		rect.m_y1 = clip.m_y;
		continue;

	cut_xy:
		//clip.m_x + clip.m_y inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = clip.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//left part
		rect.m_y = clip.m_y;
		rect.m_x1 = clip.m_x;
		continue;

	cut_x1:
		//clip.m_x1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//right part
		rect.m_x = clip.m_x1;
		continue;

	cut_xx1:
		//clip.m_x + clip.m_x1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = clip.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = rect.m_y1;
		//right part
		rect.m_x = clip.m_x1;
		continue;

	cut_yx1:
		//clip.m_y + clip.m_x1 inside
		//cut part
		dest.m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &dest.m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//right part
		rect.m_x = clip.m_x1;
		rect.m_y = clip.m_y;
	}
	return this;
}

Region *Region::remove_rect(const Rect &clip)
{
	assert(clip.m_x1 > clip.m_x);
	assert(clip.m_y1 > clip.m_y);
	Rect *new_rect;
	auto itr = m_region.before_begin();
	for (;;)
	{
		auto last_itr = itr;
		if (++itr == end(m_region)) break;

		auto &rect = *itr;
		if (rect.m_x >= clip.m_x1
			|| rect.m_y >= clip.m_y1
			|| rect.m_x1 <= clip.m_x
			|| rect.m_y1 <= clip.m_y)
		{
			//rect is outside clip area
			continue;
		}

		//jump to correct splitting code
		if (rect.m_x >= clip.m_x) goto rem_split1;
		if (rect.m_y >= clip.m_y) goto rem_split2;
		if (clip.m_x1 >= rect.m_x1) goto rem_split4;
		if (clip.m_y1 >= rect.m_y1) goto rem_xyx1;

	rem_xyx1y1:
		//clip.m_x + clip.m_y + clip.m_x1 + clip.m_y1 inside
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	rem_split1:
		//jump to correct splitting code
		if (rect.m_y >= clip.m_y) goto rem_split3;
		if (clip.m_x1 >= rect.m_x1) goto rem_split5;
		if (clip.m_y1 >= rect.m_y1) goto rem_yx1;

	rem_yx1y1:
		//clip.m_y + clip.m_x1 + clip.m_y1 inside
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	rem_split2:
		//jump to correct splitting code
		if (clip.m_x1 >= rect.m_x1) goto rem_split6;
		if (clip.m_y1 >= rect.m_y1) goto rem_xx1;

	rem_xx1y1:
		//clip.m_x + clip.m_x1 + clip.m_y1 inside
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	rem_split3:
		//jump to correct splitting code
		if (clip.m_x1 >= rect.m_x1) goto rem_split7;
		if (clip.m_y1 >= rect.m_y1) goto rem_x1;

	rem_x1y1:
		//clip.m_x1 + clip.m_y1 inside
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	rem_split4:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto rem_xy;

	rem_xyy1:
		//clip.m_x + clip.m_y + clip.m_y1 inside
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	rem_split5:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto rem_y;

	rem_yy1:
		//clip.m_y + clip.m_y1 inside
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	rem_split6:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto rem_x;

	rem_xy1:
		//clip.m_x + clip.m_y1 inside
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	rem_split7:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto rem_encl;

	rem_y1:
		//clip.m_y1 inside
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	rem_xyx1:
		//clip.m_x + clip.m_y + clip.m_x1 inside
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = clip.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = rect.m_y1;
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//left part
		rect.m_y = clip.m_y;
		rect.m_x1 = clip.m_x;
		continue;

	rem_encl:
		//region is enclosed
		m_region.erase_after(last_itr);
		itr = last_itr;
		continue;

	rem_x:
		//clip.m_x inside
		//left part
		rect.m_x1 = clip.m_x;
		continue;

	rem_y:
		//clip.m_y inside
		//top part
		rect.m_y1 = clip.m_y;
		continue;

	rem_xy:
		//clip.m_x + clip.m_y inside
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//left part
		rect.m_y = clip.m_y;
		rect.m_x1 = clip.m_x;
		continue;

	rem_x1:
		//clip.m_x1 inside
		//right part
		rect.m_x = clip.m_x1;
		continue;

	rem_xx1:
		//clip.m_x + clip.m_x1 inside
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = rect.m_y1;
		//right part
		rect.m_x = clip.m_x1;
		continue;

	rem_yx1:
		//clip.m_y + clip.m_x1 inside
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//right part
		rect.m_x = clip.m_x1;
		rect.m_y = clip.m_y;
	}
	return this;
}

Region *Region::paste_rect(const Rect &r)
{
	assert(r.m_x1 > r.m_x);
	assert(r.m_y1 > r.m_y);
	Rect *new_rect;
	Rect clip = r;
	auto itr = m_region.before_begin();
	for (;;)
	{
		auto last_itr = itr;
		if (++itr == end(m_region)) break;

		auto &rect = *itr;
		if (rect.m_x >= clip.m_x1
			|| rect.m_y >= clip.m_y1
			|| clip.m_x >= rect.m_x1
			|| clip.m_y >= rect.m_y1) continue;

		//jump to correct splitting code
		if (rect.m_x >= clip.m_x) goto paste_split1;
		if (rect.m_y >= clip.m_y) goto paste_split2;
		if (clip.m_x1 >= rect.m_x1) goto paste_split4;
		if (clip.m_y1 >= rect.m_y1) goto paste_xyx1;

	paste_xyx1y1:
		//clip.m_x + clip.m_y + clip.m_x1 + clip.m_y1 inside
		return this;

	paste_split1:
		//jump to correct splitting code
		if (rect.m_y >= clip.m_y) goto paste_split3;
		if (clip.m_x1 >= rect.m_x1) goto paste_split5;
		if (clip.m_y1 >= rect.m_y1) goto paste_yx1;

	paste_yx1y1:
		//clip.m_y + clip.m_x1 + clip.m_y1 inside
		clip.m_x1 = rect.m_x;
		continue;

	paste_split2:
		//jump to correct splitting code
		if (clip.m_x1 >= rect.m_x1) goto paste_split6;
		if (clip.m_y1 >= rect.m_y1) goto paste_xx1;

	paste_xx1y1:
		//clip.m_x + clip.m_x1 + clip.m_y1 inside
		clip.m_y1 = rect.m_y;
		continue;

	paste_split3:
		//jump to correct splitting code
		if (clip.m_x1 >= rect.m_x1) goto paste_split7;
		if (clip.m_y1 >= rect.m_y1) goto paste_x1;

	paste_x1y1:
		//clip.m_x1 + clip.m_y1 inside
		//right part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = clip.m_x1;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	paste_split4:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto paste_xy;

	paste_xyy1:
		//clip.m_x + clip.m_y + clip.m_y1 inside
		clip.m_x = rect.m_x1;
		continue;

	paste_split5:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto paste_y;

	paste_yy1:
		//clip.m_y + clip.m_y1 inside
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	paste_split6:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto paste_x;

	paste_xy1:
		//clip.m_x + clip.m_y1 inside
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = clip.m_y1;
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	paste_split7:
		//jump to correct splitting code
		if (clip.m_y1 >= rect.m_y1) goto paste_encl;

	paste_y1:
		//clip.m_y1 inside
		//bottom part
		rect.m_y = clip.m_y1;
		continue;

	paste_xyx1:
		//clip.m_x + clip.m_y + clip.m_x1 inside
		clip.m_y = rect.m_y1;
		continue;

	paste_encl:
		//region is enclosed
		m_region.erase_after(last_itr);
		itr = last_itr;
		continue;

	paste_x:
		//clip.m_x inside
		//left part
		rect.m_x1 = clip.m_x;
		continue;

	paste_y:
		//clip.m_y inside
		//top part
		rect.m_y1 = clip.m_y;
		continue;

	paste_xy:
		//clip.m_x + clip.m_y inside
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//left part
		rect.m_y = clip.m_y;
		rect.m_x1 = clip.m_x;
		continue;

	paste_x1:
		//clip.m_x1 inside
		//right part
		rect.m_x = clip.m_x1;
		continue;

	paste_xx1:
		//clip.m_x + clip.m_x1 inside
		//left part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = clip.m_x;
		new_rect->m_y1 = rect.m_y1;
		//right part
		rect.m_x = clip.m_x1;
		continue;

	paste_yx1:
		//clip.m_y + clip.m_x1 inside
		//top part
		m_region.emplace_front(Rect(0,0,0,0));
		new_rect = &m_region.front();
		new_rect->m_x = rect.m_x;
		new_rect->m_y = rect.m_y;
		new_rect->m_x1 = rect.m_x1;
		new_rect->m_y1 = clip.m_y;
		//right part
		rect.m_x = clip.m_x1;
		rect.m_y = clip.m_y;
	}

	//create new rect ?
	if (clip.m_x != clip.m_x1 && clip.m_y != clip.m_y1) m_region.push_front(clip);
	return this;
}

Region *Region::remove_region(Region &dest, int32_t rx, int32_t ry)
{
	//remove all rects in region from dest
	for (auto rect : m_region)
	{
		rect.m_x += rx;
		rect.m_y += ry;
		rect.m_x1 += rx;
		rect.m_y1 += ry;
		dest.remove_rect(rect);
	}
	return this;
}

Region *Region::paste_region(Region &dest, int32_t rx, int32_t ry)
{
	//paste all rects in region to dest
	for (auto rect : m_region)
	{
		rect.m_x += rx;
		rect.m_y += ry;
		rect.m_x1 += rx;
		rect.m_y1 += ry;
		dest.paste_rect(rect);
	}
	return this;
}

Region *Region::copy_region(Region &dest, const Region &copy, int32_t rx, int32_t ry)
{
	//copy all rects of region to dest
	for (auto rect : copy.m_region)
	{
		rect.m_x += rx;
		rect.m_y += ry;
		rect.m_x1 += rx;
		rect.m_y1 += ry;
		copy_rect(dest, rect);
	}
	return this;
}
