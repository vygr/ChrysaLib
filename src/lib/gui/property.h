#ifndef PROP_H
#define PROP_H

//property
class Property
{
public:
	Property()
	{}
	int get_int() { return m_int; };
	Property *set_int(int val) { m_int = val; return this; };
	unsigned int m_int;
};

#endif
