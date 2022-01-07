#ifndef UI_H
#define UI_H

#include "colors.h"
#include "backdrop.h"
#include "window.h"
#include "flow.h"
#include "grid.h"
#include "title.h"
#include "button.h"
#include "scroll.h"
#include "canvas.h"
#include "path.h"

#define ui_end ui_root.pop_back();

#define ui_window(_name_, _props_) \
	auto ui_root = std::vector<std::shared_ptr<View>>{}; \
	auto _name_ = std::make_shared<Window>(); \
	ui_root.push_back(_name_); \
	ui_root.back()->def_props({ \
		{"color", Property::get_default("env_window_col")->get_long()}, \
		{"ink_color", Property::get_default("env_ink_col")->get_long()}, \
		{"hint_color", Property::get_default("env_hint_col")->get_long()}, \
		{"no_hint_color", Property::get_default("env_no_hint_col")->get_long()}, \
		{"border", Property::get_default("env_window_border")->get_long()}, \
		{"shadow", Property::get_default("env_window_shadow")->get_long()}, \
		{"font", Property::get_default("env_window_font")->get_font()}, \
		}); \
	ui_root.back()->def_props _props_;

#define ui_flow(_name_, _props_) \
	auto _name_ = std::make_shared<Flow>(); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props _props_;

#define ui_grid(_name_, _props_) \
	auto _name_ = std::make_shared<Grid>(); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props _props_;

#define ui_button(_name_, _props_) \
	auto _name_ = std::make_shared<Button>(); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props({ \
		{"flow_flags", flow_flag_down | flow_flag_align_hcenter | flow_flag_align_vcenter}, \
		{"border", Property::get_default("env_button_border")->get_long()}, \
		}); \
	ui_root.back()->def_props _props_;

#define ui_label(_name_, _props_) \
	auto _name_ = std::make_shared<Label>(); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props({ \
		{"flow_flags", flow_flag_right | flow_flag_align_vcenter}, \
		{"border", Property::get_default("env_label_border")->get_long()}, \
		}); \
	ui_root.back()->def_props _props_;

#define ui_title(_name_, _props_) \
	auto _name_ = std::make_shared<Title>(); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props({ \
		{"border", Property::get_default("env_title_border")->get_long()}, \
		{"font", Property::get_default("env_title_font")->get_font()}, \
		}); \
	ui_root.back()->def_props _props_;

#define ui_scroll(_name_, _flags_, _props_) \
	auto _name_ = std::make_shared<Scroll>(_flags_); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props({ \
		{"color", Property::get_default("env_slider_col")->get_long()}, \
		}); \
	ui_root.back()->def_props _props_;

#define ui_canvas(_name_, _width_, _height_, _scale_, _props_) \
	auto _name_ = std::make_shared<Canvas>(_width_, _height_, _scale_); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props({ \
		{"color", 0}, \
		}); \
	ui_root.back()->def_props _props_;

#endif
