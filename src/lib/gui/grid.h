#ifndef GRID_H
#define GRID_H

#include "view.h"

class Grid : public View
{
public:
    Grid()
        : View()
	{}
	view_size pref_size() override;
	Grid *layout() override;
};

#endif
