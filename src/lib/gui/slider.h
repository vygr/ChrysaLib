#ifndef SLIDER_H
#define SLIDER_H

#include "view.h"

class Slider : public View
{
public:
	Slider()
		: View()
	{}
	view_size pref_size() override;
	Slider *draw(const Ctx &ctx) override;
	Slider *mouse_down(const std::shared_ptr<Msg> &event) override;
	Slider *mouse_up(const std::shared_ptr<Msg> &event) override;
	Slider *mouse_move(const std::shared_ptr<Msg> &event) override;
	int m_state = 0;
	int m_old_value = 0;
	int m_down_xy = 0;
};

#endif
