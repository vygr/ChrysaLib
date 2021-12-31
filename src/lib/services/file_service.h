#ifndef FILES_SERVICE_H
#define FILES_SERVICE_H

#include "service.h"
#include "../utils/threadpool.h"

//this is an example service.
//it is a base class for providing a dropbox read/write type folder service.
//it uses a threadpool to handle long running requests.
//this illustrates how you might construct services and helper functions
//that let clients of that service interact with them.
class File_Service : public Service
{
public:
	enum
	{
		evt_exit, //must be first !
		evt_get_file_list,
		evt_set_file_list,
		evt_send_file,
		evt_transfer_file,
	};
	struct Event_get_file_list : public Event
	{
		Net_ID m_reply;
	};
	struct Event_set_file_list : public Event
	{
		Net_ID m_src;
		char m_data[];
	};
	struct Event_send_file : public Event
	{
		Net_ID m_reply;
		char m_name[];
	};
	struct send_file_chunk
	{
		Net_ID m_ack;
		uint64_t m_offset;
		uint64_t m_length;
		uint64_t m_total;
		char m_data[];
	};
	struct Event_transfer_file : public Event
	{
		Net_ID m_src;
		Net_ID m_origin;
		int32_t m_ctx;
		char m_data[];
	};
	struct transfer_file_progress
	{
		int32_t m_progress;
	};
	File_Service(Router &router)
		: Service(router)
	{
		//async task pools, where big or blocking requests run
		m_thread_pool1 = std::make_unique<ThreadPool>(2);
		m_thread_pool2 = std::make_unique<ThreadPool>(1);
	}
	//remote push helper methods
	File_Service *set_file_list(const Net_ID &net_id, const std::vector<std::string> &file_list);
	//request helper methods
	File_Service *get_file_list(const Net_ID &net_id);
	File_Service *transfer_file(const Net_ID &dst_id, const Net_ID &src_id, const std::string &dst_name, const std::string &src_name, int32_t ctx);
private:
	//note the use of two threadpools, one for the responce and one for the requests
	//you do not want to deadlock due to all the threads being taken by requests and
	//then no one can respond to them...
	std::unique_ptr<ThreadPool> m_thread_pool1;
	std::unique_ptr<ThreadPool> m_thread_pool2;
	void run() override;
protected:
	//methods for supplying the service with info
	virtual std::string in_temp_file() { return "temp.tmp"; }
	virtual void in_file_list(std::vector<std::string> &file_list)
	{
		(void) file_list;
		return;
	}
	virtual std::string in_file_path() { return ""; }
	//methods for the service to emit results
	virtual void out_file_list(const Net_ID &src_id, const std::vector<std::string> &file_list)
	{
		(void) file_list; (void) src_id;
		return;
	}
	virtual void out_error(const Net_ID &dst_id, const Net_ID &src_id, const std::string &dst_name, const std::string &src_name, int32_t ctx)
	{
		(void) dst_id; (void) src_id; (void) dst_name; (void) src_name; (void) ctx;
		return;
	}
	virtual void out_progress(const Net_ID &dst_id, const Net_ID &src_id, const std::string &dst_name, const std::string &src_name, int32_t ctx, int32_t progress)
	{
		(void) dst_id; (void) src_id; (void) dst_name; (void) src_name; (void) ctx; (void) progress;
		return;
	}
	virtual void out_ok(const Net_ID &dst_id, const Net_ID &src_id, const std::string &dst_name, const std::string &src_name, int32_t ctx)
	{
		(void) dst_id; (void) src_id; (void) dst_name; (void) src_name; (void) ctx;
		return;
	}
	virtual void out_log(const std::string &log)
	{
		(void) log;
		return;
	}
};

#endif
