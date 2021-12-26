#ifndef TITLE_H
#define TITLE_H

#include "label.h"

class Title : public Label
{
public:
    Title();
	Title *mouse_down(const std::shared_ptr<Msg> &event) override;
	Title *mouse_move(const std::shared_ptr<Msg> &event) override;
	int m_drag_x = 0;
	int m_drag_y = 0;
};

#endif
