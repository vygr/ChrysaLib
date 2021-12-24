#ifndef TEXT_H
#define TEXT_H

#include "view.h"

class Text : public View
{
public:
    Text()
        : View()
	{}
	view_size get_pref_size() override;
	Text *draw(Ctx *ctx) override;
};

#endif
