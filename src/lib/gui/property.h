#ifndef PROP_H
#define PROP_H

#include "font.h"

//property
class Property
{
public:
	Property(int64_t num)
		: m_long(num)
	{}
	Property(const std::string &str)
		: m_string(str)
	{}
	Property(const char *str)
		: m_string(std::string(str))
	{}
	Property(const std::shared_ptr<Font> &font)
		: m_font(font)
	{}
	int64_t get_long() { return m_long; }
	const std::string &get_string() { return m_string; }
	const std::shared_ptr<Font> get_font() { return m_font; }
	const int64_t m_long = 0;
	const std::string m_string;
	const std::shared_ptr<Font> m_font;
};

#endif
