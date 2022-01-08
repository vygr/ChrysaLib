#ifndef MAILBOX_H
#define MAILBOX_H

#include <list>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include <iterator>
#include <condition_variable>

//local mailbox ID, Mailbox_ID 0 is reserved for the Kernel_Service mailbox !
//when a mailbox id is freed its ID will not be reused for a long time.
//this is to ensure that when a mailbox is destroyed any mail directed to that mailbox
//(which may still be in flight in the network) will be cleanly discarded.
struct Mailbox_ID
{
	Mailbox_ID() {}
	Mailbox_ID(uint32_t id)
		:m_id(id)
	{}
	//can be compared !
	bool operator==(const Mailbox_ID &o) const { return o.m_id == m_id; }
	//can be a key in std::set and std::map !
	bool operator<(const Mailbox_ID &o) const { return o.m_id < m_id; }
	//printable conversions
	auto to_string() const
	{
		auto s = std::string{};
		s.reserve(sizeof(m_id) * 2);
		for (auto i = 32 - 4; i >= 0; i -= 4) s.push_back(((m_id >> i) & 0xf) + 'a');
		return s;
	}
	static auto from_string(const std::string &s)
	{
		Mailbox_ID id;
		auto itr = begin(s);
		for (auto i = 32 - 4; i >= 0; i -= 4) id.m_id += (*itr++ - 'a') << i;
		return id;
	}
	uint32_t m_id = 0;
};

//thread sync.
class Sync
{
public:
	Sync() {}
	void wait()
	{
		//suspend caller until notified
		std::unique_lock<std::mutex> l(m_mutex);
		while (m_state) m_cv.wait(l);
		m_state = true;
	}
	void wake()
	{
		//wake any suspended caller
		std::lock_guard<std::mutex> l(m_mutex);
		m_state = false;
		m_cv.notify_one();
	}
private:
	std::mutex m_mutex;
	std::condition_variable m_cv;
	bool m_state = true;
};

//mailbox for thread data exchange.
//abilty for a thread to post an object into a receiver thread que.
//the sending thread does not block and the receiver can read or filter the que
//as it wishes.
template<class T>
class Mbox
{
public:
	Mbox() {}
	T poll()
	{
		//nullptr if que empty
		std::lock_guard<std::mutex> l(m_mutex);
		if (m_mail.empty()) return nullptr;
		auto msg = std::move(m_mail.front());
		m_mail.pop_front();
		return msg;
	}
	void filter(std::vector<T> &out, std::function<bool(T&)> filter)
	{
		//read all that pass filter test
		std::lock_guard<std::mutex> l(m_mutex);
		for (auto itr = begin(m_mail); itr != end(m_mail);)
		{
			if (filter(*itr))
			{
				out.emplace_back(std::move(*itr));
				itr = m_mail.erase(itr);
			}
			else itr++;
		}
	}
	T read()
	{
		//suspend caller if que empty
		std::unique_lock<std::mutex> l(m_mutex);
		while (m_mail.empty()) m_cv.wait(l);
		auto msg = std::move(m_mail.front());
		m_mail.pop_front();
		return msg;
	}
	T read(std::chrono::milliseconds timeout)
	{
		//suspend caller if que empty, nullptr if timer expires
		std::unique_lock<std::mutex> l(m_mutex);
		if (m_mail.empty())
		{
			m_cv.wait_for(l, timeout, [&]{ return !m_mail.empty(); });
			if (m_mail.empty()) return nullptr;
		}
		auto msg = std::move(m_mail.front());
		m_mail.pop_front();
		return msg;
	}
	void post(T &msg)
	{
		//wake any suspended caller
		std::lock_guard<std::mutex> l(m_mutex);
		m_mail.emplace_back(std::move(msg));
		if (m_select) m_select->wake();
		m_cv.notify_one();
	}
	auto empty() const { return m_mail.empty(); }
	void lock() { m_mutex.lock(); }
	void unlock() { m_mutex.unlock(); }
	void set_select(Sync *select) { m_select = select; }
private:
	mutable std::mutex m_mutex;
	std::condition_variable m_cv;
	std::list<T> m_mail;
	Sync *m_select = nullptr;
};

#endif
