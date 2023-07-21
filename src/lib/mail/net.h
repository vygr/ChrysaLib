#ifndef NET_H
#define NET_H

#include "node.h"
#include "mailbox.h"

std::vector<std::string> split_string(std::string str, const std::string &token);

//network ID, combination of a Node_ID and local Mailbox_ID.
//this is a network wide address for a mailbox. what might be present at that mailbox
//is not defined.
struct Net_ID
{
	Net_ID() {}
	Net_ID(const Node_ID &node, Mailbox_ID mbox)
		: m_node_id(node)
		, m_mailbox_id(mbox)
	{}
	Net_ID(const Node_ID &node)
		: m_node_id(node)
	{}
	Net_ID(Mailbox_ID mbox)
		: m_mailbox_id(mbox)
	{}
	//can be compared !
	bool operator==(const Net_ID &p) const
	{
		return std::tie(p.m_mailbox_id, p.m_node_id) == std::tie(m_mailbox_id, m_node_id);
	}
	bool operator!=(const Net_ID &p) const
	{
		return std::tie(p.m_mailbox_id, p.m_node_id) != std::tie(m_mailbox_id, m_node_id);
	}
	//can be a key in std::set and std::map !
	bool operator<(const Net_ID &p) const
	{
		return std::tie(p.m_mailbox_id, p.m_node_id) < std::tie(m_mailbox_id, m_node_id);
	}
	//printable conversions
	static auto from_string(const std::string &s)
	{
		Net_ID id;
		auto pids = split_string(s, ":");
		id.m_node_id = Node_ID::from_string(pids[0]);
		id.m_mailbox_id = Mailbox_ID::from_string(pids[1]);
		return id;
	}
	auto to_string() const
	{
		return m_node_id.to_string() + ":" + m_mailbox_id.to_string();
	}
	Node_ID m_node_id;
	Mailbox_ID m_mailbox_id;
};

#endif
