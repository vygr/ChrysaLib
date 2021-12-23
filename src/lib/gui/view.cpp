#include "view.h"
#include "property.h"
#include <SDL.h>
#include <algorithm>

std::mutex View::m_mutex;

View *View::add_front(std::shared_ptr<View> child)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!child->m_parent)
	{
		child->m_parent = this;
		m_children.push_back(child);
	}
	return this;
}

View *View::add_back(std::shared_ptr<View> child)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!child->m_parent)
	{
		child->m_parent = this;
		m_children.push_front(child);
	}
	return this;
}

View *View::sub()
{
	std::lock_guard<std::mutex> lock(m_mutex);
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

std::shared_ptr<Property> View::get_prop(const std::string &prop)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	auto view = this;
	while (view != nullptr)
	{
		auto itr = view->m_properties.find(prop);
		if (itr != end(view->m_properties)) return itr->second;
		view = view->m_parent;
	}
	return nullptr;
}

View *View::def_prop(const std::string &prop, std::shared_ptr<Property> value)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_properties[prop] = value;
	return this;
}

View *View::draw(Ctx *ctx)
{
	return this;
}
