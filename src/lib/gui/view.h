#ifndef VIEW_H
#define VIEW_H

#include "property.h"
#include "region.h"
#include <list>
#include <mutex>
#include <map>
#include <functional>
#include <memory>
#include <vector>

class Ctx;

//view flags
enum
{
	view_flag_solid = 1 << 0,
	view_flag_opaque = 1 << 1,
	view_flag_dirty_all = 1 << 2,
	view_flag_hidden = 1 << 3,
	view_flag_at_back = 1 << 4,
	view_flag_at_front = 1 << 5,
};

struct view_pos
{
	int m_x;
	int m_y;
};

struct view_size
{
	//can be compared !
	bool operator==(const view_size &p) const
	{
		return std::tie(p.m_w, p.m_h) == std::tie(m_w, m_h);
	}
	int m_w;
	int m_h;
};

//view class
//base class for all widgets
class View
{
public:
	View() {}
	//properties
	View *def_prop(const std::string &prop, std::shared_ptr<Property>);
	View *set_prop(const std::string &prop, std::shared_ptr<Property>);
	std::shared_ptr<Property> got_prop(const std::string &prop);
	std::shared_ptr<Property> get_prop(const std::string &prop);
	int64_t got_long_prop(const std::string &prop);
	const std::string got_string_prop(const std::string &prop);
	int64_t get_long_prop(const std::string &prop);
	const std::string get_string_prop(const std::string &prop);
	//children
	std::vector<std::shared_ptr<View>> children();
	View *add_front(std::shared_ptr<View> child);
	View *add_back(std::shared_ptr<View> child);
	View *sub();
	//tree iteration
	View *forward_tree(void *user, std::function<bool(View*, void*)>down, std::function<bool(View*, void*)>up);
	View *backward_tree(void *user, std::function<bool(View*, void*)>down, std::function<bool(View*, void*)>up);
	//visability
	View *add_opaque(const Rect &rect);
	View *sub_opaque(const Rect &rect);
	View *clr_opaque();
	//dirty areas
	View *add_dirty(const Rect &rect);
	View *trans_dirty(int rx, int ry);
	View *dirty();
	View *dirty_all();
	//flags
	View *set_flags(unsigned int flags, unsigned int mask);
	//info
    view_pos get_pos();
    view_size get_size();
	//subclass overides
    virtual view_size get_pref_size();
	virtual View *layout();
	virtual View *change(int x, int y, int w, int h);
	virtual View *add_child(std::shared_ptr<View> child) { return add_back(child); }
	virtual View *draw(Ctx *ctx);

	static std::recursive_mutex m_mutex;
	View *m_parent = nullptr;
	std::list<std::shared_ptr<View>> m_children;
	std::map<std::string, std::shared_ptr<Property>> m_properties;
	Region m_dirty;
	Region m_opaque;
	int m_x = 0;
	int m_y = 0;
	int m_w = 0;
	int m_h = 0;
	int m_ctx_x = 0;
	int m_ctx_y = 0;
	unsigned int m_flags = view_flag_solid; 
};

#endif
