#ifndef VIEW_H
#define VIEW_H

#include "../services/task.h"
#include "../mail/msg.h"
#include "property.h"
#include "region.h"
#include "ctx.h"
#include "font.h"
#include <list>
#include <mutex>
#include <map>
#include <functional>
#include <memory>
#include <vector>

//view event msg types
enum
{
	ev_type_mouse,
	ev_type_key,
	ev_type_action,
	ev_type_gui,
	ev_type_wheel,
	ev_type_enter,
	ev_type_exit
};

//key mods
enum
{
	ev_key_mod_left_shift = 0x1,
	ev_key_mod_right_shift = 0x2,
	ev_key_mod_left_option = 0x100,
	ev_key_mod_right_option = 0x200,
	ev_key_mod_left_command = 0x400,
	ev_key_mod_right_command = 0x800,
	ev_key_mod_caps_lock = 0x2000,
	ev_key_mod_control = 0x40,

	ev_key_mod_shift = ev_key_mod_left_shift + ev_key_mod_right_shift,
	ev_key_mod_option = ev_key_mod_left_option + ev_key_mod_right_option,
	ev_key_mod_command = ev_key_mod_left_command +ev_key_mod_right_command,
};

//view flags
enum
{
	view_flag_solid = 1 << 0,
	view_flag_opaque = 1 << 1,
	view_flag_dirty_all = 1 << 2,
	view_flag_hidden = 1 << 3,
	view_flag_at_back = 1 << 4,
	view_flag_at_front = 1 << 5,
	view_flag_screen = 1 << 6,
};

struct view_pos
{
	int32_t m_x = 0;
	int32_t m_y = 0;
};

struct view_size
{
	//can be compared !
	bool operator==(const view_size &p) const
	{
		return std::tie(p.m_w, p.m_h) == std::tie(m_w, m_h);
	}
	int32_t m_w = 0;
	int32_t m_h = 0;
};

struct view_bounds
{
	int32_t m_x = 0;
	int32_t m_y = 0;
	int32_t m_w = 0;
	int32_t m_h = 0;
};

class Router;

//view class
//base class for all widgets
class View
{
public:
	struct Event : public Task::Event
	{
		uint64_t m_type;
	};
	struct Event_mouse : public Event
	{
		uint32_t m_buttons;
		uint32_t m_count;
		int32_t m_x;
		int32_t m_y;
		int32_t m_rx;
		int32_t m_ry;
	};
	struct Event_wheel : public Event
	{
		uint32_t m_direction;
		int32_t m_x;
		int32_t m_y;
	};
	struct Event_key : public Event
	{
		uint32_t m_keycode;
		uint32_t m_key;
		uint32_t m_mod;
	};
	struct Event_action : public Event
	{
		uint64_t m_source_id;
	};
	struct Event_gui : public Event
	{};
	struct Event_enter : public Event
	{};
	struct Event_exit : public Event
	{};
	View();
	//properties
	View *def_props(const std::map<const char*, Property> &props);
	View *set_props(const std::map<const char*, Property> &props);
	std::shared_ptr<Property> got_prop(const std::string &prop);
	std::shared_ptr<Property> get_prop(const std::string &prop);
	int64_t got_long_prop(const std::string &prop);
	const std::string got_string_prop(const std::string &prop);
	const std::shared_ptr<Font> got_font_prop(const std::string &prop);
	int64_t get_long_prop(const std::string &prop);
	const std::string get_string_prop(const std::string &prop);
	const std::shared_ptr<Font> get_font_prop(const std::string &prop);
	//children
	std::vector<std::shared_ptr<View>> children();
	View *add_front(std::shared_ptr<View> child);
	View *add_back(std::shared_ptr<View> child);
	View *sub();
	View *to_back();
	View *to_front();
	View *hide();
	//tree iteration
	View *forward_tree(std::function<bool(View&)>down, std::function<bool(View&)>up);
	View *backward_tree(std::function<bool(View&)>down, std::function<bool(View&)>up);
	//visability
	View *add_opaque(int32_t x, int32_t y, int32_t w, int32_t h);
	View *sub_opaque(int32_t x, int32_t y, int32_t w, int32_t h);
	View *clr_opaque();
	//dirty areas
	View *add_dirty(int32_t x, int32_t y, int32_t w, int32_t h);
	View *trans_dirty(int32_t rx, int32_t ry);
	View *dirty();
	View *dirty_all();
	View *change_dirty(int32_t x, int32_t y, int32_t w, int32_t h);
	View *change_dirty(const view_pos &pos, const view_size &size)
		{ return change_dirty(pos.m_x, pos.m_y, size.m_w, size.m_h); }
	//flags
	View *set_flags(uint32_t flags, uint32_t mask);
	//info
	int64_t get_id() { return m_id; }
	view_pos get_pos() const;
	view_size get_size() const;
	view_bounds get_bounds() const;
	View *set_bounds(int32_t x, int32_t y, int32_t w, int32_t h);
	Net_ID find_owner() const;
	View *find_id(int64_t id);
	bool hit(int32_t x, int32_t y) const;
	View *hit_tree(int32_t x, int32_t y);
	View *change(int32_t x, int32_t y, int32_t w, int32_t h);
	View *change(const view_size &size)
		{ return change(0, 0, size.m_w, size.m_h); }
	View *change(const view_bounds &bounds)
		{ return change(bounds.m_x, bounds.m_y, bounds.m_w, bounds.m_h); }
	//actions
	View *connect(uint64_t id) { m_actions.push_back(id); return this; }
	View *emit();
	//subclass overides
	virtual view_size pref_size();
	virtual View *layout();
	virtual View *add_child(std::shared_ptr<View> child) { return add_back(child); }
	virtual View *draw(const Ctx &ctx);
	virtual View *action(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *mouse_down(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *mouse_up(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *mouse_move(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *mouse_hover(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *mouse_wheel(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *mouse_enter(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *mouse_exit(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *key_down(const std::shared_ptr<Msg> &event) { return this; }
	virtual View *key_up(const std::shared_ptr<Msg> &event) { return this; }

	static std::recursive_mutex m_mutex;
	static int64_t m_next_id;
	static uint32_t m_gui_flags;
	static std::vector<std::shared_ptr<View>> m_temps;
	View *m_parent = nullptr;
	std::list<std::shared_ptr<View>> m_children;
	std::map<std::string, std::shared_ptr<Property>> m_properties;
	Region m_dirty;
	Region m_opaque;
	int32_t m_x = 0;
	int32_t m_y = 0;
	int32_t m_w = 0;
	int32_t m_h = 0;
	uint32_t m_flags = view_flag_solid;
	int64_t m_id = 0;
	std::vector<int64_t> m_actions;
	Ctx m_ctx;
	Net_ID m_owner;
};

#endif
