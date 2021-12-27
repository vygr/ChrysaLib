#ifndef TEXT_H
#define TEXT_H

#include "view.h"

class Text : public View
{
public:
    Text()
        : View()
	{}
	view_size pref_size() override;
	Text *draw(const Ctx &ctx) override;
};

#endif
