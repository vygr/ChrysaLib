#include "shm_link.h"
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

void SHM_Link::run_send()
{
	// //assign tx channel
	// if (m_shmem->m_towel == global_router->get_node_id().m_node.m_node1)
	// {
	// 	m_out = &m_shmem->m_chan_1.m_msg0;
	// }
	// else
	// {
	// 	m_out = &m_shmem->m_chan_2.m_msg0;
	// }
	// m_out_start = m_out;
	// m_out_end = (lk_buf*)((char*)m_out + sizeof(lk_chan));

	//link driver send loop
	Link::run_send();
}

void SHM_Link::run_receive()
{
	//assign rx channel
	// if (m_shmem->m_towel == global_router->get_node_id().m_node.m_node1)
	// {
	// 	m_in = &m_shmem->m_chan_2.m_msg0;
	// }
	// else
	// {
	// 	m_in = &m_shmem->m_chan_1.m_msg0;
	// }
	// m_in_start = m_in;
	// m_in_end = (lk_buf*)((char*)m_in + sizeof(lk_chan));

	//link driver receive loop
	Link::run_receive();
}

//send msg header and body down the link
bool SHM_Link::send(const std::shared_ptr<Msg> &msg)
{
	//wait for slot ready
	while (m_running && (m_out->m_status == lk_chan_status_busy))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(LINK_PING_RATE));
	}
	if (!m_running) return false;

	//send msg to the link
	m_out->m_peer_node_id = global_router->get_node_id().m_node_id;
	m_out->m_task_count = 1000000; //a very big wall !
	m_out->m_frag_length = msg->m_header.m_frag_length;
	m_out->m_frag_offset = msg->m_header.m_frag_offset;
	m_out->m_total_length = msg->m_header.m_total_length;
	m_out->m_dest.m_mbox_id = msg->m_header.m_dest.m_mailbox_id.m_id;
	m_out->m_dest.m_node_id.m_node1 = msg->m_header.m_dest.m_node_id.m_node_id.m_node1;
	m_out->m_dest.m_node_id.m_node2 = msg->m_header.m_dest.m_node_id.m_node_id.m_node2;
	m_out->m_src.m_mbox_id = msg->m_header.m_src.m_mailbox_id.m_id;
	m_out->m_src.m_node_id.m_node1 = msg->m_header.m_src.m_node_id.m_node_id.m_node1;
	m_out->m_src.m_node_id.m_node2 = msg->m_header.m_src.m_node_id.m_node_id.m_node2;
	memcpy(&m_out->m_data, msg->begin() + msg->m_header.m_data_offset, msg->m_header.m_frag_length);

	//set busy and move on to next buffer
	m_out->m_status = lk_chan_status_busy;
	m_out = (lk_buf*)((char*)m_out + sizeof(lk_buf));
	if (m_out == m_out_end) m_out = m_out_start;

	return true;
}

std::shared_ptr<Msg> SHM_Link::receive()
{
	//wait for slot ready
	while (m_running && (m_in->m_status == lk_chan_status_ready))
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(LINK_PING_RATE));
	}
	if (!m_running) return nullptr;

	//receive msg from the link
	Msg_Header head;
	Node_ID remote;
	remote.m_node_id = m_in->m_peer_node_id;
	head.m_frag_length = m_in->m_frag_length;
	head.m_frag_offset = m_in->m_frag_offset;
	head.m_total_length = m_in->m_total_length;
	head.m_dest.m_mailbox_id.m_id = m_in->m_dest.m_mbox_id;
	head.m_dest.m_node_id.m_node_id.m_node1 = m_in->m_dest.m_node_id.m_node1;
	head.m_dest.m_node_id.m_node_id.m_node2 = m_in->m_dest.m_node_id.m_node2;
	head.m_src.m_mailbox_id.m_id = m_in->m_src.m_mbox_id;
	head.m_src.m_node_id.m_node_id.m_node1 = m_in->m_src.m_node_id.m_node1;
	head.m_src.m_node_id.m_node_id.m_node2 = m_in->m_src.m_node_id.m_node2;
	auto msg = std::make_shared<Msg>(head, (const uint8_t*)m_in->m_data);

	//set ready and move on to next buffer
	m_in->m_status = lk_chan_status_ready;
	m_in = (lk_buf*)((char*)m_in + sizeof(lk_buf));
	if (m_in == m_in_end) m_in = m_in_start;

	//refresh who we are connected to in a unplug/plug scenario.
	//if the peer node id changes we need to swap the link on the router !
	//the software equivelent of pulling the lead out and plugging another one in.
	if (remote != m_remote_node_id)
	{
		m_remote_node_id = remote;
		global_router->sub_link(this);
		global_router->add_link(this, m_remote_node_id);
	}
	return msg;
}
