#ifndef TITLE_H
#define TITLE_H

#include "label.h"

class Title : public Label
{
public:
    Title();
	Title *mouse_down(const std::shared_ptr<Msg> &event) override;
	Title *mouse_move(const std::shared_ptr<Msg> &event) override;
	int32_t m_drag_x = 0;
	int32_t m_drag_y = 0;
};

#endif
