#ifndef FLOW_H
#define FLOW_H

#include "layout.h"

//flow flags
enum
{
	flow_flag_left = 1 << 0,
	flow_flag_right = 1 << 1,
	flow_flag_up = 1 << 2,
	flow_flag_down = 1 << 3,
	flow_flag_fillw = 1 << 4,
	flow_flag_fillh = 1 << 5,
	flow_flag_lastw = 1 << 6,
	flow_flag_lasth = 1 << 7,
	flow_flag_align_hcenter = 1 << 8,
	flow_flag_align_hleft = 1 << 9,
	flow_flag_align_hright = 1 << 10,
	flow_flag_align_vcenter = 1 << 11,
	flow_flag_align_vtop = 1 << 12,
	flow_flag_align_vbottom = 1 << 13,
};

//useful flow combos
const auto flow_down_fill = flow_flag_down | flow_flag_fillw | flow_flag_lasth;
const auto flow_up_fill = flow_flag_up | flow_flag_fillw | flow_flag_lasth;
const auto flow_right_fill = flow_flag_right | flow_flag_fillh | flow_flag_lastw;
const auto flow_left_fill = flow_flag_left | flow_flag_fillh | flow_flag_lastw;
const auto flow_stack_fill = flow_flag_fillw | flow_flag_fillh | flow_flag_lastw | flow_flag_lasth;

class Flow : public Layout
{
public:
	Flow()
		: Layout()
	{
		def_prop("flow_flags", std::make_shared<Property>(0));
	}
	view_size get_pref_size() override;
	Layout *layout() override;
};

#endif
