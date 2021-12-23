#ifndef VIEW_H
#define VIEW_H

#include "property.h"
#include "region.h"
#include <list>
#include <mutex>
#include <map>
#include <functional>

class Ctx;

//view class
//base class for all widgets
class View : public std::enable_shared_from_this<View>
{
public:
	View(int x, int y, int w, int h)
		: m_x(x)
		, m_y(y)
		, m_w(w)
		, m_h(h)
	{}
	View *def_prop(const std::string &prop, std::shared_ptr<Property>);
	std::shared_ptr<Property> get_prop(const std::string &prop);
	View *add_front(std::shared_ptr<View> child);
	View *add_back(std::shared_ptr<View> child);
	View *sub();
	View *add_opaque(const Rect &rect);
	View *sub_opaque(const Rect &rect);
	View *add_dirty(const Rect &rect);
	View *sub_dirty(const Rect &rect);
	View *dirty();
	View *forward_tree(void *user, std::function<bool(View*, void*)>down, std::function<bool(View*, void*)>up);
	View *backward_tree(void *user, std::function<bool(View*, void*)>down, std::function<bool(View*, void*)>up);
	virtual View *draw(Ctx *ctx) = 0;

	static std::mutex m_mutex;
	View *m_parent = nullptr;
	std::list<std::shared_ptr<View>> m_children;
	std::map<std::string, std::shared_ptr<Property>> m_properties;
	Region m_dirty;
	Region m_opaque;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
	int m_ctx_x;
	int m_ctx_y;
};

#endif
