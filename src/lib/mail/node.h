#ifndef NODE_H
#define NODE_H

#include "../../host/pii.h"
#include <string>
#include <array>
#include <random>

extern void pii_random(char* addr, size_t len);

//node ID, any true random bits will do.
//the Node_ID provides each device with a unique identity.
//within reason a 128bit crypto random value will ensure that
//we can always up the length of the array if needed later.
struct Node_ID
{
	//can be compared !
	bool operator==(const Node_ID &o) const { return o.m_id == m_id; }
	bool operator!=(const Node_ID &o) const { return o.m_id != m_id; }
	//can be a key in std::set and std::map !
	bool operator<(const Node_ID &o) const { return o.m_id < m_id; }
	//printable conversions
	auto to_string() const
	{
		auto s = std::string{};
		s.reserve(m_id.size() * 2);
		for (auto &c : m_id) s.push_back((c & 0xf) + 'a'), s.push_back((c >> 4) + 'a');
		return s;
	}
	static auto from_string(const std::string &s)
	{
		Node_ID id;
		auto itr = begin(s);
		for (auto &c : id.m_id) c = (*itr++ - 'a') + ((*itr++ - 'a') << 4);
		return id;
	}
	//create from a strong random pool
	static auto alloc()
	{
		Node_ID id;
		pii_random((char*)&id, sizeof(id));
		return id;
	}
	union
	{
		std::array<uint8_t, 16> m_id = {0};
		node_id m_node_id;
	};
};

#endif
