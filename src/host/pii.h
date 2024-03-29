//////////////////////////////////////
// ChrysaLisp pii interface structures
//////////////////////////////////////

#ifndef PII_H
#define PII_H

#include <inttypes.h>

//hard values for now matching sys/link/class.inc
const uint32_t lk_page_size = 4096;
const uint32_t lk_data_size = 984;

enum
{
	mmap_data,
	mmap_exec,
	mmap_shared,
	mmap_none
};

enum
{
	file_open_read,
	file_open_write,
	file_open_append
};

struct pii_stat_info
{
	int64_t mtime;
	int64_t fsize;
	uint16_t mode;
};

struct fn_header
{
	uint64_t ln_fnode;
	uint16_t length;
	uint16_t entry;
	uint16_t links;
	uint16_t paths;
	uint16_t stack;
	uint16_t pathname;
};

struct node_id
{
	uint64_t m_node1;
	uint64_t m_node2;
};

struct net_id
{
	uint64_t m_mbox_id;
	node_id m_node_id;
};

enum
{
	lk_chan_status_ready,
	lk_chan_status_busy
};

struct lk_node
{
	node_id m_peer_node_id;
	uint32_t m_task_count;
};

struct lk_buf
{
	uint32_t m_status;
	uint32_t m_hash;
	uint32_t m_task_count;
	uint32_t m_frag_length;
	uint32_t m_frag_offset;
	uint32_t m_total_length;
	node_id m_peer_node_id;
	net_id m_dest;
	net_id m_src;
	char m_data[lk_data_size];
};

struct lk_chan
{
	lk_buf m_msg0;
	lk_buf m_msg1;
	lk_buf m_msg2;
};

struct lk_shmem
{
	lk_chan m_chan_1;
	uint64_t m_towel;
	char m_pad1[lk_page_size - sizeof(lk_chan) - sizeof(uint64_t)];
	lk_chan m_chan_2;
	char m_pad2[lk_page_size - sizeof(lk_chan)];
};

#endif
