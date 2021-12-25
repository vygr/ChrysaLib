#ifndef WINDOW_H
#define WINDOW_H

#include "view.h"

class Window : public View
{
public:
    Window();
	view_size get_pref_size() override;
	Window *layout() override;
	Window *add_child(std::shared_ptr<View> child) override;
	Window *draw(Ctx *ctx) override;
	std::shared_ptr<View> m_child;
};

#endif
