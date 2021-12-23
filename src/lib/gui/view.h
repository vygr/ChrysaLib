#ifndef VIEW_H
#define VIEW_H

#include <list>
#include <mutex>

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
	virtual View *draw(Ctx *ctx) = 0;
	static std::mutex m_mutex;
	View *m_parent = nullptr;
	std::list<std::shared_ptr<View>> m_children;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
};

#endif
