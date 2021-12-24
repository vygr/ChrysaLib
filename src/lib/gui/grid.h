#ifndef GRID_H
#define GRID_H

#include "layout.h"

class Grid : public Layout
{
public:
    Grid()
        : Layout()
	{}
	view_size get_pref_size() override;
	Grid *layout() override;
};

#endif
