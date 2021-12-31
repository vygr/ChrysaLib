#ifndef BUTTON_H
#define BUTTON_H

#include "label.h"

class Button : public Label
{
public:
    Button();
	Button *layout() override;
	Button *draw(const Ctx &ctx) override;
	Button *mouse_down(const std::shared_ptr<Msg> &event) override;
	Button *mouse_up(const std::shared_ptr<Msg> &event) override;
	Button *mouse_move(const std::shared_ptr<Msg> &event) override;
	int32_t m_state = 1;
};

#endif
