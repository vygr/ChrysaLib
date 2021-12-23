#ifndef VIEW_H
#define VIEW_H

#include "property.h"
#include <list>
#include <mutex>
#include <map>

class Ctx;

//view class
//base class for all widgets
class View
{
public:
	View(int x, int y, int w, int h)
		: m_x(x)
		, m_y(y)
		, m_w(w)
		, m_h(h)
	{}
	View *add_front(std::shared_ptr<View> child);
	View *add_back(std::shared_ptr<View> child);
	View *sub();
	View *def_prop(const std::string &prop, std::shared_ptr<Property>);
	std::shared_ptr<Property> get_prop(const std::string &prop);
	virtual View *draw(Ctx *ctx) = 0;
	static std::mutex m_mutex;
	View *m_parent = nullptr;
	std::list<std::shared_ptr<View>> m_children;
	std::map<std::string, std::shared_ptr<Property>> m_properties;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
};

#endif
