#include "grid.h"
#include "../math/fixed.h"
#include <algorithm>

view_size Grid::pref_size()
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto kids = children();
	auto grid_w = (int32_t)get_long_prop("grid_width");
	auto grid_h = (int32_t)get_long_prop("grid_height");
	auto num_child = (int32_t)(kids.size());
	if (!grid_w) grid_w = ((num_child - 1) / grid_h) + 1;
	if (!grid_h) grid_h = ((num_child - 1) / grid_w) + 1;

	auto max_w = 0;
	auto max_h = 0;
	for (auto &child : kids)
	{
		auto s = child->pref_size();
		max_w = std::max(max_w, s.m_w);
		max_h = std::max(max_h, s.m_h);
	}
	return view_size{max_w * grid_w, max_h * grid_h};
}

Grid *Grid::layout()
{
	std::lock_guard<std::recursive_mutex> l(m_mutex);
	auto kids = children();
	auto grid_w = (int32_t)get_long_prop("grid_width");
	auto grid_h = (int32_t)get_long_prop("grid_height");
	auto num_child = (int32_t)(kids.size());
	if (!grid_w) grid_w = ((num_child - 1) / grid_h) + 1;
	if (!grid_h) grid_h = ((num_child - 1) / grid_w) + 1;

	auto child_num = 0;
	auto w = fixed32_t(m_w);
	auto h = fixed32_t(m_h);
	for (auto &child : kids)
	{
		auto row = child_num / grid_w;
		auto col = child_num % grid_w;
		auto cell_x = w * col / grid_w;
		auto cell_y = h * row / grid_h;
		auto cell_x1 = w * (col + 1) / grid_w;
		auto cell_y1 = h * (row + 1) / grid_h;
		child->change(cell_x, cell_y, cell_x1 - cell_x, cell_y1 - cell_y);
		child_num++;
	}
	return this;
}
