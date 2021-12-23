#ifndef PROP_H
#define PROP_H

#include <string>

//property
class Property
{
public:
	Property()
	{}
	int get_int() { return m_int; };
	Property *set_int(int val) { m_int = val; return this; };
	std::string &get_str() { return m_str; };
	Property *set_str(const std::string &val) { m_str = val; return this; };
	unsigned int m_int;
	std::string m_str;
};

#endif
