#ifndef WINDOW_H
#define WINDOW_H

#include "view.h"

struct drag_mode
{
	int32_t m_mode = 0;
	int32_t m_x = 0;
	int32_t m_y = 0;
};

class Window
	: public View
{
public:
    Window();
	drag_mode get_drag_mode(int32_t x, int32_t y);
	Window *event(const std::shared_ptr<Msg> &event);
	view_size pref_size() override;
	Window *layout() override;
	Window *draw(const Ctx &ctx) override;
	Window *add_child(std::shared_ptr<View> child) override;
	Window *mouse_down(const std::shared_ptr<Msg> &event) override;
	Window *mouse_move(const std::shared_ptr<Msg> &event) override;
	std::shared_ptr<View> m_child;
	drag_mode m_mode;
	uint32_t m_last_buttons = 0;
};

#endif
