#include "shm_link.h"
#include "../mail/router.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstring>

extern std::unique_ptr<Router> global_router;
extern uint32_t arg_v;

/////////////
//SHM Manager
/////////////

void SHM_Link_Manager::run()
{
	//start up a new link
	m_links.emplace_back(std::make_unique<SHM_Link>("000-000"));
	m_links.back()->start_threads();

	while (m_running)
	{
		//wait till next polling time
		std::this_thread::sleep_for(std::chrono::milliseconds(SHM_LINK_MANAGER_POLLING_RATE));
	}

	//close any links
	for (auto &link : m_links) link->stop_threads();
	for (auto &link : m_links) link->join_threads();
	m_links.clear();
}

//////////
//SHM link
//////////

//send msg header and body down the link
bool SHM_Link::send(const std::shared_ptr<Msg> &msg)
{
	return true;
}

std::shared_ptr<Msg> SHM_Link::receive()
{
	//receive the buffer from the link

	//unpack msg from receive buffer
	auto msg = std::make_shared<Msg>(m_receive_buf.m_msg_header, m_receive_buf.m_msg_body);

	//refresh who we are connected to in a unplug/plug scenario.
	//if the peer device id changes we need to swap the link on the router !
	//the software equivelent of pulling the lead out and plugging another one in.
	if (m_receive_buf.m_dev_id != m_remote_dev_id)
	{
		m_remote_dev_id = m_receive_buf.m_dev_id;
		global_router->sub_link(this);
		global_router->add_link(this, m_remote_dev_id);
	}
	return msg;
}
