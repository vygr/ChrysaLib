#include "property.h"

std::shared_ptr<Property> Property::get_default(const char *prop)
{
	static const std::map<std::string, std::shared_ptr<Property>> defaults =
	{
		{"env_window_col", std::make_shared<Property>(0xffc0c0c0)},
		{"env_title_col", std::make_shared<Property>(0xffe0e0e0)},
		{"env_slider_col", std::make_shared<Property>(0xffe0e0e0)},
		{"env_toolbar_col", std::make_shared<Property>(0xffc0c0c0)},
		{"env_toolbar2_col", std::make_shared<Property>(0xffa0a0a0)},
		{"env_ink_col", std::make_shared<Property>(0xff000000)},
		{"env_backdrop_ink_col", std::make_shared<Property>(0xff101010)},
		{"env_backdrop_col", std::make_shared<Property>(0xff202020)},
		{"env_hint_col", std::make_shared<Property>(0xffb0b0b0)},
		{"env_no_hint_col", std::make_shared<Property>(0xff000000)},
		{"env_radio_col", std::make_shared<Property>(0xffe0e0e0)},
		{"env_window_border", std::make_shared<Property>(1)},
		{"env_window_shadow", std::make_shared<Property>(5)},
		{"env_label_border", std::make_shared<Property>(0)},
		{"env_button_border", std::make_shared<Property>(1)},
		{"env_textfield_border", std::make_shared<Property>(-1)},
		{"env_title_border", std::make_shared<Property>(1)},
		{"env_title_buttons_border", std::make_shared<Property>(1)},
		{"env_window_font", std::make_shared<Property>(Font::open("fonts/OpenSans-Regular.ctf", 18))},
		{"env_title_font", std::make_shared<Property>(Font::open("fonts/OpenSans-Regular.ctf", 20))},
		{"env_body_font", std::make_shared<Property>(Font::open("fonts/OpenSans-Regular.ctf", 14))},
		{"env_title_buttons_font", std::make_shared<Property>(Font::open("fonts/Entypo.ctf", 22))},
		{"env_toolbar_font", std::make_shared<Property>(Font::open("fonts/Entypo.ctf", 28))},
		{"env_medium_toolbar_font", std::make_shared<Property>(Font::open("fonts/Entypo.ctf", 22))},
		{"env_small_toolbar_font", std::make_shared<Property>(Font::open("fonts/Entypo.ctf", 16))},
		{"env_tiny_toolbar_font", std::make_shared<Property>(Font::open("fonts/Entypo.ctf", 10))},
		{"env_terminal_font", std::make_shared<Property>(Font::open("fonts/Hack-Regular.ctf", 16))},
		{"env_medium_terminal_font", std::make_shared<Property>(Font::open("fonts/Hack-Regular.ctf", 14))},
		{"env_small_terminal_font", std::make_shared<Property>(Font::open("fonts/Hack-Regular.ctf", 12))},
		{"env_tip_font", std::make_shared<Property>(Font::open("fonts/OpenSans-Regular.ctf", 14))}
	};
	auto itr = defaults.find(prop);
	if (itr != end(defaults)) return itr->second;
	return nullptr;
}
