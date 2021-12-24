#ifndef PROP_H
#define PROP_H

#include <string>

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
	int64_t get_long() { return m_long; };
	const std::string &get_string() { return m_string; };
	Property *set_long(int val) { m_long = val; return this; };
	Property *set_string(const std::string &val) { m_string = val; return this; };
	int64_t m_long;
	std::string m_string;
};

#endif
