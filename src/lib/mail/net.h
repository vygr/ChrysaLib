#ifndef NET_H
#define NET_H

#include "device.h"
#include "mailbox.h"

std::vector<std::string> split_string(std::string str, const std::string &token);

//network ID, combination of a Dev_ID and local Mailbox_ID.
//this is a network wide address for a mailbox. what might be present at that mailbox
//is not defined.
struct Net_ID
{
	Net_ID() {}
	Net_ID(Dev_ID dev, Mailbox_ID mbox)
		: m_device_id(dev)
		, m_mailbox_id(mbox)
	{}
	Net_ID(Dev_ID dev)
		: m_device_id(dev)
	{}
	Net_ID(Mailbox_ID mbox)
		: m_mailbox_id(mbox)
	{}
	//can be compared !
	bool operator==(const Net_ID &p) const
	{
		return std::tie(p.m_mailbox_id, p.m_device_id) == std::tie(m_mailbox_id, m_device_id);
	}
	//can be a key in std::set and std::map !
	bool operator<(const Net_ID &p) const
	{
		return std::tie(p.m_mailbox_id, p.m_device_id) < std::tie(m_mailbox_id, m_device_id);
	}
	//printable conversions
	static auto from_string(const std::string &s)
	{
		Net_ID id;
		auto pids = split_string(s, ":");
		id.m_device_id = Dev_ID::from_string(pids[0]);
		id.m_mailbox_id = Mailbox_ID::from_string(pids[1]);
		return id;
	}
	auto to_string() const
	{
		return m_device_id.to_string() + ":" + m_mailbox_id.to_string();
	}
	Dev_ID m_device_id;
	Mailbox_ID m_mailbox_id;
};

#endif
