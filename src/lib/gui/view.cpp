#include "view.h"
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

View *View::draw(Ctx *ctx)
{
	return this;
}
