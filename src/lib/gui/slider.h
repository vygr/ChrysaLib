#ifndef SLIDER_H
#define SLIDER_H

#include "view.h"

class Slider : public View
{
public:
	Slider();
	view_size pref_size() override;
	Slider *draw(const Ctx &ctx) override;
	Slider *mouse_down(const std::shared_ptr<Msg> &event) override;
	Slider *mouse_up(const std::shared_ptr<Msg> &event) override;
	Slider *mouse_move(const std::shared_ptr<Msg> &event) override;
	int32_t m_state = 0;
	int64_t m_old_value = 0;
	int32_t m_down_xy = 0;
};

#endif
