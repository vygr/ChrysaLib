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
	ui_root.back()->def_props _props_;

#define ui_label(_name_, _props_) \
	auto _name_ = std::make_shared<Label>(); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props _props_;

#define ui_title(_name_, _props_) \
	auto _name_ = std::make_shared<Title>(); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props _props_;

#define ui_scroll(_name_, _flags_, _props_) \
	auto _name_ = std::make_shared<Scroll>(_flags_); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props _props_;

#define ui_canvas(_name_, _width_, _height_, _scale_, _props_) \
	auto _name_ = std::make_shared<Canvas>(_width_, _height_, _scale_); \
	ui_root.push_back(_name_); \
	ui_root[ui_root.size() - 2]->add_child(ui_root.back()); \
	ui_root.back()->def_props _props_;

#endif
