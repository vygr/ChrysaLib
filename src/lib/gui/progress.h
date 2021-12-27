#ifndef PROGRESS_H
#define PROGRESS_H

#include "view.h"

class Progress : public View
{
public:
	Progress()
		: View()
	{}
	view_size pref_size() override;
	Progress *draw(const Ctx &ctx) override;
};

#endif
