#include "link.h"
#include "../mail/router.h"

extern std::unique_ptr<Router> global_router;

////////
// links
////////

void Link::run_send()
{
	//link driver send loop
	std::shared_ptr<Msg> out_msg;
	while (m_running)
	{
		//do we have outgoing messages ?
		out_msg = global_router->get_next_msg(m_remote_node_id, std::chrono::milliseconds(LINK_PING_RATE));
		if (!m_running) break;
		if (out_msg)
		{
			//send msg header and body down the link
			//if sent ok, drop reference to out_msg
			if (send(out_msg)) out_msg = nullptr;
		}
		else
		{
			//send a ping to get the Node_ID exchanged
			auto ping = std::make_shared<Msg>();
			ping->m_header.m_total_length = 0xffffffff;
			send(ping);
		}
	}
	//post any not sent message back to the router
	if (out_msg) global_router->send(out_msg);
}

void Link::run_receive()
{
	//link driver receive loop
	while (m_running)
	{
		//get any msg from the link and send if not a ping
		if (auto in_msg = receive())
		{
			if (in_msg->m_header.m_total_length != 0xffffffff) global_router->send(in_msg);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(LINK_PING_RATE));
		}
	}
	//remove link entry from router
	global_router->sub_link(this);
}
