#ifndef SCROLL_H
#define SCROLL_H

#include "slider.h"

//scroll flags
enum
{
	scroll_flag_vertical = 1 << 0,
	scroll_flag_horizontal = 1 << 1,
};

//scroll flag combos
const auto scroll_flag_both = scroll_flag_vertical | scroll_flag_horizontal;

class Scroll : public View
{
public:
	Scroll(int32_t flags);
	view_size pref_size() override;
	Scroll *add_child(std::shared_ptr<View> child) override;
	Scroll *layout() override;
	Scroll *action(const std::shared_ptr<Msg> &event) override;
	Scroll *mouse_wheel(const std::shared_ptr<Msg> &event) override;
	std::shared_ptr<View> m_child;
	std::shared_ptr<Slider> m_vslider;
	std::shared_ptr<Slider> m_hslider;
};

#endif
