#include "view.h"
#include "property.h"
#include <algorithm>
#include <iterator>

std::recursive_mutex View::m_mutex;
int64_t View::m_next_id = 0;

View::View()
	: m_id(--m_next_id)
{}

std::vector<std::shared_ptr<View>> View::children()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto children = std::vector<std::shared_ptr<View>>{};
	std::copy(begin(m_children), end(m_children), std::back_inserter(children));
	return children;
}

View *View::add_front(std::shared_ptr<View> child)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (child->m_parent) child->sub();
	child->m_parent = this;
	m_children.push_front(child);
	return this;
}

View *View::add_back(std::shared_ptr<View> child)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (child->m_parent) child->sub();
	child->m_parent = this;
	m_children.push_back(child);
	return this;
}

View *View::sub()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (m_parent)
	{
		auto itr = std::find_if(begin(m_parent->m_children), end(m_parent->m_children), [&] (auto &c)
		{
			return this == c.get();
		});
		if (itr != end(m_parent->m_children))
		{
			m_parent->m_children.erase(itr);
			m_parent = nullptr;
		}
	}
	return this;
}

View *View::to_back()
{
	return this;
}

View *View::to_front()
{
	return this;
}

View *View::def_prop(const std::string &prop, std::shared_ptr<Property> value)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_properties[prop] = value;
	return this;
}

View *View::set_prop(const std::string &prop, std::shared_ptr<Property> value)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto view = this;
	while (view != nullptr)
	{
		auto itr = view->m_properties.find(prop);
		if (itr != end(view->m_properties))
		{
			itr->second = value;
			break;
		}
		view = view->m_parent;
	}
	return this;
}

std::shared_ptr<Property> View::got_prop(const std::string &prop)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto itr = m_properties.find(prop);
	if (itr != end(m_properties)) return itr->second;
	return nullptr;
}

std::shared_ptr<Property> View::get_prop(const std::string &prop)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto view = this;
	while (view != nullptr)
	{
		auto itr = view->m_properties.find(prop);
		if (itr != end(view->m_properties)) return itr->second;
		view = view->m_parent;
	}
	return nullptr;
}

int64_t View::got_long_prop(const std::string &prop)
{
	auto p = got_prop(prop);
	if (p) return p->get_long();
	return 0;
};

const std::string View::got_string_prop(const std::string &prop)
{
	auto p = got_prop(prop);
	if (p) return p->get_string();
	return "";
};

int64_t View::get_long_prop(const std::string &prop)
{
	auto p = get_prop(prop);
	if (p) return p->get_long();
	return 0;
};

const std::string View::get_string_prop(const std::string &prop)
{
	auto p = get_prop(prop);
	if (p) return p->get_string();
	return "";
};

View *View::add_opaque(const Rect &rect)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_opaque.paste_rect(rect);
	return this;
}

View *View::sub_opaque(const Rect &rect)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_opaque.remove_rect(rect);
	return this;
}

View *View::clr_opaque()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_opaque.free();
	return this;
}

View *View::add_dirty(const Rect &rect)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_dirty.paste_rect(rect);
	return this;
}

View *View::trans_dirty(int rx, int ry)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	m_dirty.translate(rx, ry);
	return this;
}

View *View::dirty()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	add_dirty(Rect(0, 0, m_w, m_h));
	return this;
}

View *View::dirty_all()
{
	return set_flags(view_flag_dirty_all, view_flag_dirty_all);
}

View *View::forward_tree(std::function<bool(View &view)> down, std::function<bool(View &view)> up)
{
	//child function
	std::function<View &(View &view)> forward_tree = [&](View &view) -> View&
	{
		if (down(view))
		{
			std::for_each(begin(view.m_children), end(view.m_children), [&] (auto &child)
			{
				forward_tree(*child);
			});
		}
		up(view);
		return view;
	};
	//root locking function
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (down(*this))
	{
		std::for_each(begin(m_children), end(m_children), [&] (auto &child)
		{
			forward_tree(*child);
		});
	}
	up(*this);
	return this;
}

View *View::backward_tree(std::function<bool(View &view)> down, std::function<bool(View &view)> up)
{
	//child function
	std::function<View &(View &view)> backward_tree = [&](View &view) -> View&
	{
		if (down(view))
		{
			std::for_each(rbegin(view.m_children), rend(view.m_children), [&] (auto &child)
			{
				backward_tree(*child);
			});
		}
		up(view);
		return view;
	};
	//root locking function
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	if (down(*this))
	{
		std::for_each(rbegin(m_children), rend(m_children), [&] (auto &child)
		{
			backward_tree(*child);
		});
	}
	up(*this);
	return this;
}

View *View::set_flags(unsigned int flags, unsigned int mask)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto dirty_state = m_flags & view_flag_dirty_all;
	m_flags = (m_flags & ~mask) | flags | dirty_state;
	return this;
}

View *View::draw(const Ctx &ctx)
{
	return this;
}

view_pos View::get_pos()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return view_pos{m_x, m_y};
}

view_size View::get_size()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return view_size{m_w, m_h};
}

view_bounds View::get_bounds()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return view_bounds{m_x, m_y, m_w, m_h};
}

view_size View::pref_size()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	return view_size{(int)get_long_prop("min_width"), (int)get_long_prop("min_height")};
}

View *View::layout()
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto col = (uint32_t)get_long_prop("color");
	if ((col >> 24) == 0xff) set_flags(view_flag_opaque, view_flag_opaque);
	return this;
}

View *View::change(int x, int y, int w, int h)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto s = get_size();
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	if (s == view_size{w, h}) return this;
	return layout();
}

View *View::change_dirty(int x, int y, int w, int h)
{
	std::lock_guard<std::recursive_mutex> lock(m_mutex);
	auto old_p = get_pos();
	return dirty()->change(x, y, w, h)->trans_dirty(old_p.m_x - x, old_p.m_y - y);
}
