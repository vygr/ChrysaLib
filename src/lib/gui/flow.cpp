#include "flow.h"

Layout *Flow::layout()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto fw = m_w;
	auto fh = m_h;
	auto kids = children();
	auto flow_flags = get_long_prop("flow_flags");
	auto last_child = (int)kids.size() - 1;
	auto x = 0;
	auto y = 0;
	if (flow_flags & flow_flag_left) x = fw;
	if (flow_flags & flow_flag_up) y = fh;

	auto child_num = -1;
	for (auto &child : kids)
	{
		child_num++;

		//hidden ?
		if (child->m_flags & view_flag_hiden)
		{
			child->change(0,0,0,0);
			continue;
		};

		//flow
		auto s = child->get_pref_size();
		auto cw = s.m_w;
		auto ch = s.m_h;
		auto cx = x;
		auto cy = y;
		if (flow_flags & flow_flag_left) cx = x - cw, x = cx;
		if (flow_flags & flow_flag_up) cy = y - ch, y = cy;
		if (flow_flags & flow_flag_right) x = x + cw;
		if (flow_flags & flow_flag_down) y = y + ch;

		//filling
		if (flow_flags & flow_flag_fillw) cx = 0, cw = fw;
		if (flow_flags & flow_flag_fillh) cy = 0, ch = fh;

		//alignment
		if (flow_flags & (flow_flag_align_hcenter | flow_flag_align_hleft | flow_flag_align_hright))
		{
			if (flow_flags & flow_flag_align_hleft) cx = 0;
			else if (flow_flags & flow_flag_align_hright) cx = fw - cw;
			else cx = (fw - cw) / 2;
		}
		if (flow_flags & (flow_flag_align_vcenter | flow_flag_align_vtop | flow_flag_align_vbottom))
		{
			if (flow_flags & flow_flag_align_vtop) cy = 0;
			else if (flow_flags & flow_flag_align_vbottom) cy = fh - ch;
			else cy = (fh - ch) / 2;
		}

		//last one ?
		if (child_num == last_child - 1)
		{
			if (flow_flags & flow_flag_lastw)
			{
				if (flow_flags & flow_flag_right) cw = fw - cx;
				if (flow_flags & flow_flag_left) cw = cx + cw, cx = 0;
			}
			if (flow_flags & flow_flag_lasth)
			{
				if (flow_flags & flow_flag_down) ch = fh - cy;
				if (flow_flags & flow_flag_up) ch = cy + ch, cy = 0;
			}
		}
		child->change(cx, cy, cw, ch);
	}
	return this;
}

view_size Flow::get_pref_size()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto kids = children();
	auto flow_flags = get_long_prop("flow_flags");
	auto mw = (int)got_long_prop("min_width");
	auto mh = (int)got_long_prop("min_height");
	auto pw = 0;
	auto ph = 0;
	for (auto &child : kids)
	{
		if (child->m_flags & view_flag_hiden) continue;
		auto s = child->get_pref_size();
		if (flow_flags & (flow_flag_left | flow_flag_right)) pw += s.m_w;
		if (flow_flags & (flow_flag_up | flow_flag_down)) ph += s.m_h;
	}
	return view_size{std::max(mw, pw), std::max(mh, ph)};
}
