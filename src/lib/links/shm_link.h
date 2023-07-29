#ifndef SHM_LINK_H
#define SHM_LINK_H

#include "../../host/pii.h"
#include "../mail/router.h"
#include "link.h"
#include <cstring>

extern int64_t pii_open_shared(const char *path, size_t len);
extern int64_t pii_close_shared(const char *path, int64_t hndl);
extern void *pii_mmap(size_t len, int64_t fd, uint64_t mode);
extern int64_t pii_munmap(void *addr, size_t len, uint64_t mode);
extern std::unique_ptr<Router> global_router;

class SHM_Link : public Link
{
public:
	SHM_Link(const std::string &name)
		: Link()
		, m_name(name)
	{
		//open shared memory file
		m_handle = pii_open_shared(m_name.data(), sizeof(lk_shmem));
		//map shared object
		m_shmem = (lk_shmem*)pii_mmap(sizeof(lk_shmem), m_handle, mmap_shared);
		memset(m_shmem, 0, sizeof(lk_shmem));
		//put my towel down if seams available
		if (!m_shmem->m_towel) m_shmem->m_towel = global_router->get_node_id().m_node_id.m_node1;
	}
	virtual ~SHM_Link()
	{
		//unmap object
		pii_munmap(m_shmem, sizeof(lk_shmem), mmap_shared);
		//close it
		pii_close_shared(m_name.data(), m_handle);
	}
protected:
	virtual bool send(const std::shared_ptr<Msg> &msg) override;
	virtual std::shared_ptr<Msg> receive() override;
	virtual void run_send() override;
	virtual void run_receive() override;
private:
	lk_buf *m_in, *m_out;
	lk_buf *m_in_start, *m_in_end;
	lk_buf *m_out_start, *m_out_end;
	lk_shmem *m_shmem;
	int64_t m_handle;
	const std::string m_name;
};

class SHM_Link_Manager : public Link_Manager
{
public:
	SHM_Link_Manager()
		: Link_Manager()
	{}
	void start_thread() override
	{
		m_running = true;
		m_thread = std::thread(&SHM_Link_Manager::run, this);
	}
	void join_thread() override { if (m_thread.joinable()) m_thread.join(); }
private:
	void run() override;
	std::thread m_thread;
	std::vector<std::unique_ptr<SHM_Link>> m_links;
};

#endif
