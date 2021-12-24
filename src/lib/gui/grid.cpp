#include "grid.h"
#include <algorithm>

view_size Grid::get_pref_size()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto kids = children();
	auto grid_w = (int)get_long_prop("grid_width");
	auto grid_h = (int)get_long_prop("grid_height");
	auto num_child = (int)(kids.size());
	if (!grid_w) grid_w = ((num_child - 1) / grid_h) + 1;
	if (!grid_h) grid_h = ((num_child - 1) / grid_w) + 1;

	auto max_w = 0;
	auto max_h = 0;
	for (auto &child : kids)
	{
		auto s = child->get_pref_size();
		max_w = std::max(max_w, s.m_w);
		max_h = std::max(max_h, s.m_h);
	}
	return view_size{max_w * grid_w, max_h * grid_h};
}

Grid *Grid::layout()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto kids = children();
	auto grid_w = get_long_prop("grid_width");
	auto grid_h = get_long_prop("grid_height");
	auto num_child = static_cast<unsigned int>(kids.size());
	if (!grid_w) grid_w = ((num_child - 1) / grid_h) + 1;
	if (!grid_h) grid_h = ((num_child - 1) / grid_w) + 1;

	auto child_num = 0;
	auto w = (m_w << 16);
	auto h = (m_h << 16);
	for (auto &child : kids)
	{
		auto row = child_num / grid_w;
		auto col = child_num % grid_w;
		auto cell_x = (col * w / grid_w) >> 16;
		auto cell_y = (row * h / grid_h) >> 16;
		auto cell_x1 = ((col + 1) * w / grid_w) >> 16;
		auto cell_y1 = ((row + 1) * h / grid_h) >> 16;
		child->change(cell_x, cell_y, cell_x1 - cell_x, cell_y1 - cell_y);
		child_num++;
	}
	return this;
}
