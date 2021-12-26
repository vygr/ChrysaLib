#ifndef LABEL_H
#define LABEL_H

#include "flow.h"
#include "text.h"

class Label
	: public View
{
public:
    Label();
	view_size get_pref_size() override;
	Label *layout() override;
	Label *add_child(std::shared_ptr<View> child) override;
	Label *draw(const Ctx &ctx) override;
	std::shared_ptr<Flow> m_flow;
	std::shared_ptr<Text> m_text;
};

#endif
