#ifndef GRID_H
#define GRID_H

#include "layout.h"

class Grid : public Layout
{
public:
    Grid()
        : Layout()
	{
		def_prop("grid_width", std::make_shared<Property>(0));
		def_prop("grid_height", std::make_shared<Property>(0));
	}
	view_size get_pref_size() override;
	Grid *layout() override;
};

#endif
