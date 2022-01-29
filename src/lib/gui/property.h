#ifndef PROP_H
#define PROP_H

#include "font.h"
#include <variant>

//property
class Property
{
public:
	Property(int64_t num)
		: m_prop(num)
	{}
	Property(const std::string &str)
		: m_prop(str)
	{}
	Property(const char *str)
		: m_prop(std::string(str))
	{}
	Property(const std::shared_ptr<Font> &font)
		: m_prop(font)
	{}
	static std::shared_ptr<Property> get_default(const char *prop);
	int64_t get_long() { return std::get<int64_t>(m_prop); }
	const std::string &get_string() { return std::get<std::string>(m_prop); }
	const std::shared_ptr<Font> get_font() { return std::get<std::shared_ptr<Font>>(m_prop); }
	std::variant<std::string, std::shared_ptr<Font>, int64_t> m_prop;
};

#endif
