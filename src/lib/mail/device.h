#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <array>
#include <random>

//device ID, any true random bits will do.
//the Dev_ID provides each device with a unique identity.
//within reason a 128bit crypto random value will ensure that
//we can allways up the length of the array if needed later.
struct Dev_ID
{
	//can be compared !
	bool operator==(const Dev_ID &o) const { return o.m_id == m_id; }
	bool operator!=(const Dev_ID &o) const { return o.m_id != m_id; }
	//can be a key in std::set and std::map !
	bool operator<(const Dev_ID &o) const { return o.m_id < m_id; }
	//printable conversions
	auto to_string() const
	{
		auto s = std::string{};
		s.reserve(m_id.size() * 2);
		for (auto &c : m_id) s.push_back((c >> 4) + 'a'), s.push_back((c & 0xf) + 'a');
		return s;
	}
	static auto from_string(const std::string &s)
	{
		Dev_ID id;
		auto itr = begin(s);
		for (auto &c : id.m_id) c = ((*itr++ - 'a') << 4) + (*itr++ - 'a');
		return id;
	}
	//create from a strong random pool
	static auto alloc()
	{
		Dev_ID id;
		std::random_device rd;
		std::uniform_int_distribution<int> dist(0, 256);
		for (auto &c : id.m_id) c = dist(rd);
		return id;
	}
	std::array<uint8_t, 16> m_id;
};

#endif
