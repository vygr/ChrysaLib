#include "mailbox.h"
#include "../settings.h"
#include <thread>

/////////////////////
// mailbox management
/////////////////////

Mailbox_ID Mbox_Manager::alloc()
{
	//allocate a new mailbox id and enter the associated mailbox into the validation map.
	//it gets the next mailbox id that does not allready exist and which will not repeat
	//for a very long time.
	std::lock_guard<std::mutex> lock(m_mutex);
	while (m_next_mailbox_id.m_id == MAX_ID
		|| m_mailboxes.find(m_next_mailbox_id) != end(m_mailboxes)) m_next_mailbox_id.m_id++;
	auto id = m_next_mailbox_id;
	m_mailboxes[id];
	m_next_mailbox_id.m_id++;
	return id;
}

void Mbox_Manager::free(Mailbox_ID id)
{
	//free the mailbox associated with this mailbox id
	std::lock_guard<std::mutex> lock(m_mutex);
	auto itr = m_mailboxes.find(id);
	if (itr != end(m_mailboxes)) m_mailboxes.erase(itr);
}

Mbox<std::shared_ptr<Msg>> *Mbox_Manager::validate(Mailbox_ID id)
{
	//validate that this mailbox id has a mailbox associated with it.
	//return the mailbox if so, otherwise nullptr.
	std::lock_guard<std::mutex> lock(m_mutex);
	auto itr = m_mailboxes.find(id);
	return itr == end(m_mailboxes) ? nullptr : &itr->second;
}

int32_t Mbox_Manager::poll(const std::vector<Mailbox_ID> &ids)
{
	//given a list of mailbox id's check to see if any of them contain mail.
	//return the index of the first one that does, else -1.
	auto idx = 0;
	for (auto &id : ids)
	{
		auto mbox = validate(id);
		if (mbox && !mbox->empty()) return idx;
		idx++;
	}
	return -1;
}

int32_t Mbox_Manager::select(const std::vector<Mailbox_ID> &ids)
{
	//block until one of the list of mailboxes contains some mail.
	//returns the index of the first mailbox that has mail.
	for (;;)
	{
		auto idx = poll(ids);
		if (idx != -1) return idx;
		std::this_thread::sleep_for(std::chrono::milliseconds(SELECT_POLLING_RATE));
	}
}
